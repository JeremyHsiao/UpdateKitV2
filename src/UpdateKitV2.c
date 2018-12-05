/*

	UpdateKitV2 main code

 */

#include "chip.h"
#include "board.h"
#include "pwm.h"
#include "uart_0_rb.h"
#include "gpio.h"
#include "LED_7seg.h"
#include "adc.h"
#include "string_detector.h"

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/
bool 		SysTick_1s_timeout = false;
bool 		SysTick_100ms_timeout = false;
bool 		SysTick_led_7seg_refresh_timeout = false;
uint32_t	time_elapse=0;
/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/

/*****************************************************************************
 * Private functions
 ****************************************************************************/
#define		ADC_SAMPLE_ERROR_VALUE		(0xffff)	// length is 16 bits
#define 	SYSTICK_PER_SECOND			(4000)		// 4000 ticks per == 250us each tick
#define     SYSTICK_COUNT_VALUE_MS(x)	((SYSTICK_PER_SECOND*x/1000)-1)

uint32_t	sys_tick_1s_cnt = SYSTICK_COUNT_VALUE_MS(1000);
uint32_t	sys_tick_100_ms_cnt = SYSTICK_COUNT_VALUE_MS(100);
uint32_t	sys_tick_1ms_cnt = SYSTICK_COUNT_VALUE_MS(1);

/**
 * @brief    Handle interrupt from SysTick timer
 * @return    Nothing
 */
void SysTick_Handler(void)
{
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

	// 10ms timeout timer for SysTick_led_7seg_refresh_timeout
	if(sys_tick_1ms_cnt)
	{
		sys_tick_1ms_cnt--;
	}
	else
	{
		sys_tick_1ms_cnt = SYSTICK_COUNT_VALUE_MS(1);
		SysTick_led_7seg_refresh_timeout = true;
	}

	// 1s
	if(sys_tick_1s_cnt)
	{
		sys_tick_1s_cnt--;
	}
	else
	{
		sys_tick_1s_cnt = SYSTICK_COUNT_VALUE_MS(1000);
		SysTick_1s_timeout = true;
		time_elapse++;
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
	uint8_t 	time_elapse_str[5] = {'0','0','0','0', '\0'};
	uint16_t	ADC0_value, ADC1_value;

	SystemCoreClockUpdate();
	Board_Init();
	Init_GPIO();
	Init_LED_7seg_GPIO();
	Init_PWM();
	Init_UART0();
	Init_ADC();

	/* Enable and setup SysTick Timer at a periodic rate */
	SysTick_Config(SystemCoreClock / SYSTICK_PER_SECOND);

	/* Poll the receive ring buffer for the ESC (ASCII 27) key */
	key = 0;
	ADC0_value = ADC_SAMPLE_ERROR_VALUE;
	ADC1_value = ADC_SAMPLE_ERROR_VALUE;
	reset_string_detector();

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

			locate_POWERON_pattern_process(key);
			if(Get_POWERON_pattern()==true)
			{
				OutputString_with_newline("POWER_ON_DETECTED");
				Clear_POWERON_pattern();
			}

			locate_VER_pattern_process(key);
			if(Found_VER_string()==true)
			{
				OutputString("Version:");
				OutputString_with_newline((char *)Get_VER_string());
			}
		}

		if(SysTick_100ms_timeout==true)
		{
			SysTick_100ms_timeout = false;

			/* Manual start for ADC conversion sequence A */
			Chip_ADC_StartSequencer(LPC_ADC, ADC_SEQA_IDX);

		}

		if(SysTick_led_7seg_refresh_timeout==true)
		{
			SysTick_led_7seg_refresh_timeout = false;
			refresh_LED_7SEG_periodic_task();
		}

		if(SysTick_1s_timeout==true)
		{
			SysTick_1s_timeout = false;
			if(time_elapse_str[3]++>='9')
			{
				time_elapse_str[3]='0';
				if(time_elapse_str[2]++>='9')
				{
					time_elapse_str[2]='0';
					if(time_elapse_str[1]++>='9')
					{
						time_elapse_str[1]='0';
						if(time_elapse_str[0]++>='9')
						{
							time_elapse_str[0]='0';
						}
					}
				}
			}
			Update_LED_7SEG_Message_Buffer(time_elapse_str,4);
		}

		if(GPIOGoup0_Int==true)
		{

			dutyCycle += countdir;
			if ((dutyCycle  == 0) || (dutyCycle >= 100)) {
				countdir = -countdir;
			}

			/* Update duty cycle in SCT/PWM by change match 1 reload time */
			setPWMRate(0, dutyCycle);
			setPWMRate(1, dutyCycle);
			setPWMRate(2, dutyCycle);

			GPIOGoup0_Int = false;

			if(ADC0_value!=ADC_SAMPLE_ERROR_VALUE)
			{
				OutputString("ADC_6:");
				OutputHexValue_with_newline(ADC0_value);
			}
			if(ADC1_value!=ADC_SAMPLE_ERROR_VALUE)
			{
				OutputString("ADC_8:");
				OutputHexValue_with_newline(ADC1_value);
			}

		}

		// Is an ADC conversion overflow/underflow?
		//if (thresholdCrossed) {
		//	thresholdCrossed = false;
		//}

		/* Is an ADC conversion sequence complete? */
		if (sequenceComplete) {
			uint32_t rawSample;

			sequenceComplete = false;

			/* Get raw sample data for channels 6 */
			rawSample = Chip_ADC_GetDataReg(LPC_ADC, 6);
			/* Show some ADC data */
			if ((rawSample & (ADC_DR_OVERRUN | ADC_SEQ_GDAT_DATAVALID)) != 0) {
				ADC0_value = ADC_DR_RESULT(rawSample);
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
			}
			else
			{
				ADC1_value = ADC_SAMPLE_ERROR_VALUE;
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
