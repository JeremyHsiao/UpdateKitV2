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
//bool		lcd_g_toggle_timeout = false;
//bool		lcd_r_toggle_timeout = false;
//bool		lcd_y_toggle_timeout = false;
//bool		LED_Voltage_Current_Refresh_in_sec_timeout = false;
bool		lcd_module_wait_finish_timeout = false;
bool		System_State_Proc_timer_timeout = false;

//uint32_t	time_elapse_in_sec=0;
//uint32_t	Upgrade_elapse_in_100ms;			// select 100ms because we like to have accuracy +/- 100ms
uint32_t	SW_delay_sys_tick_cnt = 0;
uint16_t	lcd_module_auto_switch_in_ms = 0;
//uint8_t		LED_Voltage_Current_Refresh_in_sec = 0;
uint8_t		lcd_module_wait_finish_in_tick = 0;
uint32_t	System_State_Proc_timer_in_ms = 0;

//uint32_t	led_g_toggle_timer_in_100ms = 0;
//uint32_t	led_r_toggle_timer_in_100ms = 0;
//uint32_t	led_y_toggle_timer_in_100ms = 0;

// Auto-reload value
//uint32_t	led_g_toggle_timer_reload = 0;
//uint32_t	led_r_toggle_timer_reload = 0;
//uint32_t	led_y_toggle_timer_reload = 0;
uint8_t		LED_Voltage_Current_Refresh_reload = 0;

uint8_t		sys_tick_1ms_cnt =  SYSTICK_COUNT_VALUE_MS(1);
uint8_t		Counter_1s_cnt_in_100ms  = (10-1);
uint8_t		Counter_100_ms_cnt_in_ms = (100-1);

const TICK_UNIT sw_reload_ticks_by_unit[] = {
		SYSTICK_COUNT_VALUE_MS(1),
		SYSTICK_COUNT_VALUE_MS(10),
		SYSTICK_COUNT_VALUE_MS(100),
		SYSTICK_COUNT_VALUE_MS(1000),
};

SW_TIMER	sw_timer[SW_TIMER_MAX_NO];

//typedef	struct {
//	uint32_t	counts;
//	uint32_t	reload_value;
//	TICK_UNIT	ticks;
//	unsigned	unit:2;				// 0: 1ms, 1: 10ms, 2: 100ms, 3: 1000ms
//	unsigned	count_up:1;
//	unsigned	oneshot:1;
//	unsigned	running:1;
//} SW_TIMER;

bool Start_SW_Timer(uint8_t timer_no, uint32_t default_count, uint32_t upper_value, uint8_t unit, bool upcount, bool oneshot)
{
	SW_TIMER	*ptr = sw_timer + timer_no;
	ptr->counts = default_count;
	ptr->reload_value = upper_value;
	ptr->ticks = sw_reload_ticks_by_unit[unit];
	ptr->unit = unit;
	ptr->count_up = (upcount)?1:0;
	ptr->oneshot = (oneshot)?1:0;
	ptr->running = 1;
	ptr->timeup_flag = 0;
	return true;			// always successful at the moment
}

bool Init_SW_Timer(uint8_t timer_no, uint32_t default_count, uint32_t upper_value, uint8_t unit, bool upcount, bool oneshot)
{
	SW_TIMER	*ptr = sw_timer + timer_no;
	ptr->counts = default_count;
	ptr->reload_value = upper_value;
	ptr->ticks = sw_reload_ticks_by_unit[unit];
	ptr->unit = unit;
	ptr->count_up = (upcount)?1:0;
	ptr->oneshot = (oneshot)?1:0;
	ptr->running = 0;
	ptr->timeup_flag = 0;
	return true;			// always successful at the moment
}

bool Set_SW_Timer_Count(uint8_t timer_no, uint32_t new_count)
{
	SW_TIMER	*ptr = sw_timer + timer_no;
	ptr->counts = new_count;
	return true;			// always successful at the moment
}

bool Pause_SW_Timer(uint8_t timer_no)
{
	SW_TIMER	*ptr = sw_timer + timer_no;
	ptr->running = 0;
	return true;			// always successful at the moment
}

bool Play_SW_Timer(uint8_t timer_no)
{
	SW_TIMER	*ptr = sw_timer + timer_no;
	ptr->running = 1;
	return true;			// always successful at the moment
}

uint32_t Read_SW_TIMER_Value(uint8_t timer_no)
{
	SW_TIMER	*ptr = sw_timer + timer_no;
	return ptr->counts;
}

bool Read_and_Clear_SW_TIMER_Reload_Flag(uint8_t timer_no)
{
	SW_TIMER	*ptr = sw_timer + timer_no;
	if(ptr->timeup_flag)
	{
		ptr->timeup_flag = 0;
		return	true;
	}
	else
	{
		return	false;
	}
}

