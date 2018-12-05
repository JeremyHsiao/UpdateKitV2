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

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/

/* EEPROM Address used for storage */
#define EEPROM_ADDR         0x40

/* Write count */
#define IAP_NUM_BYTES_TO_READ_WRITE 128

/* Read/write buffer (32-bit aligned) */
uint32_t buffer[(IAP_NUM_BYTES_TO_READ_WRITE / sizeof(uint32_t)) + 1]; 
/* Pre-setup String */

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/

/*****************************************************************************
 * Private functions
 ****************************************************************************/

void eeprom_read_example(void)
{
	uint8_t *ptr = (uint8_t *) buffer;
	uint8_t ret_code;

	/* Data to be read from EEPROM */
	ret_code = Chip_EEPROM_Read(EEPROM_ADDR, ptr, IAP_NUM_BYTES_TO_READ_WRITE);

	/* Error checking */
	if (ret_code != IAP_CMD_SUCCESS) {
		//DEBUGOUT("Command failed to execute, return code is: %x\r\n", ret_code);
	}
}

void eeprom_write_example(void)
{
	uint8_t *ptr = (uint8_t *) buffer;
	uint8_t ret_code;

	/* Data to be written to EEPROM */
	ret_code = Chip_EEPROM_Write(EEPROM_ADDR, ptr, IAP_NUM_BYTES_TO_READ_WRITE);

	/* Error checking */
	if (ret_code == IAP_CMD_SUCCESS) {
		//DEBUGSTR("EEPROM write passed\r\n");
	}
	else {
		//DEBUGOUT("EEPROM write failed, return code is: %x\r\n", ret_code);
	}
}