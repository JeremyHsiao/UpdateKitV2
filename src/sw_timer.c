/*
 * sw_timer.c
 *
 *  Created on: 2018年12月6日
 *      Author: jeremy.hsiao
 */

#include "chip.h"
#include "board.h"
#include "sw_timer.h"

bool		SysTick_flag;
SW_TIMER	sw_timer[SW_TIMER_MAX_NO];

const TICK_UNIT sw_reload_ticks_by_unit[] = {
		SYSTICK_COUNT_VALUE_MS(1),
		SYSTICK_COUNT_VALUE_MS(10),
		SYSTICK_COUNT_VALUE_MS(100),
		SYSTICK_COUNT_VALUE_MS(1000),
};

bool Start_SW_Timer(TIMER_ID timer_no, uint32_t default_count, uint32_t upper_value, TIMER_UNIT_ID unit, bool upcount, bool oneshot)
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

bool Init_SW_Timer(TIMER_ID timer_no, uint32_t default_count, uint32_t upper_value, TIMER_UNIT_ID unit, bool upcount, bool oneshot)
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

bool Set_SW_Timer_Count(TIMER_ID timer_no, uint32_t new_count)
{
	SW_TIMER	*ptr = sw_timer + timer_no;
	ptr->counts = new_count;
	return true;			// always successful at the moment
}

bool Pause_SW_Timer(TIMER_ID timer_no)
{
	SW_TIMER	*ptr = sw_timer + timer_no;
	ptr->running = 0;
	return true;			// always successful at the moment
}

bool Play_SW_Timer(TIMER_ID timer_no)
{
	SW_TIMER	*ptr = sw_timer + timer_no;
	ptr->running = 1;
	return true;			// always successful at the moment
}

uint32_t Read_SW_TIMER_Value(TIMER_ID timer_no)
{
	SW_TIMER	*ptr = sw_timer + timer_no;
	return ptr->counts;
}

bool Read_and_Clear_SW_TIMER_Reload_Flag(TIMER_ID timer_no)
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

void Raise_SW_TIMER_Reload_Flag(TIMER_ID timer_no)
{
	SW_TIMER	*ptr = sw_timer + timer_no;
	ptr->timeup_flag = 1;
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
	SW_TIMER	*timer_ptr = &sw_timer[SW_TIMER_MAX_NO-1];
	do
	{
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
	while(timer_ptr-->sw_timer);		// if 0 (before minus 1) then end of loops
	SysTick_flag = true;
}

/*
//Not used at the moment

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
*/
