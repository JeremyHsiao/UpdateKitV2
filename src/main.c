/*

	UpdateKitV2 main code

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

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/
const char inst1[] = "HotSpring Board";
const char inst2[] = "HW: V2.0";
const char inst3[] = "FW: "__DATE__ " " __TIME__;

//#define DEBUG_RX_LOG
#ifdef DEBUG_RX_LOG
#define		UART_RX_LOG_LEN		(4096)
uint8_t 	UART_Rx_log[UART_RX_LOG_LEN];
uint8_t		*UART_Rx_ptr=UART_Rx_log;
uint16_t	UART_TX_LOG_Index = 0;
#endif // #ifdef DEBUG_RX_LOG

uint8_t g_rxBuff[VCOM_RX_BUF_SZ];
uint8_t g_txBuff[VCOM_RX_BUF_SZ];
extern int cdc_main(void);

uint8_t			vcom_start_to_work_in_sec = 2;
bool			usb_cdc_welcome_message_shown = false;

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/
uint64_t	relay_value;		// public for debug purpose

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

	/* Enable and setup SysTick Timer at a periodic rate */
	SysTick_Config(SystemCoreClock / SYSTICK_PER_SECOND);

	Init_UpdateKitV2_variables();
	Board_Init();
	Init_UART0();

#ifndef _REAL_UPDATEKIT_V2_BOARD_
	// Setup Virtual Serial com-port and UART0
	// To be updated later: UART0 uses P1_26 & P1_27 and needs to update after final board is available

	cdc_main();

	// Init TPIC6B595 IC
	Init_Shift_Register_GPIO();
	Enable_Shift_Register_Output(false);
	Clear_Register_Byte();
	Clear_Shiftout_log();
#endif // ! _REAL_UPDATEKIT_V2_BOARD_

	Init_GPIO();

	Init_LCD_Module_GPIO();
	// This is used for (1) software delay within lcm_sw_init() (2) regular content update lcm_auto_display_refresh_task() in main loop
	Repeat_DownCounter(LCD_MODULE_INTERNAL_DELAY_IN_MS,(LONGER_DELAY_US/1000)+1,TIMER_MS);	// Take longer delay for more tolerance of all possible LCM usages.
	lcm_sw_init();

	lcm_auto_display_init();
	lcm_content_init();

	// V01
	// lcm_page_change_duration_in_sec = DEFAULT_LCM_PAGE_CHANGE_S_WELCOME;
	//V02
	lcm_page_change_duration_in_sec = DEFAULT_VR_BLINKING_IN_S;
	Repeat_DownCounter(LCD_MODULE_PAGE_CHANGE_TIMER_IN_S,lcm_page_change_duration_in_sec,TIMER_S);
	lcd_module_display_enable_page(LCM_WELCOME_PAGE);
//	lcd_module_display_enable_page(LCM_PC_MODE);
//	lcd_module_display_enable_page(LCM_ALL_VR_DISPLAY);
	lcm_force_to_display_page(LCM_WELCOME_PAGE);

	// Clear events if we want to check it at this state
	EVENT_Button_pressed_debounced = false;
	Countdown_Once(SYSTEM_STATE_PROC_TIMER,(WELCOME_MESSAGE_DISPLAY_TIME_IN_S),TIMER_S);

#ifndef _REAL_UPDATEKIT_V2_BOARD_
	while(vcom_connected()==0); // wait until virtual com connected.
	Countdown_Once(WELCOME_MESSAGE_IN_S,vcom_start_to_work_in_sec,TIMER_S); // Read_and_Clear_SW_TIMER_Reload_Flag
#else
	Countdown_Once(WELCOME_MESSAGE_IN_S,WELCOME_MESSAGE_DISPLAY_TIME_IN_S,TIMER_S); // Read_and_Clear_SW_TIMER_Reload_Flag
