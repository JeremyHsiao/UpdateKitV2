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
#if defined(BOARD_MANLEY_11U68)
	Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 18, (IOCON_FUNC1 | IOCON_MODE_INACT | IOCON_DIGMODE_EN));
	Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 19, (IOCON_FUNC1 | IOCON_MODE_INACT | IOCON_DIGMODE_EN));

#elif defined(BOARD_NXP_LPCXPRESSO_11U68)
	Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 18, (IOCON_FUNC1 | IOCON_MODE_INACT | IOCON_DIGMODE_EN));
	Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 19, (IOCON_FUNC1 | IOCON_MODE_INACT | IOCON_DIGMODE_EN));

#else
#error "No UART setup defined"
#endif
}

/*****************************************************************************
 * Public functions
 ****************************************************************************/

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

/**
 * @brief	Main UART program body
 * @return	Always returns 1
 */
int main(void)
{
	uint8_t key;
	int bytes;

	SystemCoreClockUpdate();
	Board_Init();
	Init_UART_PinMux();
	Board_LED_Set(0, false);

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

	/* Poll the receive ring buffer for the ESC (ASCII 27) key */
	key = 0;
	while (key != 27) {
		bytes = Chip_UART0_ReadRB(LPC_USART0, &rxring, &key, 1);
		if (bytes > 0) {
			/* Wrap value back around */
			if (Chip_UART0_SendRB(LPC_USART0, &txring, (const uint8_t *) &key, 1) != 1) {
				Board_LED_Toggle(0); /* Toggle LED if the TX FIFO is full */
			}
		}
	}

	/* DeInitialize UART0 peripheral */
	NVIC_DisableIRQ(USART0_IRQn);
	Chip_UART0_DeInit(LPC_USART0);

	return 1;
}
