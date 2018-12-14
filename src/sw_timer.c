/*
 * sw_timer.c
 *
 *  Created on: 2018年12月6日
 *      Author: jeremy.hsiao
 */

#include "chip.h"
#include "board.h"
#include "sw_timer.h"

bool 		SysTick_1s_timeout = false;
bool 		SysTick_100ms_timeout = false;
bool 		SysTick_led_7seg_refresh_timeout = false;
bool		SW_delay_timeout = false;
bool		lcd_module_auto_switch_timer_timeout = false;

uint32_t	time_elapse_in_sec=0;
uint32_t	SW_delay_sys_tick_cnt = 0;
uint32_t	lcd_module_auto_switch_in_ms = 0;

uint8_t		sys_tick_1ms_cnt =  SYSTICK_COUNT_VALUE_MS(1);
uint8_t		Counter_1s_cnt_in_100ms  = 10;
uint8_t		Counter_100_ms_cnt_in_ms = 100;

/**
 * @brief    Handle interrupt from SysTick timer
 * @return    Nothing
 */
void SysTick_Handler(void)
{
	// For software delay loop -- decrement each tick
	if(SW_delay_sys_tick_cnt>0)
	{
		SW_delay_sys_tick_cnt--;
	}
	else
	{
		SW_delay_timeout = true;
	}


	// For regular timer
	if(sys_tick_1ms_cnt)
	{
		sys_tick_1ms_cnt--;
	}
	else
	{
		sys_tick_1ms_cnt = SYSTICK_COUNT_VALUE_MS(1);

		// 1ms timeout timer for SysTick_led_7seg_refresh_timeout
		SysTick_led_7seg_refresh_timeout = true;

		// Timer for lcd module page-rotate -- decrement each ms
		if(lcd_module_auto_switch_in_ms>0)
		{
			lcd_module_auto_switch_in_ms--;
		}
		else
		{
			lcd_module_auto_switch_timer_timeout = true;
		}

		// 100ms timeout timer
		if(Counter_100_ms_cnt_in_ms)
		{
			Counter_100_ms_cnt_in_ms--;
		}
		else
		{
			Counter_100_ms_cnt_in_ms = 100;
			SysTick_100ms_timeout = true;

			// 1s timeout timer
			if(Counter_1s_cnt_in_100ms)
			{
				Counter_1s_cnt_in_100ms--;
			}
			else
			{
				Counter_1s_cnt_in_100ms = 10;
				SysTick_1s_timeout = true;
				time_elapse_in_sec++;
			}
		}
	}
}

uint8_t time_elapse_str[5] = {'0','0','0','0', '\0'};

void Update_Elapse_Timer(void)
{
	SysTick_1s_timeout = false;
	if(time_elapse_str[3]++>='9')
	{
		time_elapse_str[3]='0';
		if(time_elapse_str[2]++>='9')
		{
			time_elapse_str[2]='0';
			if(time_elapse_str[1]++>='9')
			{
				time_elapse_str[1]='0';
				if(time_elapse_str[0]++>='9')
				{
					time_elapse_str[0]='0';
				}
			}
		}
	}
}

