/*
 * @brief EEPROM example
 * This example show how to use the EEPROM interface
 *
 * @note
 * Copyright(C) NXP Semiconductors, 2013
 * All rights reserved.
 *
 * @par
 * Software that is described herein is for illustrative purposes only
 * which provides customers with programming information regarding the
 * LPC products.  This software is supplied "AS IS" without any warranties of
 * any kind, and NXP Semiconductors and its licensor disclaim any and
 * all warranties, express or implied, including all implied warranties of
 * merchantability, fitness for a particular purpose and non-infringement of
 * intellectual property rights.  NXP Semiconductors assumes no responsibility
 * or liability for the use of the software, conveys no license or rights under any
 * patent, copyright, mask work right, or any other intellectual property rights in
 * or to any products. NXP Semiconductors reserves the right to make changes
 * in the software without notification. NXP Semiconductors also makes no
 * representation or warranty that such application will be suitable for the
 * specified use without further testing or modification.
 *
 * @par
 * Permission to use, copy, modify, and distribute this software and its
 * documentation is hereby granted, under NXP Semiconductors' and its
 * licensor's relevant copyrights in the software, without fee, provided that it
 * is used in conjunction with NXP Semiconductors microcontrollers.  This
 * copyright, permission, and disclaimer notice must appear in all copies of
 * this code.
 */

#include "board.h"
#include "eeprom.h"
#include "user_opt.h"
#include "sw_timer.h"
#include "res_state.h"
#include "user_if.h"
#include "string.h"

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/

///* EEPROM Address used for storage */
//#define EEPROM_ADDR         0x40
//
///* Write count */
//#define IAP_NUM_BYTES_TO_READ_WRITE 128
//
///* Read/write buffer (32-bit aligned) */
//uint32_t buffer[(IAP_NUM_BYTES_TO_READ_WRITE / sizeof(uint32_t)) + 1];
///* Pre-setup String */

#define EEPROM_RESERVE_AREA				(0x60)
#define OPTION_COUNT					(4)
#define	RESISTOR_VALUE_POSITION			(0x80)													// must be larger than EEPROM_RESERVE_AREA
#define RESISTOR_VALUE_LENGTH			(sizeof(uint32_t)*(OPTION_COUNT*2))

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/

/*****************************************************************************
 * Private functions
 ****************************************************************************/

//void eeprom_read_example(void)
//{
//	uint8_t *ptr = (uint8_t *) buffer;
//	uint8_t ret_code;
//
//	/* Data to be read from EEPROM */
//	ret_code = Chip_EEPROM_Read(EEPROM_ADDR, ptr, IAP_NUM_BYTES_TO_READ_WRITE);
//
//	/* Error checking */
//	if (ret_code != IAP_CMD_SUCCESS) {
//		//DEBUGOUT("Command failed to execute, return code is: %x\r\n", ret_code);
//	}
//}
//
//void eeprom_write_example(void)
//{
//	uint8_t *ptr = (uint8_t *) buffer;
//	uint8_t ret_code;
//
//	/* Data to be written to EEPROM */
//	ret_code = Chip_EEPROM_Write(EEPROM_ADDR, ptr, IAP_NUM_BYTES_TO_READ_WRITE);
//
//	/* Error checking */
//	if (ret_code == IAP_CMD_SUCCESS) {
//		//DEBUGSTR("EEPROM write passed\r\n");
//	}
//	else {
//		//DEBUGOUT("EEPROM write failed, return code is: %x\r\n", ret_code);
//	}
//}

static uint32_t	temp_ISER0;

static inline void DisableAllInterrupt(void)
{
	temp_ISER0 = NVIC->ISER[0];
	NVIC->ICER[0] = temp_ISER0;
	SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;                    /* Disable SysTick IRQ */
}

static inline void RestoreAllInterrupt(void)
{
	SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;                    /* Enable SysTick IRQ */
	NVIC->ISER[0] = temp_ISER0;
}

/* Write data to EEPROM */
static inline uint8_t Chip_EEPROM_Write_v2(uint32_t dstAdd, uint8_t *ptr, uint32_t byteswrt)
{
	uint32_t command[5], result[4];

	command[0] = IAP_EEPROM_WRITE;
	command[1] = dstAdd;
	command[2] = (uint32_t) ptr;
	command[3] = byteswrt;
	command[4] = SystemCoreClock / 1000;
	DisableAllInterrupt();
	iap_entry(command, result);
	RestoreAllInterrupt();

	return result[0];
}

