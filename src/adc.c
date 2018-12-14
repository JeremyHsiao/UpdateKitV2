/*
 * @brief LPC11u6x ADC example
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
#include "lcd_module.h"
#include "uart_0_rb.h"
#include "string.h"
#include "adc.h"
#include "UpdateKitV2.h"

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/
bool sequenceComplete, thresholdCrossed;

/*****************************************************************************
 * Private functions
 ****************************************************************************/

/*****************************************************************************
 * Public functions
 ****************************************************************************/

/**
 * @brief	Handle interrupt from ADC sequencer A
 * @return	Nothing
 */
void ADCA_IRQHandler(void)
{
	uint32_t pending;

	pending = Chip_ADC_GetFlags(LPC_ADC);

	/* Sequence A completion interrupt */
	if (pending & ADC_FLAGS_SEQA_INT_MASK) {
		sequenceComplete = true;
	}

	/* Threshold crossing interrupt on ADC input channel */
	if (pending & ADC_FLAGS_THCMP_MASK(BOARD_ADC_CH)) {
		thresholdCrossed = true;
	}

	/* Clear any pending interrupts */
	Chip_ADC_ClearFlags(LPC_ADC, pending);
}

void Init_ADC(void)
{
	/* Setup ADC for 12-bit mode and normal power */
	Chip_ADC_Init(LPC_ADC, 0);

	/* Need to do a calibration after initialization and trim */
	Chip_ADC_StartCalibration(LPC_ADC);
	while (!(Chip_ADC_IsCalibrationDone(LPC_ADC))) {}

	/* Setup for maximum ADC clock rate using sycnchronous clocking */
	Chip_ADC_SetClockRate(LPC_ADC, ADC_MAX_CLOCK_RATE);
	
	/* Optionally, you can setup the ADC to use asycnchronous clocking mode.
	   To enable this, mode use 'LPC_ADC->CTRL |= ADC_CR_ASYNMODE;'.
	   In asycnchronous clocking mode mode, the following functions are
	   used to set and determine ADC rates:
	   Chip_Clock_SetADCASYNCSource();
	   Chip_Clock_SetADCASYNCClockDiv();
	   Chip_Clock_GetADCASYNCRate();
	   clkRate = Chip_Clock_GetADCASYNCRate() / Chip_Clock_GetADCASYNCClockDiv; */

	/* Setup sequencer A for ADC channel 0, EOS interrupt */

	/* Setup a sequencer to do the following:
	   Perform ADC conversion of ADC channels 0 only */
	Chip_ADC_SetupSequencer(LPC_ADC, ADC_SEQA_IDX,(ADC_SEQ_CTRL_CHANSEL(6) | ADC_SEQ_CTRL_CHANSEL(8) | ADC_SEQ_CTRL_MODE_EOS));

	/* ADC input 0 is on PIO1_9 mapped to FUNC3 */
	//Chip_IOCON_PinMuxSet(LPC_IOCON, 1, 9, (IOCON_FUNC3 | IOCON_MODE_INACT | IOCON_ADMODE_EN));

	/* ADC input 8 is on PIO0_12 mapped to FUNC2 */
	Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 12, (IOCON_FUNC2 | IOCON_MODE_INACT | IOCON_ADMODE_EN));
	/* ADC input 6 is on PIO0_14 mapped to FUNC2 */
	Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 14, (IOCON_FUNC2 | IOCON_MODE_INACT | IOCON_ADMODE_EN));

	/* Use higher voltage trim */
	Chip_ADC_SetTrim(LPC_ADC, ADC_TRIM_VRANGE_HIGHV);

	/* Setup threshold 0 low and high values to about 25% and 75% of max */