void Clear_SW_TIMER_Reload_Flag(uint8_t timer_no)
{
	SW_TIMER	*ptr = sw_timer + timer_no;
	ptr->timeup_flag = 0;
}

/**
 * @brief    Handle interrupt from SysTick timer
 * @return    Nothing
 */
void SysTick_Handler(void)
{
	// Experimenting new way of timer
	uint8_t		timer_index = SW_TIMER_MAX_NO-1;

	do
	{
		SW_TIMER	*timer_ptr = &sw_timer[timer_index];
		if(timer_ptr->running)
		{
			if(timer_ptr->ticks)
			{
				timer_ptr->ticks--;
			}
			else
			{
				if(timer_ptr->count_up)
				{
					// Up-count
					if(timer_ptr->counts < timer_ptr->reload_value)
					{
						timer_ptr->ticks = sw_reload_ticks_by_unit[timer_ptr->unit];
						timer_ptr->counts++;
					}
					else
					{
						timer_ptr->timeup_flag = 1;
						if(timer_ptr->oneshot)
							timer_ptr->running = 0;
						else
						{
							timer_ptr->ticks = sw_reload_ticks_by_unit[timer_ptr->unit];
							timer_ptr->counts = 0;
						}
					}
				}
				else			// down-counter: from reload_value until 0
				{
					// Down-count
					if(timer_ptr->counts)
					{
						timer_ptr->ticks = sw_reload_ticks_by_unit[timer_ptr->unit];
						timer_ptr->counts--;
					}
					else
					{
						timer_ptr->timeup_flag = 1;
						if(timer_ptr->oneshot)
							timer_ptr->running = 0;
						else
						{
							timer_ptr->ticks = sw_reload_ticks_by_unit[timer_ptr->unit];
							timer_ptr->counts = timer_ptr->reload_value;
						}
					}
				}
			}
		}
	}
	while(timer_index-->0);		// if 0 (before minus 1) then end of loops

	// For software delay loop -- decrement each tick
	if(SW_delay_sys_tick_cnt>0)
	{
		SW_delay_sys_tick_cnt--;
	}
	else
	{
		SW_delay_timeout = true;
	}

	if(lcd_module_wait_finish_in_tick>0)
	{
		lcd_module_wait_finish_in_tick--;
	}
	else
	{
		lcd_module_wait_finish_timeout = true;
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


		// Timer for System_State_Proc() -- decrement each ms
		if(System_State_Proc_timer_in_ms>0)
		{
			System_State_Proc_timer_in_ms--;
		}
		else
		{
			System_State_Proc_timer_timeout = true;
		}


		// 100ms timeout timer
		if(Counter_100_ms_cnt_in_ms)
		{
			Counter_100_ms_cnt_in_ms--;
		}
		else
		{
			Counter_100_ms_cnt_in_ms = (100-1);		// 0....99 total is 100
			SysTick_100ms_timeout = true;

			// Timer for LED G -- decrement each 100ms
//			if(led_g_toggle_timer_in_100ms>0)
//			{
//				led_g_toggle_timer_in_100ms--;
//			}
//			else
//			{
//				led_g_toggle_timer_in_100ms = led_g_toggle_timer_reload;
//				lcd_g_toggle_timeout = true;
//			}

//			// Timer for LED R -- decrement each 100ms
//			if(led_r_toggle_timer_in_100ms>0)
//			{
//				led_r_toggle_timer_in_100ms--;
//			}
//			else
//			{
//				led_r_toggle_timer_in_100ms = led_r_toggle_timer_reload;
//				lcd_r_toggle_timeout = true;
//			}

//			// Timer for LED Y -- decrement each 100ms
//			if(led_y_toggle_timer_in_100ms>0)
//			{
//				led_y_toggle_timer_in_100ms--;
//			}
//			else
//			{
//				led_y_toggle_timer_in_100ms = led_y_toggle_timer_reload;
//				lcd_y_toggle_timeout = true;
//			}
//			Upgrade_elapse_in_100ms++;

			// 1s timeout timer
			if(Counter_1s_cnt_in_100ms)
			{
				Counter_1s_cnt_in_100ms--;
			}
			else
			{
				Counter_1s_cnt_in_100ms = (10-1);	// 0...9 total is 10
				SysTick_1s_timeout = true;
//				time_elapse_in_sec++;

//				// Timer for lcd module page-rotate -- decrement each ms
//				if(LED_Voltage_Current_Refresh_in_sec>0)
//				{
//					LED_Voltage_Current_Refresh_in_sec--;
//				}
//				else
//				{
//					LED_Voltage_Current_Refresh_in_sec = LED_Voltage_Current_Refresh_reload;
//					LED_Voltage_Current_Refresh_in_sec_timeout = true;
//				}
			}
		}
	}
}

uint8_t time_elapse_str[5] = {'0','0','0','0', '\0'};

void Update_Elapse_Timer(void)
{
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

