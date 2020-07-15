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
#include "cdc_main.h"
#include "user_opt.h"

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

uint8_t			vcom_start_to_work_in_sec = 2;
bool			usb_cdc_welcome_message_shown = false;

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/
uint64_t	relay_value;		// public for debug purpose

// for debugging usb-cdc
uint32_t prompt = 0, rdCnt = 0;
static uint8_t g_rxBuff[256];
//

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

#if	defined(_REAL_UPDATEKIT_V2_BOARD_) || defined (_HOT_SPRING_BOARD_V2_)

	Init_GPIO();

	// Hot Spring Board only
#if	defined (_HOT_SPRING_BOARD_V2_)
	cdc_main();
	// Init TPIC6B595 IC
	Init_Shift_Register_GPIO();
	Enable_Shift_Register_Output(false);
	Clear_Register_Byte();
	Clear_Shiftout_log();
	// init value
	//Enable_Shift_Register_Output(true);
#endif //
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

#else
	// Setup Virtual Serial com-port
	cdc_main();
#endif // ! _REAL_UPDATEKIT_V2_BOARD_

	// Endless loop at the moment
	while (1)
	{
		static uint32_t		led = LED_STATUS_G;
		uint8_t 			temp;

#if defined(_REAL_UPDATEKIT_V2_BOARD_) || defined (_HOT_SPRING_BOARD_V2_)

		LED_Status_Set_Value(led);

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


#if defined (_HOT_SPRING_BOARD_V2_)
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
#endif // defined (_HOT_SPRING_BOARD_V2_)

		if((SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk))		// Returns 1 if the SysTick timer counted to 0 since the last read of this register
		{
			// Entering here means SysTick handler has been processed so we could check timeout-event now.

			if (usb_cdc_welcome_message_shown==true)
			{
				UI_Version_02();
				if(If_value_has_been_changed())
				{
					Countdown_Once(EEPROM_UPDATE_TIMER_IN_S,5,TIMER_S);
				}
			}
			else
			{
				// Display welcome message until time-out or button-pressed
				if(Read_and_Clear_SW_TIMER_Reload_Flag(WELCOME_MESSAGE_IN_S) || If_any_button_pressed())
				{
#ifdef _BOARD_DEBUG_SW_
					lcd_module_display_enable_only_one_page(LCM_ALL_SET_2N_VALUE);
					lcm_force_to_display_page(LCM_ALL_SET_2N_VALUE);
					usb_cdc_welcome_message_shown = true;
					Repeat_DownCounter(RELAY_SETUP_HYSTERSIS_IN_100MS,5,TIMER_100MS);
#else
					lcd_module_display_enable_only_one_page(LCM_ALL_VR_DISPLAY);
					lcm_force_to_display_page(LCM_ALL_VR_DISPLAY);
					usb_cdc_welcome_message_shown = true;
					Repeat_DownCounter(RELAY_SETUP_HYSTERSIS_IN_100MS,5,TIMER_100MS);
#endif // _BOARD_DEBUG_SW_
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

#if defined (_HOT_SPRING_BOARD_V2_)
			if(Read_and_Clear_SW_TIMER_Reload_Flag(RELAY_SETUP_HYSTERSIS_IN_100MS))
			{
				uint32_t	*resistor_ptr;
				//uint64_t	relay_value;
				uint32_t	readout_high, readout_low;
				uint32_t	relay_high, relay_low;

				resistor_ptr = GetResistorValue();
				Calc_Relay_Value(resistor_ptr,&relay_value);
				relay_high = (uint32_t)((relay_value>>32)&~(0UL));
				relay_low  = (uint32_t)(relay_value&~(0UL));
				Setup_Shift_Register_32it(relay_high);
				Setup_Shift_Register_32it(relay_low);
				readout_high = Setup_Shift_Register_32it(relay_high);
				readout_low = Setup_Shift_Register_32it(relay_low);
				if((readout_high!=relay_high)||(readout_low!=relay_low))
				{
					relay_low = relay_high; // dummy line for breakpoint
					// need to debug
				}
				Latch_Register_Byte_to_Output();
				Enable_Shift_Register_Output(true);// to be removed?
			}
#endif // #if defined (_HOT_SPRING_BOARD_V2_)

			if(Read_and_Clear_SW_TIMER_Reload_Flag(EEPROM_UPDATE_TIMER_IN_S))
			{
				if(Check_if_Resistor_different_from_last_ReadWrite()==true)
				{
					Save_Resistor_Value();
				}
			}
			//
			// End of Output UI section
			//
		}

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
