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

uint32_t	sys_tick_1s_cnt = SYSTICK_COUNT_VALUE_MS(1000);
uint32_t	sys_tick_100_ms_cnt = SYSTICK_COUNT_VALUE_MS(100);
uint32_t	sys_tick_1ms_cnt = SYSTICK_COUNT_VALUE_MS(1);

/**
 * @brief    Handle interrupt from SysTick timer
 * @return    Nothing
 */
void SysTick_Handler(void)
{
	// 100ms timeout timer
	if(sys_tick_100_ms_cnt)
	{
		sys_tick_100_ms_cnt--;
	}
	else
	{
		sys_tick_100_ms_cnt = SYSTICK_COUNT_VALUE_MS(100);
		SysTick_100ms_timeout = true;
	}

	// 1ms timeout timer for SysTick_led_7seg_refresh_timeout
	// temporarily 1ms delay timer
	if(sys_tick_1ms_cnt)
	{
		sys_tick_1ms_cnt--;
	}
	else
	{
		sys_tick_1ms_cnt = SYSTICK_COUNT_VALUE_MS(1);
		SysTick_led_7seg_refresh_timeout = true;

		// For lcd module page-rotate -- decrement each ms
		if(lcd_module_auto_switch_in_ms>0)
		{
			lcd_module_auto_switch_in_ms--;
		}
		else
		{
			lcd_module_auto_switch_timer_timeout = true;
		}

	}

	// 1s
	if(sys_tick_1s_cnt)
	{
		sys_tick_1s_cnt--;
	}
	else
	{
		sys_tick_1s_cnt = SYSTICK_COUNT_VALUE_MS(1000);
		SysTick_1s_timeout = true;
		time_elapse_in_sec++;
	}

	// For software delay loop -- decrement each tick
	if(SW_delay_sys_tick_cnt>0)
	{
		SW_delay_sys_tick_cnt--;
	}
	else
	{
		SW_delay_timeout = true;
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

