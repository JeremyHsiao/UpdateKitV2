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
#include "voltage_output.h"
#include "cmd_interpreter.h" // For voltage output branch

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
uint16_t	UART_TX_LOG_Index = 0;
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
	Init_Value_From_EEPROM();
	Init_Value_From_EEPROM_for_voltage_output(); // For voltage output branch

	/* Enable and setup SysTick Timer at a periodic rate */
	SysTick_Config(SystemCoreClock / SYSTICK_PER_SECOND);

//	Start_SW_Timer(SYSTEM_TIME_ELAPSE_IN_SEC,0,~1,TIMER_S, true, false);		// System elapse timer: starting from 0 / no-reload-upper-value / 1000ms each count / upcount / not-oneshot
	Init_UpdateKitV2_variables();
	Board_Init();
	Init_UART0();

//	/* Send initial messages */
//	OutputString_with_newline((char*)inst1);
//	OutputString_with_newline((char*)inst2);

	Init_GPIO();
	Init_LED_7seg_GPIO();

	Repeat_DownCounter(LED_REFRESH_EACH_DIGIT_TIMER_MS,DEFAULT_LED_REFRESH_EACH_DIGIT_MS,TIMER_MS);

	Init_PWM();
	PowerOutputSetting(DEFAULT_POWER_OUTPUT_STEP);
	Init_ADC();

	Init_LCD_Module_GPIO();
	// This is used for (1) software delay within lcm_sw_init() (2) regular content update lcm_auto_display_refresh_task() in main loop
	Repeat_DownCounter(LCD_MODULE_INTERNAL_DELAY_IN_MS,(LONGER_DELAY_US/1000)+1,TIMER_MS);	// Take longer delay for more tolerance of all possible LCM usages.
	lcm_sw_init();

	Repeat_DownCounter(SYSTEM_UPDATE_VOLTAGE_CURRENT_DATA_IN_100MS,DEFAULT_UPDATE_VOLTAGE_CURRENT_DATA_100MS,TIMER_100MS);

	lcm_auto_display_init();
	lcm_content_init();
	lcm_content_init_for_voltage_output(); // // For voltage output branch

	Repeat_DownCounter(LCD_MODULE_PAGE_CHANGE_TIMER_IN_S,lcm_page_change_duration_in_sec,TIMER_S);

	//LED_7seg_self_test();
	LED_Status_Clear_Auto_Toggle(LED_STATUS_ALL);
	LED_Status_Set_Value(0);						// all off

	reset_string_detector();
	init_cmd_interpreter(); // For voltage output branch
