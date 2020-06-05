/*

	UpdateKitV2 main code

 */

#include "chip.h"
#include "board.h"
#include "uart_0_rb.h"
#include "gpio.h"
#include "lcd_module.h"
#include "sw_timer.h"
#include "string.h"
#include "user_if.h"
#include "event.h"
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

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/

/*****************************************************************************
 * Private functions
 ****************************************************************************/
void Init_Value_From_EEPROM(void)
{
}

void Init_UpdateKitV2_variables(void)
{
	lcm_page_change_duration_in_sec = DEFAULT_LCM_PAGE_CHANGE_S_WELCOME;
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
	SystemCoreClockUpdate();
	Init_Value_From_EEPROM();

	/* Enable and setup SysTick Timer at a periodic rate */
	SysTick_Config(SystemCoreClock / SYSTICK_PER_SECOND);

	//Init_UpdateKitV2_variables();
	Board_Init();
	Init_UART0();
	Init_GPIO();

	Init_LCD_Module_GPIO();
	// This is used for (1) software delay within lcm_sw_init() (2) regular content update lcm_auto_display_refresh_task() in main loop
	Repeat_DownCounter(LCD_MODULE_INTERNAL_DELAY_IN_MS,(LONGER_DELAY_US/1000)+1,TIMER_MS);	// Take longer delay for more tolerance of all possible LCM usages.
	lcm_sw_init();

	lcm_auto_display_init();
	lcm_content_init();

	lcm_page_change_duration_in_sec = DEFAULT_LCM_PAGE_CHANGE_S_WELCOME;
	Repeat_DownCounter(LCD_MODULE_PAGE_CHANGE_TIMER_IN_S,lcm_page_change_duration_in_sec,TIMER_S);

	lcd_module_display_enable_page(LCM_WELCOME_PAGE);
	lcd_module_display_enable_page(LCM_PC_MODE);
	// Clear events if we want to check it at this state
	EVENT_Button_pressed_debounced = false;

	// Endless loop at the moment
	while (1)
	{
		uint8_t 				temp;
		static uint32_t			led = LED_STATUS_G;

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

		if((SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk))		// Returns 1 if the SysTick timer counted to 0 since the last read of this register
		{
			// Entering here means SysTick handler has been processed so we could check timeout-event now.

			if(State_Proc_Button(BUTTON_SRC_ID))
			{
				led ^= LED_STATUS_Y;
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
