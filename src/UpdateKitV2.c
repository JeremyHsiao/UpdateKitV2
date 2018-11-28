/*

	UpdateKitV2 main code

 */

#include "chip.h"
#include "board.h"
#include "pwm.h"
#include "uart_0_rb.h"
#include "gpio.h"
#include "adc.h"
#include "string_detector.h"

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/
bool SysTick_100ms_timeout = false;
bool SysTick_1s_timeout = false;

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/

/*****************************************************************************
 * Private functions
 ****************************************************************************/
#define 	SYSTICK_PER_SECOND			(10000)		// 100us
#define     SYSTICK_COUNT_VALUE_MS(x)	((SYSTICK_PER_SECOND*x/1000)-1)

uint32_t	sys_tick_100_ms_cnt = SYSTICK_COUNT_VALUE_MS(100);
/**
 * @brief    Handle interrupt from SysTick timer
 * @return    Nothing
 */
void SysTick_Handler(void)
{
	SysTick_1s_timeout = true;

	// 100ms timeout timer
	if(sys_tick_100_ms_cnt)
	{
		sys_tick_100_ms_cnt--;
	}
	else
	{
		sys_tick_100_ms_cnt = SYSTICK_COUNT_VALUE_MS(100);
		SysTick_100ms_timeout = true;
	}
}

/*****************************************************************************
 * Public functions
 ****************************************************************************/

/**
 * @brief	Main UpdateKitV2 program body
 * @return	Always returns 1
 */
int main(void)
{
	uint8_t key, dutyCycle = 50, temp;  	/* Start at 50% duty cycle */
	int bytes, countdir = 10;

	SystemCoreClockUpdate();
	Board_Init();
	//Init_UART_PinMux();
	//Board_LED_Set(0, false);
	Init_GPIO();
	Init_PWM();
	Init_UART0();
	Init_ADC();

	/* Enable and setup SysTick Timer at a periodic rate */
	SysTick_Config(SystemCoreClock / SYSTICK_PER_SECOND);

	/* Poll the receive ring buffer for the ESC (ASCII 27) key */
	key = 0;
	while (key != 27) {

		/* Sleep until something happens */
		__WFI();

		bytes = UART0_GetChar(&key);
		if (bytes > 0) {
			/* Wrap value back around */
			UART0_PutChar((char)key);

			temp=locate_OK_pattern_process(key);
			if(temp==10)
			{
				OutputHexValue_with_newline(temp);
			}
		}

		if(SysTick_100ms_timeout==true)
		{
			SysTick_100ms_timeout = false;

			dutyCycle += countdir;
			if ((dutyCycle  == 0) || (dutyCycle >= 100)) {
				countdir = -countdir;
			}

			/* Update duty cycle in SCT/PWM by change match 1 reload time */
			setPWMRate(0, dutyCycle);
			setPWMRate(1, dutyCycle);
			setPWMRate(2, dutyCycle);
		}

		if(GPIOGoup0_Int==true)
		{
			GPIOGoup0_Int = false;

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

			/* Get raw sample data for channels 6 */
			rawSample = Chip_ADC_GetDataReg(LPC_ADC, 6);
			/* Show some ADC data */
			if ((rawSample & (ADC_DR_OVERRUN | ADC_SEQ_GDAT_DATAVALID)) != 0) {
				OutputString("ADC_6:");
				OutputHexValue_with_newline(ADC_DR_RESULT(rawSample));
			}

			/* Get raw sample data for channels 8 */
			rawSample = Chip_ADC_GetDataReg(LPC_ADC, 8);
			/* Show some ADC data */
			if ((rawSample & (ADC_DR_OVERRUN | ADC_SEQ_GDAT_DATAVALID)) != 0) {
				OutputString("ADC_8:");
				OutputHexValue_with_newline(ADC_DR_RESULT(rawSample));
			}

			// Overtun example code
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