/* Read data from EEPROM */
static inline uint8_t Chip_EEPROM_Read_v2(uint32_t srcAdd, uint8_t *ptr, uint32_t bytesrd)
{
	uint32_t command[5], result[4];

	command[0] = IAP_EEPROM_READ;
	command[1] = srcAdd;
	command[2] = (uint32_t) ptr;
	command[3] = bytesrd;
	command[4] = SystemCoreClock / 1000;
	DisableAllInterrupt();
	iap_entry(command, result);
	RestoreAllInterrupt();

	return result[0];
}


static uint32_t last_read_write_value[OPTION_COUNT*2];

int Load_Resistor_Value(void)
{
	uint32_t res[OPTION_COUNT*2];
	uint8_t *ptr = (uint8_t *) res;
	uint8_t ret_code, index;
	uint32_t *res_ptr = GetResistorValue();
	uint32_t *power_2_ptr = Get_2PowerN_Value();
	int 		return_value = 0;

	/* Data to be read from EEPROM */
	ret_code = Chip_EEPROM_Read_v2(RESISTOR_VALUE_POSITION, ptr, RESISTOR_VALUE_LENGTH);

	/* Error checking */
	if (ret_code != IAP_CMD_SUCCESS) {
		//DEBUGOUT("Command failed to execute, return code is: %x\r\n", ret_code);
		res_ptr[0]=res_ptr[1]=res_ptr[2]=1;
		*power_2_ptr = 0;
		return -1;		// cannot validate
	}

	// update last_read_write value
	memcpy(last_read_write_value,res,RESISTOR_VALUE_LENGTH);

	index = 2;
	do
	{
		if((res[index]>=(1UL<<20))||(res[index]!=(~res[index+OPTION_COUNT])))
		{
			res_ptr[index]=1;
			return_value = 1;
		}
		else
		{
			res_ptr[index] = res[index];
		}
	}
	while(index-->0);

	if((res[3]>=(22))||(res[3]!=(~res[3+OPTION_COUNT])))
	{
		*power_2_ptr=0;
		return_value = 1;
	}
	else
	{
		*power_2_ptr=res[3];
	}

	return return_value;
}

bool Check_if_Resistor_different_from_last_ReadWrite(void)
{
	uint32_t *res_ptr = GetResistorValue();
	uint32_t *power_2_ptr = Get_2PowerN_Value();

	if(	(last_read_write_value[0]!=res_ptr[0])||
		(last_read_write_value[1]!=res_ptr[1])||
		(last_read_write_value[2]!=res_ptr[2])||
		(last_read_write_value[3]!=*power_2_ptr))
	{
		return true;
	}
	else
	{
		return false;
	}

}

bool Save_Resistor_Value(void)
{
	uint32_t res[OPTION_COUNT*2];
	uint8_t *ptr = (uint8_t *) res;
	uint8_t ret_code, index;
	uint32_t *res_ptr = GetResistorValue();
	uint32_t *power_2_ptr = Get_2PowerN_Value();

	index = 2;
	do
	{
		if(res_ptr[index]>=(1UL<<20))
		{
			// return false if out of range
			return false;
		}
		else
		{
			res[index] = res_ptr[index];
			res[index+OPTION_COUNT] = ~res[index];
		}
	}
	while(index-->0);

	if(*power_2_ptr>=22)
	{
		// return false if out of range
		return false;
	}
	else
	{
		res[3] = *power_2_ptr;
		res[3+OPTION_COUNT] = ~res[3];
	}

	/* Data to be written to EEPROM */
	ret_code = Chip_EEPROM_Write_v2(RESISTOR_VALUE_POSITION, ptr, RESISTOR_VALUE_LENGTH);

	/* Error checking */
	if (ret_code == IAP_CMD_SUCCESS)
	{
		memcpy(last_read_write_value,res,RESISTOR_VALUE_LENGTH);
		return true;
	}
	else
	{
		return false;
	}
}

void Update_VR_Page_value_at_beginning()
{

	uint32_t *res_ptr = GetResistorValue();
	int	temp_len;
	char temp_text[10];

	temp_len = Show_Resistor_3_Digits(res_ptr[0],temp_text);
	UI_V2_Update_after_change(0,res_ptr[0],temp_text,temp_len);
	temp_len = Show_Resistor_3_Digits(res_ptr[1],temp_text);
	UI_V2_Update_after_change(1,res_ptr[1],temp_text,temp_len);
	temp_len = Show_Resistor_3_Digits(res_ptr[2],temp_text);
	UI_V2_Update_after_change(2,res_ptr[2],temp_text,temp_len);

}

