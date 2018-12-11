/*
 * @brief UART 0 interrupt example with ring buffers
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

#include "chip.h"
#include "board.h"
#include "string.h"
#include "uart_0_rb.h"

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/
/* Transmit and receive ring buffers */
STATIC RINGBUFF_T txring, rxring;

/* Transmit and receive ring buffer sizes */
#define UART_SRB_SIZE 128	/* Send */
#define UART_RRB_SIZE 32	/* Receive */

/* Transmit and receive buffers */
static uint8_t rxbuff[UART_RRB_SIZE], txbuff[UART_SRB_SIZE];

const char inst1[] = "LPC11u6x UART example using ring buffers\r\n";
const char inst2[] = "Press a key to echo it back or ESC to quit\r\n";

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/

/*****************************************************************************
 * Private functions
 ****************************************************************************/
static void Init_UART_PinMux(void)
{
#ifdef _REAL_UPDATEKIT_V2_BOARD_

	Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 18, (IOCON_FUNC1 | IOCON_MODE_INACT | IOCON_DIGMODE_EN));
	Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 19, (IOCON_FUNC1 | IOCON_MODE_INACT | IOCON_DIGMODE_EN));

#else
#if defined(BOARD_MANLEY_11U68)
	Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 18, (IOCON_FUNC1 | IOCON_MODE_INACT | IOCON_DIGMODE_EN));
	Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 19, (IOCON_FUNC1 | IOCON_MODE_INACT | IOCON_DIGMODE_EN));

#elif defined(BOARD_NXP_LPCXPRESSO_11U68)
	Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 18, (IOCON_FUNC1 | IOCON_MODE_INACT | IOCON_DIGMODE_EN));
	Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 19, (IOCON_FUNC1 | IOCON_MODE_INACT | IOCON_DIGMODE_EN));

#else
#error "No UART setup defined"
#endif
#endif // #ifdef _REAL_UPDATEKIT_V2_BOARD_
}

/*****************************************************************************
 * Public functions
 ****************************************************************************/
void SCT_IRQHandler(void)
{
	/* Clear the Interrupt */
	Chip_SCT_ClearEventFlag(LPC_SCT1, SCT_EVT_0);
}

/**
 * @brief	UART interrupt handler using ring buffers
 * @return	Nothing
 */
void USART0_IRQHandler(void)
{
	/* Want to handle any errors? Do it here. */

	/* Use default ring buffer handler. Override this with your own
	   code if you need more capability. */
	Chip_UART0_IRQRBHandler(LPC_USART0, &rxring, &txring);
}

void Init_UART0(void)
{
	Init_UART_PinMux();

	/* Setup UART for 115.2K8N1 */
	Chip_UART0_Init(LPC_USART0);
	Chip_UART0_SetBaud(LPC_USART0, 115200);
	Chip_UART0_ConfigData(LPC_USART0, (UART0_LCR_WLEN8 | UART0_LCR_SBS_1BIT));
	Chip_UART0_SetupFIFOS(LPC_USART0, (UART0_FCR_FIFO_EN | UART0_FCR_TRG_LEV2));
	Chip_UART0_TXEnable(LPC_USART0);

	/* Before using the ring buffers, initialize them using the ring
	   buffer init function */
	RingBuffer_Init(&rxring, rxbuff, 1, UART_RRB_SIZE);
	RingBuffer_Init(&txring, txbuff, 1, UART_SRB_SIZE);

	/* Enable receive data and line status interrupt */
	Chip_UART0_IntEnable(LPC_USART0, (UART0_IER_RBRINT | UART0_IER_RLSINT));

	/* Enable UART 0 interrupt */
	NVIC_EnableIRQ(USART0_IRQn);

	/* Send initial messages */
	Chip_UART0_SendRB(LPC_USART0, &txring, inst1, sizeof(inst1) - 1);
	Chip_UART0_SendRB(LPC_USART0, &txring, inst2, sizeof(inst2) - 1);
}

void DeInit_UART0(void)
{
    NVIC_DisableIRQ(USART0_IRQn);
    Chip_UART0_DeInit(LPC_USART0);
}

int UART0_GetChar(void *return_ch)
{
	return Chip_UART0_ReadRB(LPC_USART0, &rxring, return_ch, 1);
}

uint32_t UART0_PutChar(char ch)
{
	uint32_t return_value;

	return_value = Chip_UART0_SendRB(LPC_USART0, &txring, (const uint8_t *) &ch, 1);
	if ( return_value!= 1) {
//				Board_LED_Toggle(0); /* Toggle LED if the TX FIFO is full */
	}

	return return_value;
}

int itoa_10(uint32_t value, char* result)
{
	// check that the base if valid

	char*       ptr = result, *ptr1 = result, tmp_char;
	uint32_t    tmp_value;
    int         str_len;

	str_len = 0;
	do {
		tmp_value = value % 10;
		value /= 10;
		*ptr++ = "0123456789" [tmp_value];
		str_len++;
	} while ( value );

	*ptr-- = '\0';
	while(ptr1 < ptr) {
		tmp_char = *ptr;
		*ptr--= *ptr1;
		*ptr1++ = tmp_char;
	}
	return str_len;
}

int itoa_16(uint32_t value, char* result)
{
	// check that the base if valid

	char*       ptr = result, *ptr1 = result, tmp_char;
	uint32_t    tmp_value;
    int         str_len;

	str_len = 0;
	do {
		tmp_value = value % 16;
		value /= 16;
		*ptr++ = "0123456789abcdef" [tmp_value];
		str_len++;
	} while ( value );

	*ptr-- = '\0';
	while(ptr1 < ptr) {
		tmp_char = *ptr;
		*ptr--= *ptr1;
		*ptr1++ = tmp_char;
	}
	return str_len;
}

int OutputHexValue(uint32_t value)
{
    char    temp_str[8+1];
    int     byte_length, return_value;
    byte_length = itoa_16(value,temp_str);

    return_value = Chip_UART0_SendRB(LPC_USART0, &txring, (const uint8_t *) temp_str, byte_length);
	if ( return_value!= byte_length) {
//				Board_LED_Toggle(0); /* Toggle LED if the TX FIFO is full */
	}

	return return_value;
}

int OutputHexValue_with_newline(uint32_t value)		// Assume that HEX is at most 8-digits
{
    char    temp_str[8+1+2];
    int     byte_length, return_value;

    byte_length = itoa_16(value,temp_str);
    temp_str[byte_length++] = '\n';

#if (NEW_LINE_SYMBOL==_R_N_)
    temp_str[byte_length++] = '\r';
#endif //  (NEW_LINE_SYMBOL==_R_N_)

    return_value = Chip_UART0_SendRB(LPC_USART0, &txring, (const uint8_t *) temp_str, byte_length);
	if ( return_value!= byte_length) {
//				Board_LED_Toggle(0); /* Toggle LED if the TX FIFO is full */
	}

	return return_value;
}

int OutputString(char *str)
{
    int return_value, temp_len=0;
    char *temp_ch = str;

    while(*temp_ch!='\0')
    {
    	temp_ch++;
    	temp_len++;
    }

    return_value = Chip_UART0_SendRB(LPC_USART0, &txring, (const uint8_t *) str, temp_len);
    return return_value;
}

int OutputString_with_newline(char *str)
{
    int return_value = 0;
    return_value = OutputString(str);
    return_value += UART0_PutChar('\n');
#if (NEW_LINE_SYMBOL==_R_N_)
    return_value += UART0_PutChar('\r');
#endif //  (NEW_LINE_SYMBOL==_R_N_)

    return return_value;
}

