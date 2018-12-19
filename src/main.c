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
#include "UpdateKitV2.h"

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/
const char inst1[] = "TPV UpdateKit";
const char inst2[] = "HW: V2.0";
const char inst3[] = "FW: "__DATE__ " " __TIME__;

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
	SystemCoreClockUpdate();
	/* Enable and setup SysTick Timer at a periodic rate */
	SysTick_Config(SystemCoreClock / SYSTICK_PER_SECOND);

	Board_Init();
	Init_UART0();

	/* Send initial messages */
	OutputString_with_newline(inst1);
	OutputString_with_newline(inst2);

	Init_GPIO();
	Init_LED_7seg_GPIO();
	Init_PWM();
	PowerOutputSetting(DEFAULT_POWER_OUTPUT_STEP);

	Init_ADC();
	Init_LCD_Module_GPIO();

	lcm_sw_init();
	lcm_auto_display_init();
	//lcm_demo();
	lcm_content_init();
	//LED_7seg_self_test();
	LED_G_setting(0);
	LED_R_setting(0xff);
	LED_Y_setting(5);
	LED_Voltage_Current_Refresh_reload = DEFAULT_VOLTAGE_CURRENT_REFRESH_SEC;		// 2 second

	init_filtered_input_current();
	reset_string_detector();

	OutputString_with_newline(inst3);	// Relocate here can use fewer send buffer

	// Endless loop at the moment
	while (1) {
		uint8_t temp;

		/* Sleep until interrupt/sys_tick happens */
		__WFI();

		// Update LCD module display after each lcm command delay
		if(lcd_module_wait_finish_timeout==true)
		{
			lcd_module_wait_finish_timeout = false;
			lcm_auto_display_refresh_task();
			lcd_module_wait_finish_in_tick = SYSTICK_COUNT_VALUE_US(250);
		}

		// Processing at most 4-char each ticks
		temp = ((115200/8)/SYSTICK_PER_SECOND)+1;
		do
		{
			uint8_t	key, bytes;

			// Process RS-232 input character
			bytes = UART0_GetChar(&key);
			if (bytes > 0)
			{
				uint8_t	temp_ok_cnt;

				/* Wrap value back around */
				UART0_PutChar((char)key);

				// To identify 10x OK
				temp_ok_cnt=locate_OK_pattern_process(key);
				if(temp_ok_cnt==10)
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
			else
			{
				break;
			}
		}
		while(--temp>0);

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
			UpdateKitV2_LED_7_UpdateDisplayValueAfterADC_Task();
		}

		// Time to switch LED-7Segment content?
		if(LED_Voltage_Current_Refresh_in_sec_timeout==true)
		{
			LED_Voltage_Current_Refresh_in_sec_timeout = false;
			UpdateKitV2_LED_7_ToggleDisplayVoltageCurrent();
			UpdateKitV2_LED_7_UpdateDisplayValueAfterADC_Task();
		}

		// Refresh each char of 7 Segment LED every 1ms
		if(SysTick_led_7seg_refresh_timeout==true)
		{
			SysTick_led_7seg_refresh_timeout = false;
			refresh_LED_7SEG_periodic_task();
		}

		LED_Status_Update_Process();

		if(SysTick_1s_timeout==true)
		{
			SysTick_1s_timeout = false;
			Update_Elapse_Timer();
			memcpy((void *)&lcd_module_display_content[0][1][8], time_elapse_str, 4);
//			Update_LED_7SEG_Message_Buffer(time_elapse_str,4);
		}

		// Process when button is pressed
//		if(GPIOGoup0_Int==true)
		if(Debounce_Button()==true)
		{
			ButtonPressedTask();
//			char temp_str[LCM_DISPLAY_COL+1];
//			int  temp_str_len;
//
//			dutyCycle += countdir;
//			if ((dutyCycle  == 0) || (dutyCycle >= 100)) {
//				countdir = -countdir;
//			}
//
//			/* Update duty cycle in SCT/PWM by change match 1 reload time */
//			setPWMRate(0, dutyCycle);
//			//setPWMRate(1, dutyCycle);
//			//setPWMRate(2, dutyCycle);
//			temp_str_len = itoa_10(dutyCycle, temp_str);
//			memcpy((void *)&lcd_module_display_content[1][1][9], temp_str, temp_str_len);
//			memset((void *)&lcd_module_display_content[1][1][9+temp_str_len], ' ', LCM_DISPLAY_COL-9-temp_str_len);
//			lcm_force_to_display_page(1);
//			GPIOGoup0_Int = false;
		}

	}

	/* DeInitialize peripherals before ending */
	DeInit_ADC();
	DeInit_UART0();
	DeInit_PWM();
	DeInit_GPIO();
	return 1;
}