#endif // #ifndef _REAL_UPDATEKIT_V2_BOARD_

	// Endless loop at the moment
	while (1)
	{
		static uint32_t		led = LED_STATUS_G;
		uint8_t 			temp;
#ifndef _REAL_UPDATEKIT_V2_BOARD_
		int 				rdCnt = 0, txCnt = 0;
		uint8_t				shift_register_state;
		uint32_t			shift_out_data_log;
#endif // #ifndef _REAL_UPDATEKIT_V2_BOARD_

		LED_Status_Set_Value(led);

#ifdef _REAL_UPDATEKIT_V2_BOARD_

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

#else
		/* Check if host has connected and opened the VCOM port */
		if((usb_cdc_welcome_message_shown!=true)&&(Read_and_Clear_SW_TIMER_Reload_Flag(WELCOME_MESSAGE_IN_S)))
		{
			txCnt = CDC_OutputString(inst1);
			usb_cdc_welcome_message_shown = true;
			Repeat_DownCounter(SHIFT_REGISTER_NEXT_SHIFT_TIMER_IN_S,(4-1),TIMER_S);
			Clear_Register_Byte();
			Clear_Shiftout_log();
			Enable_Shift_Register_Output(true);
		}

		if(usb_cdc_welcome_message_shown)
		{
			/* If VCOM port is opened echo whatever we receive back to host. */
			rdCnt = vcom_bread(&g_rxBuff[0], VCOM_RX_BUF_SZ);
			if (rdCnt)
			{
				uint32_t push_uart_cnt = rdCnt, push_uart_index = 0;
				do
				{
					uint32_t out_cnt;
					out_cnt = OutputData(g_rxBuff+push_uart_index, push_uart_cnt);
					push_uart_index += out_cnt;
					push_uart_cnt -= out_cnt;
				}
				while(push_uart_cnt>0);
			}

			if(vcom_write_precheck())		// if pre-check ok then proceed -- to avoid some waiting delay due to vcom_write is not available.
			{
				txCnt = UART0_GetData(&g_txBuff[0],VCOM_RX_BUF_SZ);
				if (txCnt)
				{
					uint32_t push_cdc_cnt = txCnt, push_cdc_index = 0;
					do
					{
						uint32_t out_cnt;
						out_cnt = vcom_write(g_txBuff+push_cdc_index, push_cdc_cnt);
						push_cdc_index += out_cnt;
						push_cdc_cnt -= out_cnt;
					}
					while(push_cdc_cnt>0);
				}
			}

			if(Read_and_Clear_SW_TIMER_Reload_Flag(SHIFT_REGISTER_NEXT_SHIFT_TIMER_IN_S))
			{
				shift_out_data_log = Test_Shift_Register(shift_register_state++);
				CDC_OutputHexValue_with_newline(shift_out_data_log);
			}
		}
#endif // #ifndef _REAL_UPDATEKIT_V2_BOARD_


		if((SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk))		// Returns 1 if the SysTick timer counted to 0 since the last read of this register
		{
			// Entering here means SysTick handler has been processed so we could check timeout-event now.

			if (usb_cdc_welcome_message_shown==true)
			{
				UI_Version_02();
			}
			else
			{
				// Display welcome message until time-out or button-pressed
				if(Read_and_Clear_SW_TIMER_Reload_Flag(WELCOME_MESSAGE_IN_S) || If_any_button_pressed())
				{
					lcd_module_display_enable_only_one_page(LCM_ALL_VR_DISPLAY);
					lcm_force_to_display_page(LCM_ALL_VR_DISPLAY);
					usb_cdc_welcome_message_shown = true;
					Repeat_DownCounter(RELAY_SETUP_HYSTERSIS_IN_100MS,1,TIMER_100MS);
				}
			}

			// Update LCD module display after each lcm command delay (currently about 3ms)
			if(Read_and_Clear_SW_TIMER_Reload_Flag(LCD_MODULE_INTERNAL_DELAY_IN_MS))
			{

				lcm_auto_display_refresh_task();

				if(Read_and_Clear_SW_TIMER_Reload_Flag(LCD_MODULE_PAGE_CHANGE_TIMER_IN_S))
				{
					lcd_module_display_find_next_enabled_page();
					led ^= LED_STATUS_R;
					LED_Status_Set_Value(led);
				}
			}

			if(Read_and_Clear_SW_TIMER_Reload_Flag(RELAY_SETUP_HYSTERSIS_IN_100MS))
			{
				uint32_t	*resistor_ptr;
				//uint64_t	relay_value;
				//uint32_t	readout_high, readout_low;
				uint32_t	relay_high, relay_low;

				resistor_ptr = GetResistorValue();
				Calc_Relay_Value(resistor_ptr,&relay_value);
				relay_high = (uint32_t)((relay_value>>32)&0xffffffff);
				relay_low  = (uint32_t)(relay_value&0xffffffff);
				Setup_Shift_Register_32it(relay_high);
				Setup_Shift_Register_32it(relay_low);
//				readout_high = Setup_Shift_Register_32it(relay_high);
//				readout_low = Setup_Shift_Register_32it(relay_low);
			}

			//
			// End of Output UI section
			//
		}
	}

	/* DeInitialize peripherals before ending */
	DeInit_UART0();
	DeInit_GPIO();
	return 1;
}
