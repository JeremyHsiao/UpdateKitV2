/*
 * @brief State Configurable Timer (SCT) PWM example
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
#include "pwm.h"

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/
/* Saved tick count used for a PWM cycle */
static uint32_t cycleTicks;

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

/*****************************************************************************
 * Public functions
 ****************************************************************************/
/**
 * @brief	Handle interrupt from State Configurable Timer
 * @return	Nothing
 */
void SCT_IRQHandler(void)
{
	/* Clear the Interrupt */
	Chip_SCT_ClearEventFlag(LPC_SCT1, SCT_EVT_0);
}

// Added/modified by Jeremy
void Init_PWM(void)
{
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

	/* Setup match counter 0 for the number of ticks in a PWM sweep, event 0
	    will be used with the match 0 count to reset the counter.  */
	LPC_SCT1->MATCH[0].U = cycleTicks;
	LPC_SCT1->MATCHREL[0].U = cycleTicks;
	LPC_SCT1->EVENT[0].CTRL = 0x00001000;
	LPC_SCT1->EVENT[0].STATE = 0xFFFFFFFF;
	LPC_SCT1->LIMIT_L = (1 << 0);

	/* For CTOUT0 to CTOUT2, event 1 is used to clear the output */
	LPC_SCT1->OUT[0].CLR = (1 << 0);
	//LPC_SCT1->OUT[1].CLR = (1 << 0);
	//LPC_SCT1->OUT[2].CLR = (1 << 0);

	/* Setup PWM0(CTOUT0), PWM1(CTOUT1), and PWM2(CTOUT2) to 50% dutycycle */
	setPWMRate(0, default_duty_cycle);	/* On match 1 */
	//setPWMRate(1, default_duty_cycle);	/* On match 2 */
	//setPWMRate(2, default_duty_cycle);	/* On match 3 */

	/* Setup event 1 to trigger on match 1 and set CTOUT0 high */
	LPC_SCT1->EVENT[1].CTRL = (1 << 0) | (1 << 12);
	LPC_SCT1->EVENT[1].STATE = 1;
	LPC_SCT1->OUT[0].SET = (1 << 1);

	/* Setup event 2 trigger on match 2 and set CTOUT1 high */
	//LPC_SCT1->EVENT[2].CTRL = (2 << 0) | (1 << 12);
	//LPC_SCT1->EVENT[2].STATE = 1;
	//LPC_SCT1->OUT[1].SET = (1 << 2);

	/* Setup event 3 trigger on match 3 and set CTOUT2 high */
	//LPC_SCT1->EVENT[3].CTRL = (3 << 0) | (1 << 12);
	//LPC_SCT1->EVENT[3].STATE = 1;
	//LPC_SCT1->OUT[2].SET = (1 << 3);

	/* Don't use states */
	LPC_SCT1->STATE_L = 0;

	/* Unhalt the counter to start */
	LPC_SCT1->CTRL_U &= ~(1 << 2);
}

void DeInit_PWM(void)
{
	NVIC_DisableIRQ(GINT0_IRQn);
	Chip_SCT_DeInit(LPC_SCT1);
}
