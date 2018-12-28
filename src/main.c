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

//#define DEBUG_RX_LOG
#ifdef DEBUG_RX_LOG
#define		UART_RX_LOG_LEN		(4096)
uint8_t 	UART_Rx_log[UART_RX_LOG_LEN];
uint8_t		*UART_Rx_ptr=UART_Rx_log;
#endif // #ifdef DEBUG_RX_LOG

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

	Start_SW_Timer(SYSTEM_TIME_ELAPSE_IN_SEC,0,~1,TIMER_S, true, false);		// System elapse timer: starting from 0 / no-reload-upper-value / 1000ms each count / upcount / not-oneshot
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

	Start_SW_Timer(LCD_MODULE_INTERNAL_DELAY_IN_MS,0,0,TIMER_MS, false, false);		// repeat count down
	lcm_sw_init();
	lcm_auto_display_init();
	lcm_content_init();
	Start_SW_Timer(LCD_MODULE_PAGE_CHANGE_TIMER_IN_S,(lcm_page_change_duration_in_sec-1),(lcm_page_change_duration_in_sec-1),TIMER_S, false, false);  // countdown / repeated

	//LED_7seg_self_test();
	LED_Status_Clear_Auto_Toggle(LED_STATUS_ALL);
	LED_Status_Set_Value(0);						// all off
//	LED_Voltage_Current_Refresh_reload = DEFAULT_LED_DATA_CHANGE_SEC - 1;		// 3 second

	init_filtered_input_current();
	init_filtered_input_voltage();
	Start_SW_Timer(SYSTEM_UPDATE_VOLTAGE_CURRENT_DATA_IN_MS,0,(DEFAULT_UPDATE_VOLTAGE_CURRENT_DATA_MS-1),TIMER_MS, false, false);
	// LED display data swap timer: starting from DEFAULT_VOLTAGE_CURRENT_REFRESH_SEC-1 / reload-upper-value / 1000ms each count / downcount / not-oneshot
	Start_SW_Timer(LED_VOLTAGE_CURRENT_DISPLAY_SWAP_IN_SEC,(DEFAULT_LED_DATA_CHANGE_SEC-1),(DEFAULT_LED_DATA_CHANGE_SEC-1),TIMER_S, false, false);
	// count-down, repeated (not one shot timer)
	Start_SW_Timer(LED_REFRESH_EACH_DIGIT_TIMER_MS,0,(DEFAULT_LED_REFRESH_EACH_DIGIT_MS-1),TIMER_MS, false, false);
	// count-down, one-shot timer
	Start_SW_Timer(SYSTEM_STATE_PROC_TIMER,0,0,TIMER_MS, false, true);		// one-shot count down

	reset_string_detector();
//	OutputString_with_newline((char*)inst3);	// Relocate here can use fewer send buffer

	// Endless loop at the moment
	while (1)
	{
		uint8_t 		temp;
		UPDATE_STATE	state_before;

		/* Sleep if no system tick within one loop; otherwise it means execution time is longer so no need to sleep for next tick */
		if(SysTick_flag)
		{
			SysTick_flag = false;
		}
		else
		{
			__WFI();
		}

		//
		// Input data processing section
		//

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
#ifdef DEBUG_RX_LOG
				*UART_Rx_ptr++ = key;
				if(UART_Rx_ptr>=(UART_Rx_log+UART_RX_LOG_LEN))
				{
					UART_Rx_ptr = UART_Rx_log;
				}
#endif // #ifdef DEBUG_RX_LOG
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

		/* Is an ADC conversion sequence complete? */
		if (sequenceComplete)
		{
			Read_ADC();
			/* Manual start for ADC conversion sequence A */
			Chip_ADC_StartSequencer(LPC_ADC, ADC_SEQA_IDX);
			sequenceComplete=false;
		}

		// Update displaying value of voltage/current (from adc read-back value)
		if(Read_and_Clear_SW_TIMER_Reload_Flag(SYSTEM_UPDATE_VOLTAGE_CURRENT_DATA_IN_MS))
		{
			UpdateKitV2_UpdateDisplayValueForADC_Task();
		}

		// Button-pressed event
		EVENT_Button_pressed_debounced = Debounce_Button();

		//
		// End of Input data processing section
		//

		//
		// After processing external input & regular output, system process event & system transition
		//

		// Processing events not relevant to system_state_processor -- if any
		Event_Proc_State_Independent();

		// temporarily store state for comparison later
		state_before = current_system_proc_state;
		// First processing event according to current state
		if(EVENTS_IN_USE_OR_FLAG()==true)
		{
			current_system_proc_state = Event_Proc_by_System_State(current_system_proc_state);
		}

		// If state unchanged by event (unchanged in most cases), go on with either end-check or regular-task
		if(state_before==current_system_proc_state)
		{
			if(Read_and_Clear_SW_TIMER_Reload_Flag(SYSTEM_STATE_PROC_TIMER)==false)
			{
				// Regular task (unrelated to events) before state time's up
				current_system_proc_state = System_State_Running_Proc(current_system_proc_state);
			}
			else
			{
				// Time's up and go to do State's End-Check
				current_system_proc_state = System_State_End_Proc(current_system_proc_state);
			}
		}

		// If State has been changed (either by event or by end-check, execute one-time-task of entering a state
		if(state_before!=current_system_proc_state)
		{
			Clear_SW_TIMER_Reload_Flag(SYSTEM_STATE_PROC_TIMER);
			System_State_Begin_Proc(current_system_proc_state);
		}

		//
		// End of system process event & system transition
		//

		//
		// Output UI section
		//

		// Check whether it is time to flash LED GRY
		LED_Status_Update_Process();

		// Time to switch LED-7Segment content? ==> force to next visible page
		if(Read_and_Clear_SW_TIMER_Reload_Flag(LED_VOLTAGE_CURRENT_DISPLAY_SWAP_IN_SEC))
		{
			LED_7SEG_GoToNextVisiblePage();
		}

		// Refresh each char of 7 Segment LED every 1ms
		if(Read_and_Clear_SW_TIMER_Reload_Flag(LED_REFRESH_EACH_DIGIT_TIMER_MS))
		{
			refresh_LED_7SEG_periodic_task();
		}

		// Update LCD module display after each lcm command delay
		if(Read_and_Clear_SW_TIMER_Reload_Flag(LCD_MODULE_INTERNAL_DELAY_IN_MS))
		{
//			lcd_module_wait_finish_timeout = false;
			lcm_auto_display_refresh_task();
			//lcd_module_wait_finish_in_tick = SYSTICK_COUNT_VALUE_US(250);
		}
		//
		// End of Output UI section
		//

	}

	/* DeInitialize peripherals before ending */
	DeInit_ADC();
	DeInit_UART0();
	DeInit_PWM();
	DeInit_GPIO();
	return 1;
}
