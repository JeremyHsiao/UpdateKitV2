/*

	HotSpringBoard main code

 */

#include "chip.h"
#include "board.h"
#include "uart_0_rb.h"
#include "gpio.h"
#include "sw_timer.h"
#include "string.h"
#include "event.h"
#include "res_state.h"
#include "user_if.h"
#include "lcd_module.h"
#include "tpic6b595.h"
#include "cdc_vcom.h"
#include "cdc_main.h"
#include "user_opt.h"
#include "adc.h"
#include "cmd_interpreter.h"

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/
//const char inst1[] = "HotSpring Board";
//const char inst2[] = "HW: V2.0";
//const char inst3[] = "FW: "__DATE__ " " __TIME__;

//#define DEBUG_RX_LOG
#ifdef DEBUG_RX_LOG
#define		UART_RX_LOG_LEN		(4096)
uint8_t 	UART_Rx_log[UART_RX_LOG_LEN];
uint8_t		*UART_Rx_ptr=UART_Rx_log;
uint16_t	UART_TX_LOG_Index = 0;
#endif // #ifdef DEBUG_RX_LOG

uint8_t			vcom_start_to_work_in_sec = 2;
bool			usb_cdc_welcome_message_shown = false;

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/
uint64_t	relay_value;		// public for debug purpose

// for debugging usb-cdc
uint32_t prompt = 0, rdCnt = 0;
#define MAX_USB_RX_BUFF_SIZE	(256)
static uint8_t g_rxBuff[MAX_USB_RX_BUFF_SIZE+1];
//

/*****************************************************************************
 * Private functions
 ****************************************************************************/

void Process_USB_CDC(void)
{
#if defined (_HOT_SPRING_BOARD_V2_)
			if (Check_USB_IsConfigured())
			{
				char 		*command_string_usb, *return_string_ptr_usb;
				char 		input_command_copy[MAX_SERIAL_GETS_LEN+1];			// Extra one is for '\0'
				uint8_t 	*cmd_ptr_usb, *remaining_string_usb;
				CmdExecutionPacket 	cmd_exe_packet_usb;

				rdCnt = vcom_bread(&g_rxBuff[0], 256);
				if (rdCnt)
				{
					g_rxBuff[rdCnt] = '\0';		// Insert a null char at the end of input string
					cmd_ptr_usb = remaining_string_usb = g_rxBuff;
					do
					{
						command_string_usb = serial_gets(*cmd_ptr_usb);
						cmd_ptr_usb++;
						if ((command_string_usb!=(char*)NULL)&&(*command_string_usb!='\0'))
						{
							// echoing input command
							if(CheckEchoEnableStatus())
							{
								vcom_write(remaining_string_usb, (uint32_t)(cmd_ptr_usb-remaining_string_usb));
								CDC_OutputString_with_newline("");
								remaining_string_usb = cmd_ptr_usb;
							}

							// Check if command+paramemter is valid
							strcpy(input_command_copy,command_string_usb);
							if(CommandInterpreter(command_string_usb,&cmd_exe_packet_usb))
							{
								return_string_ptr_usb = input_command_copy;		// this is for get/set nop command
								// Execute if valid
								if(CommandExecution(cmd_exe_packet_usb, &return_string_ptr_usb))
								{
									CDC_OutputString_with_newline(return_string_ptr_usb);	// returning message
								}
								else
								{
									CDC_OutputString_with_newline(return_string_ptr_usb);  // error message
								}
							}
							else
							{
								CDC_OutputString_with_newline("ERROR: Input command is invalid!");  // error message
							}
						}
					}
					while(--rdCnt>0);
					if(CheckEchoEnableStatus())
					{
						CDC_OutputString((char*)remaining_string_usb);
					}
				}
			}
#endif // defined (_HOT_SPRING_BOARD_V2_)
}

/*****************************************************************************
 * Public functions
 ****************************************************************************/

/**
 * @brief	Main HotSpringBoard program body
 * @return	Always returns 1
 */