//	Chip_ADC_SetThrLowValue(LPC_ADC, 0, ((1 * 0xFFF) / 4));
//	Chip_ADC_SetThrHighValue(LPC_ADC, 0, ((3 * 0xFFF) / 4));
	Chip_ADC_SetThrLowValue(LPC_ADC, 0, ((0 * 0xFFF) / 4));
	Chip_ADC_SetThrHighValue(LPC_ADC, 0, ((4 * 0xFFF) / 4));

	/* Clear all pending interrupts */
	Chip_ADC_ClearFlags(LPC_ADC, Chip_ADC_GetFlags(LPC_ADC));

	/* Enable ADC overrun and sequence A completion interrupts */
	Chip_ADC_EnableInt(LPC_ADC, (ADC_INTEN_SEQA_ENABLE | ADC_INTEN_OVRRUN_ENABLE));

	/* Use threshold 0 for ADC channel and enable threshold interrupt mode for
	   channel as crossing */
	Chip_ADC_SelectTH0Channels(LPC_ADC, ADC_THRSEL_CHAN_SEL_THR1(BOARD_ADC_CH));
	Chip_ADC_SetThresholdInt(LPC_ADC, BOARD_ADC_CH, ADC_INTEN_THCMP_CROSSING);

	/* Enable ADC NVIC interrupt */
	NVIC_EnableIRQ(ADC_A_IRQn);

	/* Enable sequencer */
	Chip_ADC_EnableSequencer(LPC_ADC, ADC_SEQA_IDX);
}

void DeInit_ADC(void)
{
	NVIC_EnableIRQ(ADC_A_IRQn);
	Chip_ADC_DeInit(LPC_ADC);
}

void Read_ADC(void)
{
	uint16_t	ADC0_value, ADC1_value;

	// Is an ADC conversion overflow/underflow?
	//if (thresholdCrossed) {
	//	thresholdCrossed = false;
	//}

		uint32_t rawSample, temp_value;
		char temp_str[LCM_DISPLAY_COL];
		int  temp_str_len;

		sequenceComplete = false;

		/* Get raw sample data for channels 6 */
		rawSample = Chip_ADC_GetDataReg(LPC_ADC, 6);
		/* Show some ADC data */
		if ((rawSample & (ADC_DR_OVERRUN | ADC_SEQ_GDAT_DATAVALID)) != 0) {
			ADC0_value = ADC_DR_RESULT(rawSample);
			temp_value = ADC0_value;
			temp_value = (temp_value * 4 * 33) / 4096; // use 0.1V as unit
			SetDisplayVoltageCurrent(temp_value,0);
			temp_str_len = itoa_10(ADC0_value, temp_str);
			memset((void *)&lcd_module_display_content[1][0][5], ' ', (4-temp_str_len));
			memcpy((void *)&lcd_module_display_content[1][0][5+(4-temp_str_len)], temp_str, temp_str_len);
		}
		else
		{
			ADC0_value = ADC_SAMPLE_ERROR_VALUE;
		}

		/* Get raw sample data for channels 8 */
		rawSample = Chip_ADC_GetDataReg(LPC_ADC, 8);
		/* Show some ADC data */
		if ((rawSample & (ADC_DR_OVERRUN | ADC_SEQ_GDAT_DATAVALID)) != 0) {
			ADC1_value = ADC_DR_RESULT(rawSample);
			temp_str_len = itoa_10(ADC1_value, temp_str);
			memset((void *)&lcd_module_display_content[1][0][12], ' ', (4-temp_str_len));
			memcpy((void *)&lcd_module_display_content[1][0][12+(4-temp_str_len)], temp_str, temp_str_len-1);
		}
		else
		{
			ADC1_value = ADC_SAMPLE_ERROR_VALUE;
		}

		// Overtun example code
//			DEBUGOUT("Overrun    = %d\r\n", ((rawSample & ADC_DR_OVERRUN) != 0));
//			DEBUGOUT("Data valid = %d\r\n", ((rawSample & ADC_SEQ_GDAT_DATAVALID) != 0));
//			DEBUGSTR("\r\n");
		//			if(ADC0_value!=ADC_SAMPLE_ERROR_VALUE)
		//			{
		//				OutputString("ADC_6:");
		//				OutputHexValue_with_newline(ADC0_value);
		//			}
		//			if(ADC1_value!=ADC_SAMPLE_ERROR_VALUE)
		//			{
		//				OutputString("ADC_8:");
		//				OutputHexValue_with_newline(ADC1_value);
		//			}

}
