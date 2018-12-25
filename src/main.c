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
//	LED_Voltage_Current_Refresh_reload = DEFAULT_VOLTAGE_CURRENT_REFRESH_SEC - 1;		// 3 second

	init_filtered_input_current();
	init_filtered_input_voltage();
	reset_string_detector();

//	OutputString_with_newline((char*)inst3);	// Relocate here can use fewer send buffer

	// Endless loop at the moment
	while (1) {
		uint8_t temp;

		/* Sleep until interrupt/sys_tick happens */
		__WFI();

		if(System_State_Proc_timer_timeout)
		{
			System_State_Proc_timer_timeout = false;
			upcoming_system_state = System_State_Proc(upcoming_system_state);
		}

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

		if(EVENT_OK_string_confirmed)
		{
			EVENT_OK_string_confirmed = false;

			//OutputHexValue_with_newline(temp);
//			memcpy((void *)&lcd_module_display_content[LCM_DEV_OK_DETECT_PAGE][1][0], "OK is detected! ",LCM_DISPLAY_COL);
//			lcm_force_to_display_page(LCM_DEV_OK_DETECT_PAGE);
			if(upcoming_system_state==US_WAIT_FW_UPGRADE_OK_VER_STRING)		// it means we are fw upgrading now
			{
				upcoming_system_state = US_FW_UPGRADE_DONE;
				System_State_Proc_timer_timeout = true;							// Enter next state at next tick
				LED_G_setting(0xff);
				LED_Y_setting(0);
				LED_R_setting(0);
			}
		}

		if(EVENT_Version_string_confirmed)
		{
			//OutputString("Version:");
			//OutputString_with_newline((char *)Get_VER_string());
			char	*temp_str = (char *) Get_VER_string();
			uint8_t	temp_len = strlen(temp_str);

			if((upcoming_system_state==US_WAIT_FW_UPGRADE_OK_VER_STRING)||				// it means we are fw upgrading now
			   (upcoming_system_state==US_FW_UPGRADE_DONE)||							// it means fw upgrade is ok
			   (upcoming_system_state==US_FW_UPGRADE_DONE_PAGE2))
			{
				memcpy((void *)&lcd_module_display_content[LCM_FW_OK_VER_PAGE][1][3], temp_str, temp_len);
				memset((void *)&lcd_module_display_content[LCM_FW_OK_VER_PAGE][1][3+temp_len], ' ', LCM_DISPLAY_COL-3-(temp_len));
			}

//			if(temp_len<=LCM_DISPLAY_COL)
//			{
//				strcpy((void *)&lcd_module_display_content[LCM_DEV_UPGRADE_VER_PAGE][1][0], temp_str);
//			}
//			else
//			{
//				memcpy((void *)&lcd_module_display_content[LCM_DEV_UPGRADE_VER_PAGE][0][4], temp_str, 12);
//				temp_len-=12;
//				if(temp_len>LCM_DISPLAY_COL)
//				{
//					memcpy((void *)&lcd_module_display_content[LCM_DEV_UPGRADE_VER_PAGE][1][0], temp_str+12, LCM_DISPLAY_COL);
//				}
//				else
//				{
//					memcpy((void *)&lcd_module_display_content[LCM_DEV_UPGRADE_VER_PAGE][1][0], temp_str+12, temp_len);
//					memset((void *)&lcd_module_display_content[LCM_DEV_UPGRADE_VER_PAGE][1][temp_len], ' ', LCM_DISPLAY_COL-temp_len);
//				}
//			}
//			lcm_force_to_display_page(LCM_DEV_UPGRADE_VER_PAGE);
			Clear_VER_string();
			EVENT_Version_string_confirmed = false;
		}

		if(EVENT_POWERON_string_confirmed)
		{

			//OutputString_with_newline("POWER_ON_DETECTED");
//			memcpy((void *)&lcd_module_display_content[LCM_DEV_OK_DETECT_PAGE][0][0], "POWERON detected", LCM_DISPLAY_COL);
//			lcm_force_to_display_page(LCM_DEV_OK_DETECT_PAGE);
			Clear_POWERON_pattern();
			EVENT_POWERON_string_confirmed = false;
		}


		// ADC conversion is triggered every 100ms
		if(SysTick_100ms_timeout==true)
		{
			SysTick_100ms_timeout = false;
			UpdateKitV2_UpdateDisplayValueForADC_Task();

			if(upcoming_system_state==US_OUTPUT_ENABLE)		// it means we are counting down now before really output
			{
				lcd_module_display_content[LCM_REMINDER_BEFORE_OUTPUT][1][10] = (System_State_Proc_timer_in_ms/1000)+'0';
			}

			if(upcoming_system_state==US_WAIT_FW_UPGRADE_OK_VER_STRING)		// it means we are fw upgrading now
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

		if(EVENT_filtered_current_goes_above_threshold)
		{
			EVENT_filtered_current_goes_above_threshold = false;
			if(upcoming_system_state==US_TV_IN_STANDBY)					// it means we are in standby and TV has been plugged
			{
				LED_Y_setting(5);  // 500ms as half-period (toggle period)
				LED_G_setting(0);
				LED_R_setting(0);
				upcoming_system_state = US_OUTPUT_ENABLE;
				System_State_Proc_timer_timeout = true;							// Enter next state at next tick
			}
			if(upcoming_system_state==US_WAIT_FW_UPGRADE_OK_VER_STRING) 				// it means we are fw upgrading now
			{
				LED_Y_setting(5);  // 500ms as half-period (toggle period)
				LED_G_setting(0);
				LED_R_setting(0);
				//Upgrade_elapse_in_100ms = 0;								// reset fw upgrade elapse timer
				Start_SW_Timer(UPGRADE_ELAPSE_IN_100MS,0,~1,TIMER_100MS, true, false);
				// Upgrade elapse timer: starting from 0 / no-reload-upper-value / 1000ms each count / upcount / not-oneshot
			}
		}

		if(EVENT_filtered_current_goes_below_threshold)
		{
			EVENT_filtered_current_goes_below_threshold = false;
			LED_Y_setting(0);
			LED_G_setting(0);
			LED_R_setting(0);
			Clear_OK_pattern_state();
			Clear_VER_string();
			Clear_POWERON_pattern();
			if(upcoming_system_state==US_WAIT_FW_UPGRADE_OK_VER_STRING)				// it means we are fw upgrading now
			{
				// reset current upgrade info but do not overwrite previous one
				lcm_reset_FW_VER_Content();
				upcoming_system_state = US_TV_IN_STANDBY;
				System_State_Proc_timer_timeout = true;							// Enter next state at next tick
			}
			else if((upcoming_system_state==US_FW_UPGRADE_DONE)||(upcoming_system_state==US_FW_UPGRADE_DONE_PAGE2)) // it means fw upgrade is successfully done
			{
				// Move existing FW upgrade info to previous update info page               				                        1234567890123456
			    memcpy((void *)&lcd_module_display_content[LCM_FW_OK_VER_PAGE_PREVIOUS_UPDATE_INFO][0][11], (void *)&lcd_module_display_content[LCM_FW_OK_VER_PAGE][0][11], 3);
				memcpy((void *)&lcd_module_display_content[LCM_FW_OK_VER_PAGE_PREVIOUS_UPDATE_INFO][1][3], (void *)&lcd_module_display_content[LCM_FW_OK_VER_PAGE][1][3], LCM_DISPLAY_COL-3);
				lcm_reset_FW_VER_Content();
				upcoming_system_state = US_TV_IN_STANDBY;
				System_State_Proc_timer_timeout = true;							// Enter next state at next tick
			}
		}
		// Time to switch LED-7Segment content?
		if(LED_Voltage_Current_Refresh_in_sec_timeout==true)
		{
			LED_Voltage_Current_Refresh_in_sec_timeout = false;
			UpdateKitV2_LED_7_ToggleDisplayVoltageCurrent();
			UpdateKitV2_UpdateDisplayValueForADC_Task();
		}

		// Refresh each char of 7 Segment LED every 1ms
		if(SysTick_led_7seg_refresh_timeout==true)
		{
			SysTick_led_7seg_refresh_timeout = false;
			refresh_LED_7SEG_periodic_task();
		}

		LED_Status_Update_Process();

//		if(SysTick_1s_timeout==true)
//		{
//			SysTick_1s_timeout = false;
//			Update_Elapse_Timer();
//			memcpy((void *)&lcd_module_display_content[LCM_DEV_TITLE_PAGE][1][8], time_elapse_str, 4);
////			Update_LED_7SEG_Message_Buffer(time_elapse_str,4);
//		}

		EVENT_Button_pressed_debounced = Debounce_Button();
		if(EVENT_Button_pressed_debounced)
		{
			ButtonPressedTask();
		}

	}

	/* DeInitialize peripherals before ending */
	DeInit_ADC();
	DeInit_UART0();
	DeInit_PWM();
	DeInit_GPIO();
	return 1;
}
