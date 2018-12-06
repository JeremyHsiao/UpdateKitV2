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
uint32_t	time_elapse=0;
uint32_t	SW_delay_cnt = 0;

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
	}

	// For software delay loop -- decrement each tick
	if(SW_delay_cnt>0)
	{
		SW_delay_cnt--;
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
		time_elapse++;
	}
}


