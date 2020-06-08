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

// User Selection
#define EEPROM_RESERVE_AREA				(0x60)
#define	USER_SELECTION_POSITION			(0x80)			// must be larger than EEPROM_RESERVE_AREA
#define USER_SELECTION_LENGTH			(4)
uint8_t User_Select_Last_ReadWrite = POWER_OUTPUT_STEP_TOTAL_NO;
// Timeout according to previous update
#define	SYSTEM_TIMEOUT_VALUE_POSITION	(USER_SELECTION_POSITION+USER_SELECTION_LENGTH)
#define SYSTEM_TIMEOUT_VALUE_LENGTH		(4)
uint16_t System_Timeout_Last_ReadWrite = DEFAULT_MAX_FW_UPDATE_TIME_IN_S;
uint8_t For_TIMEOUT_EEPROM_User_Select_Last_ReadWrite = 0;
// This is starting position for future EEPROM user data
#define NEXT_FUTURE_EEPROM_DATA_START	(SYSTEM_TIMEOUT_VALUE_POSITION+(SYSTEM_TIMEOUT_VALUE_POSITION*POWER_OUTPUT_STEP_TOTAL_NO))
// For voltage output branch
#define	PWM_SELECTION_POSITION			(NEXT_FUTURE_EEPROM_DATA_START)
#define PWM_SELECTION_LENGTH			(4)
uint8_t PWM_Select_Last_ReadWrite = 0;
// END - For voltage output branch

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

bool Load_User_Selection(uint8_t *pUserSelect)
{
	uint32_t user_selection_buffer[(USER_SELECTION_LENGTH / sizeof(uint32_t)) + 1];
	uint8_t *ptr = (uint8_t *) user_selection_buffer;
	uint8_t ret_code;

	/* Data to be read from EEPROM */
	ret_code = Chip_EEPROM_Read_v2(USER_SELECTION_POSITION, ptr, USER_SELECTION_LENGTH);

	/* Error checking */
	if (ret_code != IAP_CMD_SUCCESS) {
		//DEBUGOUT("Command failed to execute, return code is: %x\r\n", ret_code);
		*pUserSelect = User_Select_Last_ReadWrite = DEFAULT_POWER_OUTPUT_STEP;
		return false;		// cannot validate
	}

	if((*ptr<POWER_OUTPUT_STEP_TOTAL_NO)&&(ptr[0]==ptr[3])&&(ptr[1]==ptr[2])&&(ptr[0]==(ptr[1]^0xff)))
	{
		*pUserSelect = User_Select_Last_ReadWrite = *ptr;
		return true;
	}
	else
	{
		*pUserSelect = User_Select_Last_ReadWrite = DEFAULT_POWER_OUTPUT_STEP;
		return false;
	}
}

bool Check_if_different_from_last_ReadWrite(uint8_t UserSelect)
{
	return (UserSelect!=User_Select_Last_ReadWrite)?true:false;
}

bool Save_User_Selection(uint8_t UserSelect)
{
	uint32_t user_selection_buffer[(USER_SELECTION_LENGTH / sizeof(uint32_t)) + 1];
	uint8_t *ptr = (uint8_t *) user_selection_buffer;
	uint8_t ret_code;

	// return false if out of range
	if(UserSelect>=POWER_OUTPUT_STEP_TOTAL_NO){
		return false;
	}

	/* Data to be written to EEPROM */
	ptr[0]=ptr[3]=UserSelect;
	ptr[1]=ptr[2]=(UserSelect^0xff);
	ret_code = Chip_EEPROM_Write_v2(USER_SELECTION_POSITION, ptr, USER_SELECTION_LENGTH);

	/* Error checking */
	if (ret_code == IAP_CMD_SUCCESS)
	{
		User_Select_Last_ReadWrite = UserSelect;
		return true;
	}
	else
	{
		return false;
	}
}