//	OutputString_with_newline((char*)inst3);	// Relocate here can use fewer send buffer

	sequenceComplete=false;
	// Force to ADC again before entering main loop
	Chip_ADC_StartSequencer(LPC_ADC, ADC_SEQA_IDX);

	Repeat_DownCounter(LED_VOLTAGE_CURRENT_DISPLAY_SWAP_IN_SEC,DEFAULT_LED_DATA_CHANGE_SEC,TIMER_S);

	// Endless loop at the moment
	while (1)
	{
		uint8_t 				temp;
		UPDATE_STATE			state_before;

		//
		// UART/ADC Input data processing section
		//
		if(UART_Check_InputBuffer_IsEmpty()==false)
		{
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
					UART_TX_LOG_Index++;
					if(UART_Rx_ptr>=(UART_Rx_log+UART_RX_LOG_LEN))
					{
						UART_Rx_ptr = UART_Rx_log;
						UART_TX_LOG_Index=0;
					}
	#endif // #ifdef DEBUG_RX_LOG
					// For voltage output branch
					{
						char *return_str = serial_gets(key);
						if (return_str!=(char*)NULL)
						{
							if(CheckEchoEnableStatus())
								OutputString_with_newline(return_str);					// Echo incoming command (if echo_enabled)

							trimwhitespace(return_str);

							if(CommandInterpreter(return_str,&received_cmd_packet))
								EVENT_UART_CMD_Received = true;
						}
					}
					// For voltage output branch

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
		}

		/* Is an ADC conversion sequence complete? */
		if (sequenceComplete)
		{
			Read_ADC();
			sequenceComplete=false;
		}

		//
		// End of Input data processing section
		//

		if((SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk))		// Returns 1 if the SysTick timer counted to 0 since the last read of this register
		{
			// Entering here means SysTick handler has been processed so we could check timeout-event now.

			if (sequenceComplete==false)
			{
				Chip_ADC_StartSequencer(LPC_ADC, ADC_SEQA_IDX);
			}

			// Button-pressed event
			EVENT_Button_pressed_debounced = Debounce_Button();
			EVENT_2nd_key_pressed_debounced = Debounce_2nd_Key();		// For voltage output branch
			if(EVENT_2nd_key_pressed_debounced)							// For voltage output branch
			{
				if(EVENT_Button_pressed_debounced)						// If both key are pressed -- no effect so clear all events
				{
					EVENT_Button_pressed_debounced = EVENT_2nd_key_pressed_debounced = false;
				}
				else
				{
					EVENT_Button_pressed_debounced = true;				// If only 2nd key are pressed, set both event true (note: if only original event is true then 1st key are pressed.
				}
			}
			// For voltage output branch

			//
			// Output UI section
			//

			// Check whether it is time to flash LED GRY
			LED_Status_Update_Process();

			// Update displaying value of voltage/current (from adc read-back value)
			if(Read_and_Clear_SW_TIMER_Reload_Flag(SYSTEM_UPDATE_VOLTAGE_CURRENT_DATA_IN_100MS))
			{
				UpdateKitV2_UpdateDisplayValueForADC_Task();
				OutputVoltageCurrentViaUART_Task();				// For voltage output branch
			}

			// Refresh each char of 7 Segment LED every 2ms
			if(Read_and_Clear_SW_TIMER_Reload_Flag(LED_REFRESH_EACH_DIGIT_TIMER_MS))
			{
				// Time to switch LED-7Segment content? ==> force to next visible page
				if(Read_and_Clear_SW_TIMER_Reload_Flag(LED_VOLTAGE_CURRENT_DISPLAY_SWAP_IN_SEC))
				{
					LED_7SEG_GoToNextVisiblePage();
				}

				refresh_LED_7SEG_periodic_task();
			}

			// Update LCD module display after each lcm command delay (currently about 3ms)
			if(Read_and_Clear_SW_TIMER_Reload_Flag(LCD_MODULE_INTERNAL_DELAY_IN_MS))
			{
				lcm_auto_display_refresh_task();

				if(Read_and_Clear_SW_TIMER_Reload_Flag(LCD_MODULE_PAGE_CHANGE_TIMER_IN_S))
				{
					lcd_module_display_find_next_enabled_page();
				}
			}
			//
			// End of Output UI section
			//
		}
		//
		// After processing external input & regular output, start to process system event & system transition
		//

		// Processing events not relevant to system_state_processor -- if any
//		Event_Proc_State_Independent();
		Event_Proc_State_Independent_for_voltage_output();			// For voltage output branch

		// temporarily store state for comparison later
		state_before = current_system_proc_state;
		// First processing event according to current state
		if(EVENTS_IN_USE_OR_FLAG()==true)
		{
//			current_system_proc_state = Event_Proc_by_System_State(current_system_proc_state);
			current_system_proc_state = Event_Proc_by_System_State_for_voltage_output(current_system_proc_state);		// For voltage output branch
		}

		// If state unchanged by event (unchanged in most cases), go on with either end-check or regular-task
		if(state_before==current_system_proc_state)
		{
			if(Read_and_Clear_SW_TIMER_Reload_Flag(SYSTEM_STATE_PROC_TIMER)==false)
			{
				// Regular task (unrelated to events) before state time's up
//				current_system_proc_state = System_State_Running_Proc(current_system_proc_state);
				current_system_proc_state = System_State_Running_Proc_for_voltage_output(current_system_proc_state);		// For voltage output branch
			}
			else
			{
				// Time's up and go to do State's End-Check
//				current_system_proc_state = System_State_End_Proc(current_system_proc_state);
				current_system_proc_state = System_State_End_Proc_for_voltage_output(current_system_proc_state);		// For voltage output branch
			}
		}

		// If State has been changed (either by event or by end-check, execute one-time-task of entering a state
		if(state_before!=current_system_proc_state)
		{
			Clear_SW_TIMER_Reload_Flag(SYSTEM_STATE_PROC_TIMER);
//			System_State_Begin_Proc(current_system_proc_state);
			System_State_Begin_Proc_for_voltage_output(current_system_proc_state);		// For voltage output branch
		}

		//
		// End of system process event & system transition
		//
	}

	/* DeInitialize peripherals before ending */
	DeInit_ADC();
	DeInit_UART0();
	DeInit_PWM();
	DeInit_GPIO();
	return 1;
}
