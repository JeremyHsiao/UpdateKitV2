/*

	UpdateKitV2 main code

 */

#include "chip.h"
#include "board.h"
#include "string.h"
#include "pwm.h"
#include "uart_0_rb.h"
#include "gpio.h"
#include "adc.h"


/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/

/*****************************************************************************
 * Private functions
 ****************************************************************************/

/*****************************************************************************
 * Public functions
 ****************************************************************************/

/**
 * @brief	Main UpdateKitV2 program body
 * @return	Always returns 1
 */
int main(void)
{
	uint8_t key, dutyCycle = 50;  	/* Start at 50% duty cycle */
	int bytes, countdir = 10;

	SystemCoreClockUpdate();
	Board_Init();
	//Init_UART_PinMux();
	//Board_LED_Set(0, false);
	Init_GPIO();
	Init_PWM();
	Init_UART0();
	Init_ADC();

	/* Poll the receive ring buffer for the ESC (ASCII 27) key */
	key = 0;
	while (key != 27) {
		bytes = UART0_GetChar(&key);
		if (bytes > 0) {
			/* Wrap value back around */
			UART0_PutChar((char)key);
		}

		if(GPIOGoup0_Int==true)
		{
			GPIOGoup0_Int = false;

			dutyCycle += countdir;
			if ((dutyCycle  == 0) || (dutyCycle >= 100)) {
				countdir = -countdir;
			}

			/* Update duty cycle in SCT/PWM by change match 1 reload time */
			setPWMRate(0, dutyCycle);
			setPWMRate(1, dutyCycle);
			setPWMRate(2, dutyCycle);

			/* Manual start for ADC conversion sequence A */
			Chip_ADC_StartSequencer(LPC_ADC, ADC_SEQA_IDX);
		}

		if (thresholdCrossed) {
			thresholdCrossed = false;
		}

		/* Is a conversion sequence complete? */
		if (sequenceComplete) {
			uint32_t rawSample;

			sequenceComplete = false;

			/* Get raw sample data for channels 0-11 */
//			for (j = 0; j < 12; j++) {
//			    rawSample = Chip_ADC_GetDataReg(LPC_ADC, j);
				rawSample = Chip_ADC_GetDataReg(LPC_ADC, 0);

				/* Show some ADC data */
				if ((rawSample & (ADC_DR_OVERRUN | ADC_SEQ_GDAT_DATAVALID)) != 0) {
					OutputHexValue_with_newline(ADC_DR_RESULT(rawSample));
//					DEBUGOUT("Sample value = 0x%x (Data sample %d)\r\n", ADC_DR_RESULT(rawSample), j);
//					DEBUGOUT("Threshold range = 0x%x\r\n", ADC_DR_THCMPRANGE(rawSample));
//					DEBUGOUT("Threshold cross = 0x%x\r\n", ADC_DR_THCMPCROSS(rawSample));
//				}
			}

//			DEBUGOUT("Overrun    = %d\r\n", ((rawSample & ADC_DR_OVERRUN) != 0));
//			DEBUGOUT("Data valid = %d\r\n", ((rawSample & ADC_SEQ_GDAT_DATAVALID) != 0));
//			DEBUGSTR("\r\n");
		}



	}

	/* DeInitialize peripherals before ending */
	DeInit_ADC();
	DeInit_UART0();
	DeInit_PWM();
	DeInit_GPIO();
	return 1;
}
