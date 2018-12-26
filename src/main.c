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
#include "event.h"

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

	Start_SW_Timer(SYSTEM_TIME_ELAPSE_IN_SEC,0,~1,TIMER_1000MS, true, false);		// System elapse timer: starting from 0 / no-reload-upper-value / 1000ms each count / upcount / not-oneshot
	Board_Init();
	Init_UART0();

//	/* Send initial messages */
//	OutputString_with_newline((char*)inst1);
//	OutputString_with_newline((char*)inst2);

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
	LED_R_setting(0);
	LED_Y_setting(0);
//	LED_Voltage_Current_Refresh_reload = DEFAULT_LED_DATA_CHANGE_SEC - 1;		// 3 second

	init_filtered_input_current();
	init_filtered_input_voltage();
	Start_SW_Timer(SYSTEM_UPDATE_VOLTAGE_CURRENT_DATA_IN_MS,0,(DEFAULT_UPDATE_VOLTAGE_CURRENT_DATA_MS-1),TIMER_MS, false, false);
	// LED display data swap timer: starting from DEFAULT_VOLTAGE_CURRENT_REFRESH_SEC-1 / reload-upper-value / 1000ms each count / downcount / not-oneshot
	Start_SW_Timer(LED_VOLTAGE_CURRENT_DISPLAY_SWAP_IN_SEC,(DEFAULT_LED_DATA_CHANGE_SEC-1),(DEFAULT_LED_DATA_CHANGE_SEC-1),TIMER_1000MS, false, false);
	// count-down, repeated (not one shot timer)
	Start_SW_Timer(LED_REFRESH_EACH_DIGIT_TIMER_MS,0,(DEFAULT_LED_REFRESH_EACH_DIGIT_MS-1),TIMER_MS, false, false);
	// count-down, one-shot timer
	Start_SW_Timer(SYSTEM_STATE_PROC_TIMER,0,0,TIMER_MS, false, true);		// one-shot count down

	reset_string_detector();
//	OutputString_with_newline((char*)inst3);	// Relocate here can use fewer send buffer

	// Endless loop at the moment
	while (1) {
		uint8_t temp;

		/* Sleep if no system tick within one loop; otherwise it means execution time is longer so no need to sleep for next tick */
		if(SysTick_flag)
		{
			SysTick_flag = false;
		}
		else
		{
			__WFI();
		}

		// Update LCD module display after each lcm command delay
		if(lcd_module_wait_finish_timeout==true)
		{
			lcd_module_wait_finish_timeout = false;
			lcm_auto_display_refresh_task();
			lcd_module_wait_finish_in_tick = SYSTICK_COUNT_VALUE_US(250);
		}

		// Processing chars according to sys_tick -> faster tick means fewer char for each loop
		temp = ((115200/8)/SYSTICK_PER_SECOND)+1;
		do
		{
			uint8_t	key, bytes;
			bool	processor_event_detected;

			// Process RS-232 input character
			bytes = UART0_GetChar(&key);
			if (bytes > 0)
			{
				processor_event_detected = UART_input_processor(key);
				if(processor_event_detected!=false)
				{
					break;			// leave loop whenever event has been detected
				}
			}
			else
			{
				break;
			}
		}
		while(--temp>0);

		// ADC conversion is triggered every 100ms
		//if(SysTick_100ms_timeout==true)
		if(Read_and_Clear_SW_TIMER_Reload_Flag(SYSTEM_UPDATE_VOLTAGE_CURRENT_DATA_IN_MS))
		{
			//SysTick_100ms_timeout = false;
			UpdateKitV2_UpdateDisplayValueForADC_Task();

			if(upcoming_system_state==US_OUTPUT_ENABLE)		// it means we are counting down now before really output
			{
				lcd_module_display_content[LCM_REMINDER_BEFORE_OUTPUT][1][10] = (Read_SW_TIMER_Value(SYSTEM_STATE_PROC_TIMER))+'0';	// Timer here should be 1000ms as unit
			}
			else if(upcoming_system_state==US_WAIT_FW_UPGRADE_OK_STRING_UNTIL_TIMEOUT)		// it means we are fw upgrading now
//			if(upcoming_system_state==US_WAIT_FW_UPGRADE_OK_STRING_UNTIL_TIMEOUT)		// it means we are fw upgrading now
			{
				char 	 temp_elapse_str[5+1];
				int 	 temp_elapse_str_len;
				uint8_t		*content1 = &lcd_module_display_content[LCM_FW_UPGRADING_PAGE][0][9],
							*content2 = &lcd_module_display_content[LCM_FW_OK_VER_PAGE][0][9];

				temp_elapse_str_len = itoa_10(Read_SW_TIMER_Value(UPGRADE_ELAPSE_IN_100MS), temp_elapse_str);

				switch (temp_elapse_str_len)
				{
					case 1:
						*content1 = *content2 = ' ';
						content1++; content2++;
						*content1 = *content2 = ' ';
						content1++; content2++;
						*content1 = *content2 = '0';
						break;
					case 2:
						*content1 = *content2 = ' ';
						content1++; content2++;
						*content1 = *content2 = ' ';
						content1++; content2++;
						*content1 = *content2 =  temp_elapse_str[0];
						break;
					case 3:
						*content1 = *content2 = ' ';
						content1++; content2++;
						*content1 = *content2 = temp_elapse_str[0];
						content1++; content2++;
						*content1 = *content2 = temp_elapse_str[1];
						break;
					case 4:
						*content1 = *content2 = temp_elapse_str[0];
						content1++; content2++;
						*content1 = *content2 = temp_elapse_str[1];
						content1++; content2++;
						*content1 = *content2 = temp_elapse_str[2];
						break;
				}
			}

		}

		/* Is an ADC conversion sequence complete? */
		if (sequenceComplete)
		{
			Read_ADC();
			/* Manual start for ADC conversion sequence A */
			Chip_ADC_StartSequencer(LPC_ADC, ADC_SEQA_IDX);
			sequenceComplete=false;
		}


		// Time to switch LED-7Segment content? ==> force to next visible page
		if(Read_and_Clear_SW_TIMER_Reload_Flag(LED_VOLTAGE_CURRENT_DISPLAY_SWAP_IN_SEC))
		{
//			LED_Voltage_Current_Refresh_in_sec_timeout = false;
			LED_7SEG_GoToNextVisiblePage();
		}

		// Refresh each char of 7 Segment LED every 1ms
		if(Read_and_Clear_SW_TIMER_Reload_Flag(LED_REFRESH_EACH_DIGIT_TIMER_MS))
		{
//			SysTick_led_7seg_refresh_timeout = false;
			refresh_LED_7SEG_periodic_task();
		}

		LED_Status_Update_Process();

		EVENT_Button_pressed_debounced = Debounce_Button();
		if(EVENT_Button_pressed_debounced)
		{
			ButtonPressedTask();
		}

		// After processing external input & regular output, process event & system transition
		upcoming_system_state = System_Event_Proc(upcoming_system_state);

		if(Read_and_Clear_SW_TIMER_Reload_Flag(SYSTEM_STATE_PROC_TIMER))
		{
//			System_State_Proc_timer_timeout = false;
			upcoming_system_state = System_State_Proc(upcoming_system_state);
		}

	}

	/* DeInitialize peripherals before ending */
	DeInit_ADC();
	DeInit_UART0();
	DeInit_PWM();
	DeInit_GPIO();
	return 1;
}