bool Load_System_Timeout_v2(uint8_t user_selection, uint16_t *pSystemTimeout)
{
	uint32_t system_timeout_buffer[(SYSTEM_TIMEOUT_VALUE_LENGTH / sizeof(uint32_t)) + 1];
	uint16_t *ptr = (uint16_t *) system_timeout_buffer;
	uint8_t ret_code;

	// return default value if user_selection is 0 -- in fact this value is not used for pc-mode
	if(user_selection==0)
	{
		*pSystemTimeout = System_Timeout_Last_ReadWrite = DEFAULT_MAX_FW_UPDATE_TIME_IN_S;
		return true;
	}

	/* Data to be read from EEPROM */
	ret_code = Chip_EEPROM_Read_v2(SYSTEM_TIMEOUT_VALUE_POSITION+(user_selection*SYSTEM_TIMEOUT_VALUE_LENGTH), (uint8_t *)ptr, SYSTEM_TIMEOUT_VALUE_LENGTH);

	/* Error checking */
	if (ret_code != IAP_CMD_SUCCESS) {
		//DEBUGOUT("Command failed to execute, return code is: %x\r\n", ret_code);
		*pSystemTimeout = System_Timeout_Last_ReadWrite = DEFAULT_MAX_FW_UPDATE_TIME_IN_S;
		return false;		// cannot validate
	}

	if((*ptr>MINIMAL_TIMEOUT_VALUE)&&(ptr[0]==(ptr[1]^0xffff)))
	{
//		To check max. value
//		if(*ptr>MAXIMAL_TIMEOUT_VALUE)
//			*ptr = DEFAULT_MAX_FW_UPDATE_TIME_IN_S;
		*pSystemTimeout = System_Timeout_Last_ReadWrite = *ptr;
		For_TIMEOUT_EEPROM_User_Select_Last_ReadWrite = user_selection;
		return true;
	}
	else
	{
		*pSystemTimeout = System_Timeout_Last_ReadWrite = DEFAULT_MAX_FW_UPDATE_TIME_IN_S;
		return false;
	}
}

bool Check_if_different_from_last_System_Timeout_v2(uint8_t user_selection, uint16_t timeout)
{
	return ((timeout!=System_Timeout_Last_ReadWrite)||(user_selection!=For_TIMEOUT_EEPROM_User_Select_Last_ReadWrite))?true:false;
}

bool Save_System_Timeout_v2(uint8_t user_selection, uint16_t SystemTimeout)
{
	uint32_t system_timeout_buffer[(SYSTEM_TIMEOUT_VALUE_LENGTH / sizeof(uint32_t)) + 1];
	uint16_t *ptr = (uint16_t *) system_timeout_buffer;
	uint8_t ret_code;

	// return true if user_selection is 0 -- no need to save timeout value for pc-mode
	if(user_selection==0)
	{
		System_Timeout_Last_ReadWrite = DEFAULT_MAX_FW_UPDATE_TIME_IN_S;
		return true;
	}

	// return false if out of range
	if(SystemTimeout<=MINIMAL_TIMEOUT_VALUE){
		return false;
	}

	/* Data to be written to EEPROM */
	ptr[0]=SystemTimeout;
	ptr[1]=(SystemTimeout^0xffff);
	ret_code = Chip_EEPROM_Write_v2(SYSTEM_TIMEOUT_VALUE_POSITION+(user_selection*SYSTEM_TIMEOUT_VALUE_LENGTH), (uint8_t *)ptr, SYSTEM_TIMEOUT_VALUE_LENGTH);

	/* Error checking */
	if (ret_code == IAP_CMD_SUCCESS)
	{
		System_Timeout_Last_ReadWrite = SystemTimeout;
		return true;
	}
	else
	{
		return false;
	}
}

// For voltage output branch
//#define	PWM_SELECTION_POSITION			(NEXT_FUTURE_EEPROM_DATA_START)
//#define PWM_SELECTION_LENGTH			(4)