int main(void)
{
	bool			show_tpic6b595_error = false;

	SystemCoreClockUpdate();
	Init_Value_From_EEPROM();

	/* Enable and setup SysTick Timer at a periodic rate */
	SysTick_Config(SystemCoreClock / SYSTICK_PER_SECOND);

	Init_HotSpringBoard_variables();
	Board_Init();
	Init_GPIO();
#if	defined (_HOT_SPRING_BOARD_V2_)
	show_tpic6b595_error = false;
	// Init TPIC6B595 IC
	Init_Shift_Register_GPIO();
	Enable_Shift_Register_Output(false);
	Clear_Register_Byte();
	Clear_Shiftout_log();
	// init value
	//Enable_Shift_Register_Output(true);
#endif // defined (_HOT_SPRING_BOARD_V2_)
	Init_ADC();
	Chip_ADC_StartSequencer(LPC_ADC, ADC_SEQA_IDX);
	Init_UART0();

#if	defined(_REAL_UPDATEKIT_V2_BOARD_) || defined (_HOT_SPRING_BOARD_V2_)

	Init_LCD_Module_GPIO();
	// This is used for (1) software delay within lcm_sw_init() (2) regular content update lcm_auto_display_refresh_task() in main loop
	Repeat_DownCounter(LCD_MODULE_INTERNAL_DELAY_IN_MS,(LONGER_DELAY_US/1000)+1,TIMER_MS);	// Take longer delay for more tolerance of all possible LCM usages.
	lcm_sw_init();

	lcm_auto_display_init();
	lcm_content_init();
	Update_VR_Page_value_at_beginning();

//	while(sequenceComplete);
//	sequenceComplete = false;
//	adc_voltage = Read_ADC_Voltage();
//	OutputHexValue_with_newline(adc_voltage);
//	if((adc_voltage>6000)||(adc_voltage<4000))
//	{
//		// voltage too-high, trapped endlessly
//		// please switch to over-voltage page
//		lcd_module_display_enable_page(LCM_WELCOME_PAGE);
//		lcm_force_to_display_page(LCM_WELCOME_PAGE);
//		while(1);
//	}

	// V01
	// lcm_page_change_duration_in_sec = DEFAULT_LCM_PAGE_CHANGE_S_WELCOME;
	//V02
	lcm_page_change_duration_in_sec = DEFAULT_VR_BLINKING_IN_S;
	Repeat_DownCounter(LCD_MODULE_PAGE_CHANGE_TIMER_IN_S,lcm_page_change_duration_in_sec,TIMER_S);
	lcd_module_display_enable_page(LCM_WELCOME_PAGE);
//	lcd_module_display_enable_page(LCM_PC_MODE);
//	lcd_module_display_enable_page(LCM_ALL_VR_DISPLAY);
//	lcm_force_to_display_page(LCM_WELCOME_PAGE);

	// Clear events if we want to check it at this state
	EVENT_Button_pressed_debounced = false;
	Countdown_Once(WELCOME_MESSAGE_IN_S,(WELCOME_MESSAGE_DISPLAY_TIME_IN_S),TIMER_S);

#if	defined (_HOT_SPRING_BOARD_V2_)
	cdc_main();
	init_cmd_interpreter();

//	if(SelfTest_Shift_Register())
//	{
//		Clear_Register_Byte();
//		Clear_Shiftout_log();
//		// init value
//		//Enable_Shift_Register_Output(true);
//	}
//	else
//	{
//		show_tpic6b595_error = true;
//		lcd_module_display_enable_only_one_page(LCM_SHIFT_REGISTER_DISPLAY);
//	}
#endif //

#else
	// Setup Virtual Serial com-port
	cdc_main();
	Init_ADC();
	init_cmd_interpreter();
#endif // ! _REAL_UPDATEKIT_V2_BOARD_

	// Endless loop at the moment
	while (1)
	{
		static uint32_t		led = LED_STATUS_G;
		static bool			show_5v_protection_page = false;
		uint32_t 			temp;
		char 				temp_text[10];
		int 				temp_len;

#if defined (_HOT_SPRING_BOARD_V2_)

		while(Get_PinMode_Command()==Get_Max_PinMode())
		{
			// If command_mode is the largest pin_mode, processing only USB_CDC
			Process_USB_CDC();
		}

		if (sequenceComplete)
		{
			sequenceComplete = false;
			temp = Read_ADC_Voltage();
			if(temp!=ADC_SAMPLE_ERROR_VALUE)
			{
				adc_voltage = temp;
			}
			//OutputHexValue_with_newline(adc_voltage);
			Chip_ADC_StartSequencer(LPC_ADC, ADC_SEQA_IDX);

			if((adc_voltage<=6000)&&(adc_voltage>=4000))
			{
				// CHECK PAGE
				if(show_5v_protection_page)
				{
					show_5v_protection_page = false;
					lcd_module_display_enable_only_one_page(LCM_ALL_VR_DISPLAY);
					//lcm_force_to_display_page(LCM_ALL_VR_DISPLAY);
				}

				if((!show_tpic6b595_error)&&(Read_and_Clear_SW_TIMER_Reload_Flag(RELAY_SETUP_HYSTERSIS_IN_100MS)))
				{
					uint32_t	*resistor_ptr;

					resistor_ptr = GetResistorValue();
					Calc_Relay_Value(resistor_ptr,&relay_value);
					if(Setup_Shift_Register_and_Test_64bit(relay_value))
					{
						Latch_Register_Byte_to_Output();
						Enable_Shift_Register_Output(true);// to be removed?
					}
					else
					{
						Enable_Shift_Register_Output(false);
						Clear_Register_Byte();
						Clear_Shiftout_log();
						show_tpic6b595_error = true;
						lcd_module_display_enable_only_one_page(LCM_SHIFT_REGISTER_DISPLAY);
					}
				}
			}
			else if(adc_voltage>=6000)
			{
				// switch PAGE
				//lcm_auto_disable_all_page();
				temp_len = Show_ADC_Voltage_3_Digits(adc_voltage,temp_text);
				lcm_text_buffer_cpy(LCM_INPUT_HIGH_BLINKING,0,10,temp_text,temp_len);
				if(!show_5v_protection_page)
				{
					lcd_module_display_enable_only_one_page(LCM_INPUT_HIGH_BLINKING);
					lcd_module_display_enable_page(LCM_5V_PROTECTION_DISPLAY);
					show_5v_protection_page = true;
				}
				Read_and_Clear_SW_TIMER_Reload_Flag(RELAY_SETUP_HYSTERSIS_IN_100MS);
			}

			else if(adc_voltage<=4000)
			{
				//lcm_auto_disable_all_page();
				temp_len = Show_ADC_Voltage_3_Digits(adc_voltage,temp_text);
				lcm_text_buffer_cpy(LCM_INPUT_LOW_BLINKING,0,10,temp_text,temp_len);
				if(!show_5v_protection_page)
				{
					lcd_module_display_enable_only_one_page(LCM_INPUT_LOW_BLINKING);
					lcd_module_display_enable_page(LCM_5V_PROTECTION_DISPLAY);
					show_5v_protection_page = true;
				}
				Read_and_Clear_SW_TIMER_Reload_Flag(RELAY_SETUP_HYSTERSIS_IN_100MS);
			}
		}
#endif // #if defined (_HOT_SPRING_BOARD_V2_)

#if defined(_REAL_UPDATEKIT_V2_BOARD_) || defined (_HOT_SPRING_BOARD_V2_)
		LED_Status_Set_Value(led);

		if((adc_voltage<=6000)&&(adc_voltage>=4000))
		{
			//
			// UART/ADC Input data processing section
			//
			if(UART_Check_InputBuffer_IsEmpty()==false)
			{
				led ^= LED_STATUS_G;

				// Processing chars according to sys_tick -> faster tick means fewer char for each loop
				temp = ((115200/8)/SYSTICK_PER_SECOND)+1;
				do
				{
					uint8_t	key, bytes;

					bytes = UART0_GetChar(&key);
					if (bytes > 0)
					{
						UART0_PutChar(key);
					}
					else
					{
						break;
					}
				}
				while(--temp>0);
			}

			Process_USB_CDC();

		}

		if((SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk))		// Returns 1 if the SysTick timer counted to 0 since the last read of this register
		{
			// Entering here means SysTick handler has been processed so we could check timeout-event now.

			if (usb_cdc_welcome_message_shown==true)
			{
				if((!show_5v_protection_page)&&(!show_tpic6b595_error))
				{
					UI_Version_02();
				}
				if(If_value_has_been_changed())
				{
					Countdown_Once(EEPROM_UPDATE_TIMER_IN_S,5,TIMER_S);
				}
			}
			else
			{
				// Display welcome message until time-out or button-pressed
				if((!show_5v_protection_page)&&(Read_and_Clear_SW_TIMER_Reload_Flag(WELCOME_MESSAGE_IN_S) || If_any_button_pressed()))
				{
#ifdef _BOARD_DEBUG_SW_
					lcd_module_display_enable_only_one_page(LCM_ALL_SET_2N_VALUE);
//					lcm_force_to_display_page(LCM_ALL_SET_2N_VALUE);
					usb_cdc_welcome_message_shown = true;
					Repeat_DownCounter(RELAY_SETUP_HYSTERSIS_IN_100MS,5,TIMER_100MS);
#else
					lcd_module_display_enable_only_one_page(LCM_ALL_VR_DISPLAY);
//					lcm_force_to_display_page(LCM_ALL_VR_DISPLAY);
					usb_cdc_welcome_message_shown = true;
					Repeat_DownCounter(RELAY_SETUP_HYSTERSIS_IN_100MS,3,TIMER_100MS);
#endif // _BOARD_DEBUG_SW_
				}
			}

			if(Read_and_Clear_SW_TIMER_Reload_Flag(EEPROM_UPDATE_TIMER_IN_S))
			{
				if(Check_if_Resistor_different_from_last_ReadWrite()==true)
				{
					Save_Resistor_Value();
				}
			}

			// Update LCD module display after each lcm command delay (currently about 3ms)
			if(Read_and_Clear_SW_TIMER_Reload_Flag(LCD_MODULE_INTERNAL_DELAY_IN_MS))
			{

				update_resistor_value();

				lcm_auto_display_refresh_task();

				if(Read_and_Clear_SW_TIMER_Reload_Flag(LCD_MODULE_PAGE_CHANGE_TIMER_IN_S))
				{
					lcd_module_display_find_next_enabled_page();
					led ^= LED_STATUS_R;
					LED_Status_Set_Value(led);
				}
			}

				// add self-test here so that it won't affect refreshing rate.
				if(get_tpic6b595_selftest_On())
				{
					SelfTest_Shift_Register();
				}
		}

		//
		// End of Output UI section
		//

#else
		if (Check_USB_IsConfigured())
		{
			/* If VCOM port is opened echo whatever we receive back to host. */
			if (prompt) {
				rdCnt = vcom_bread(&g_rxBuff[0], 256);
				if (rdCnt) {
					vcom_write(&g_rxBuff[0], rdCnt);
				}
			}
			else
			{
				/* Check if host has connected and opened the VCOM port */
				if ((vcom_connected() != 0) && (prompt == 0)) {
					prompt = vcom_write("Hello World!!\r\n", 15);
				}
			}
		}
#endif // #if defined(_REAL_UPDATEKIT_V2_BOARD_) || (_HOT_SPRING_BOARD_V2_)

	}

	/* DeInitialize peripherals before ending */
	DeInit_UART0();
	DeInit_GPIO();
	return 1;
}
