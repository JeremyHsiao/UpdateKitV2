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
#include "lcd_module.h"
#include "sw_timer.h"
#include "string.h"

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
	uint8_t key, dutyCycle = 50, temp;  	/* Start at 50% duty cycle */
	int bytes, countdir = 10;

	SystemCoreClockUpdate();
	/* Enable and setup SysTick Timer at a periodic rate */
	SysTick_Config(SystemCoreClock / SYSTICK_PER_SECOND);

	Board_Init();
	Init_GPIO();
	Init_LED_7seg_GPIO();
	Init_PWM();
	Init_UART0();
	Init_ADC();

	/* Poll the receive ring buffer for the ESC (ASCII 27) key */
	key = 0;
	reset_string_detector();
	lcm_init();
	lcm_auto_display_init();
	//lcm_demo();
	lcm_content_init();

	while (key != 27) {

		/* Sleep until interrupt/sys_tick happens */
		__WFI();

		// Update LCD module display from time-to-time
		lcm_auto_display_refresh_task();

		// Process RS-232 input character
		bytes = UART0_GetChar(&key);
		if (bytes > 0) {
			/* Wrap value back around */
			UART0_PutChar((char)key);

			// To identify 10x OK
			temp=locate_OK_pattern_process(key);
			if(temp==10)
			{
				//OutputHexValue_with_newline(temp);
			 	memcpy((void *)&lcd_module_display_content[3][1][0], "OK is detected! ",LCM_DISPLAY_COL);
				lcm_force_to_display_page(3);
			}

			// To identify @POWERON
			locate_POWERON_pattern_process(key);
			if(Get_POWERON_pattern()==true)
			{
				//OutputString_with_newline("POWER_ON_DETECTED");
				memcpy((void *)&lcd_module_display_content[3][0][0], "POWERON detected", LCM_DISPLAY_COL);
				lcm_force_to_display_page(3);
				Clear_POWERON_pattern();
			}

			// To identify @VER
			locate_VER_pattern_process(key);
			if(Found_VER_string()==true)
			{
				//OutputString("Version:");
				//OutputString_with_newline((char *)Get_VER_string());
				char	*temp_str = (char *) Get_VER_string();
				uint8_t	temp_len = strlen(temp_str);
				if(temp_len<=LCM_DISPLAY_COL)
				{
					strcpy((void *)&lcd_module_display_content[2][1][0], temp_str);
				}
				else
				{
					memcpy((void *)&lcd_module_display_content[2][0][4], temp_str, 12);
					temp_len-=12;
					if(temp_len>LCM_DISPLAY_COL)
					{
						memcpy((void *)&lcd_module_display_content[2][1][0], temp_str+12, LCM_DISPLAY_COL);
					}
					else
					{
						memcpy((void *)&lcd_module_display_content[2][1][0], temp_str+12, temp_len);
						memset((void *)&lcd_module_display_content[2][1][temp_len], ' ', LCM_DISPLAY_COL-temp_len);
					}
				}
				lcm_force_to_display_page(2);
				Clear_VER_string();
			}
		}

		// ADC conversion is triggered every 100ms
		if(SysTick_100ms_timeout==true)
		{
			SysTick_100ms_timeout = false;

			/* Manual start for ADC conversion sequence A */
			Chip_ADC_StartSequencer(LPC_ADC, ADC_SEQA_IDX);
		}

		/* Is an ADC conversion sequence complete? */
		if (sequenceComplete)
		{
			Read_ADC();
		}

		// Refresh each char of 7 Segment LED every 100ms
		if(SysTick_led_7seg_refresh_timeout==true)
		{
			SysTick_led_7seg_refresh_timeout = false;
			refresh_LED_7SEG_periodic_task();
		}

		// This is update every second
		if(SysTick_1s_timeout==true)
		{
			Update_Elapse_Timer(); // Can be removed if this demo is not required
			memcpy((void *)&lcd_module_display_content[0][1][8], time_elapse_str, 4);
			Update_LED_7SEG_Message_Buffer(time_elapse_str,4);

			switch(time_elapse&0x03)
			{
				case 0:
					LED_R_HIGH;
					LED_G_LOW;
					LED_Y_LOW;
					break;
				case 1:
					LED_R_LOW;
					LED_G_HIGH;
					LED_Y_LOW;
					break;
				case 2:
					LED_R_LOW;
					LED_G_LOW;
					LED_Y_HIGH;
					break;
				case 3:
					LED_R_LOW;
					LED_G_LOW;
					LED_Y_LOW;
					break;
			}
		}

		// Process when button is pressed
		if(GPIOGoup0_Int==true)
		{
			char temp_str[LCM_DISPLAY_COL+1];
			int  temp_str_len;

			dutyCycle += countdir;
			if ((dutyCycle  == 0) || (dutyCycle >= 100)) {
				countdir = -countdir;
			}

			/* Update duty cycle in SCT/PWM by change match 1 reload time */
			setPWMRate(0, dutyCycle);
			//setPWMRate(1, dutyCycle);
			//setPWMRate(2, dutyCycle);
			temp_str_len = itoa_10(dutyCycle, temp_str);
			memset((void *)&lcd_module_display_content[1][1][9], ' ', LCM_DISPLAY_COL-9);
			memcpy((void *)&lcd_module_display_content[1][1][9], temp_str, temp_str_len);
			lcm_force_to_display_page(1);
			GPIOGoup0_Int = false;
		}


	}

	/* DeInitialize peripherals before ending */
	DeInit_ADC();
	DeInit_UART0();
	DeInit_PWM();
	DeInit_GPIO();
	return 1;
}
