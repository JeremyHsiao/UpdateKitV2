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

// BEGIN - from PWM example
/* Systick is used for general timing */
#define TICKRATE_HZ (33)	/* 33% rate of change per second */

/* PWM cycle time - time of a single PWM sweep */
#define PWMCYCLERATE (1000)

/* Saved tick count used for a PWM cycle */
static uint32_t cycleTicks;
// END

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

/* Set SCT PWM 'high' period based on passed percentage */
void setPWMRate(int pwnNum, uint8_t percentage)
{
	uint32_t value;

	/* Limit valid PWMs to 3 */
	if ((pwnNum < 0) || (pwnNum > 3)) {
		return;
	}

	if (percentage >= 100) {
		value = 1;
	}
	else if (percentage == 0) {
		value = cycleTicks + 1;
	}
	else {
		uint32_t newTicks;

		newTicks = (cycleTicks * percentage) / 100;

		/* Approximate duty cycle rate */
		value = cycleTicks - newTicks;
	}

	LPC_SCT1->MATCHREL[pwnNum + 1].U = value;
}

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

/**
 * @brief	Main UART program body
 * @return	Always returns 1
 */
int main(void)
{
	uint8_t key, dutyCycle;
	int bytes, countdir = 10;

	SystemCoreClockUpdate();
	Board_Init();
	Init_UART_PinMux();
	Board_LED_Set(0, false);

	// BEGIN PWM
	/* Chip specific SCT setup - clocks and peripheral reset
	   There are a lot of registers in the SCT peripheral. Performing
	   the reset allows the default states of the SCT to be loaded, so
	   we don't need to set them all and rely on defaults when needed. */
	Chip_SCT_Init(LPC_SCT1);

	/* SCT1_OUT0 on PIO2_16 mapped to FUNC2 */
	Chip_IOCON_PinMuxSet(LPC_IOCON, 2, 16, (IOCON_FUNC1 | IOCON_MODE_INACT |
										   IOCON_ADMODE_EN));


	/* Configure the SCT as a 32bit counter using the bus clock */
	LPC_SCT1->CONFIG = SCT_CONFIG_32BIT_COUNTER | SCT_CONFIG_CLKMODE_BUSCLK;

	/* Initial CTOUT0 state is high */
	LPC_SCT1->OUTPUT = (7 << 0);

	/* The PWM will use a cycle time of (PWMCYCLERATE)Hz based off the bus clock */
	cycleTicks = Chip_Clock_GetSystemClockRate() / PWMCYCLERATE;

	/* Setup for match mode */
	LPC_SCT1->REGMODE_L = 0;

	/* Start at 50% duty cycle */
	dutyCycle = 50;

	/* Setup match counter 0 for the number of ticks in a PWM sweep, event 0
	    will be used with the match 0 count to reset the counter.  */
	LPC_SCT1->MATCH[0].U = cycleTicks;
	LPC_SCT1->MATCHREL[0].U = cycleTicks;
	LPC_SCT1->EVENT[0].CTRL = 0x00001000;
	LPC_SCT1->EVENT[0].STATE = 0xFFFFFFFF;
	LPC_SCT1->LIMIT_L = (1 << 0);

	/* For CTOUT0 to CTOUT2, event 1 is used to clear the output */
	LPC_SCT1->OUT[0].CLR = (1 << 0);
	LPC_SCT1->OUT[1].CLR = (1 << 0);
	LPC_SCT1->OUT[2].CLR = (1 << 0);

	/* Setup PWM0(CTOUT0), PWM1(CTOUT1), and PWM2(CTOUT2) to 50% dutycycle */
	setPWMRate(0, dutyCycle);	/* On match 1 */
	setPWMRate(1, dutyCycle);	/* On match 2 */
	setPWMRate(2, dutyCycle);	/* On match 3 */

	/* Setup event 1 to trigger on match 1 and set CTOUT0 high */
	LPC_SCT1->EVENT[1].CTRL = (1 << 0) | (1 << 12);
	LPC_SCT1->EVENT[1].STATE = 1;
	LPC_SCT1->OUT[0].SET = (1 << 1);

	/* Setup event 2 trigger on match 2 and set CTOUT1 high */
	LPC_SCT1->EVENT[2].CTRL = (2 << 0) | (1 << 12);
	LPC_SCT1->EVENT[2].STATE = 1;
	LPC_SCT1->OUT[1].SET = (1 << 2);

	/* Setup event 3 trigger on match 3 and set CTOUT2 high */
	LPC_SCT1->EVENT[3].CTRL = (3 << 0) | (1 << 12);
	LPC_SCT1->EVENT[3].STATE = 1;
	LPC_SCT1->OUT[2].SET = (1 << 3);

	/* Don't use states */
	LPC_SCT1->STATE_L = 0;

	/* Unhalt the counter to start */
	LPC_SCT1->CTRL_U &= ~(1 << 2);

	setPWMRate(0, dutyCycle);
	setPWMRate(1, dutyCycle);
	setPWMRate(2, dutyCycle);

	// END PWM

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
//				Board_LED_Toggle(0); /* Toggle LED if the TX FIFO is full */
			}

			dutyCycle += countdir;
			if ((dutyCycle  == 0) || (dutyCycle >= 100)) {
				countdir = -countdir;
			}

			/* Update duty cycle in SCT/PWM by change match 1 reload time */
			setPWMRate(0, dutyCycle);
			setPWMRate(1, dutyCycle);
			setPWMRate(2, dutyCycle);

		}
	}

	/* DeInitialize UART0 peripheral */
	NVIC_DisableIRQ(USART0_IRQn);
	Chip_UART0_DeInit(LPC_USART0);

	return 1;
}
