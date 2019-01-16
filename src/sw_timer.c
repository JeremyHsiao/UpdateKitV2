/*
 * sw_timer.c
 *
 *  Created on: 2018年12月6日
 *      Author: jeremy.hsiao
 */

#include "chip.h"
#include "board.h"
#include "sw_timer.h"

//bool		SysTick_flag;
SW_TIMER	sw_timer[SW_TIMER_MAX_NO];

//#define TIMER_IMPLEMENTATION_V3
#ifdef TIMER_IMPLEMENTATION_V3
typedef	struct {
	int32_t		counter;				// running(1) + timeout(1) + counter (30) so 1,342,177.275 if 8000 ticks each second.
	int32_t	reload;					// repeat(1) + downcount(1) +  reload_value
} SW_TIMER;

#define		SW_TIMER_RUNNING_MASK			(1L<<31)
#define		SW_TIMER_TIMEOUT_MASK			(1L<<30)
#define		SW_TIMER_COUNTER_FLAG_MASK		(SW_TIMER_RUNNING_MASK|SW_TIMER_TIMEOUT_MASK)	// 2 MSB is flag
#define		SW_TIMER_RUNNING_TRUE			(0)
#define		SW_TIMER_RUNNING_FALSE			(SW_TIMER_RUNNING_MASK)
#define		SW_TIMER_TIMEOUT_TRUE			(SW_TIMER_TIMEOUT_MASK)
#define		SW_TIMER_TIMEOUT_FALSE			(0)

#define		SW_TIMER_REPEAT_MASK			(1L<<31)
#define		SW_TIMER_COUNTDOWN_MASK			(1L<<30)
#define		SW_TIMER_RELOAD_FLAG_MASK		(SW_TIMER_REPEAT_MASK|SW_TIMER_COUNTDOWN_MASK)	// 2 MSB is flag
#define		SW_TIMER_REPEAT_TRUE			(0)
#define		SW_TIMER_REPEAT_FALSE			(SW_TIMER_REPEAT_MASK)
#define		SW_TIMER_COUNTDOWN_TRUE			(0)
#define		SW_TIMER_COUNTDOWN_FALSE		(SW_TIMER_COUNTDOWN_MASK)

#define		SW_TIMER_MAX_COUNT_VALUE		((~SW_TIMER_COUNTER_FLAG_MASK)/(SYSTICK_PER_SECOND/1000))	// Max count-value (in unit of ms)
#define		SW_TIMER_TICK_LEN				(3)															// 8000 ticks as 1S == 2^3 ticks each 1ms
#define		SW_TIMER_TICK_MASK				((1L<<(SW_TIMER_TICK_LEN))-1)								// 0b111 == 0b1000-1
#define		SW_TIMER_MAX_TICK_VALUE			(SW_TIMER_TICK_MASK)										// Max count-value (in unit of ms)

//#define

static inline uint32_t Conversion_to_internal_count_value(uint32_t input_up_count_value, uint32 input_reload_value)
{
	return (input_reload_value-input_up_count_value);
}

bool Start_SW_Timer(TIMER_ID timer_no, uint32_t init_count, uint32_t reload_value, TIMER_UNIT_ID unit, bool upcount, bool oneshot)
{
	SW_TIMER	*ptr = sw_timer + timer_no;
	uint32_t	temp;

	ptr->reload_value = (int32_t)(((reload_value>SW_TIMER_MAX_COUNT_VALUE)?SW_TIMER_MAX_COUNT_VALUE:reload_value)|(oneshot?SW_TIMER_REPEAT_FALSE:SW_TIMER_REPEAT_TRUE));

	if(upcount)
	{
		ptr->reload_value |= (oneshot?SW_TIMER_REPEAT_FALSE:SW_TIMER_REPEAT_TRUE);
		if(init_count>reload_value)			// to avoid init_value is larger than upper value
		{
			init_count = reload_value;
		}
		// convert to down-count value
		init_count = Conversion_to_internal_count_value(init_count, reload_value);
	}
	else
	{

	}
	ptr->counts = (int32_t)(((init_count>SW_TIMER_MAX_COUNT_VALUE)?SW_TIMER_MAX_COUNT_VALUE:init_count)<<SW_TIMER_TICK_LEN) | SW_TIMER_TICK_MASK ;

	temp = ((default_count>SW_TIMER_MAX_COUNT_VALUE)?SW_TIMER_MAX_COUNT_VALUE:default_count)<<SW_TIMER_TICK_LEN;

	ptr->counts = (int32_t)((default_count>SW_TIMER_MAX_COUNT_VALUE)?SW_TIMER_MAX_COUNT_VALUE:default_count);
	ptr->reload_value = (int32_t)(  ((default_count>SW_TIMER_MAX_COUNT_VALUE)?SW_TIMER_MAX_COUNT_VALUE:default_count) |
									(oneshot?SW_TIMER_REPEAT_FLAG_VALUE:0) |
									(upcount?SW_TIMER_COUNTDOWN_FLAG_VALUE:0) );
	return true;			// always successful at the moment
}

bool Init_SW_Timer(TIMER_ID timer_no, uint32_t default_count, uint32_t upper_value, TIMER_UNIT_ID unit, bool upcount, bool oneshot)
{
	SW_TIMER	*ptr = sw_timer + timer_no;
	ptr->counts = (int32_t)(((default_count>SW_TIMER_MAX_COUNT_VALUE)?SW_TIMER_MAX_COUNT_VALUE:default_count)|SW_TIMER_RUNNING_FALSE));
	ptr->reload_value = (int32_t)(  ((default_count>SW_TIMER_MAX_COUNT_VALUE)?SW_TIMER_MAX_COUNT_VALUE:default_count)|(oneshot?SW_TIMER_REPEAT_FLAG_VALUE:0));
	if(upcount){
		ptr->reload_value |= SW_TIMER_COUNTDOWN_FALSE;

	}
	return true;			// always successful at the moment
}

bool Set_SW_Timer_Count(TIMER_ID timer_no, uint32_t new_count)
{
	SW_TIMER	*timer_ptr = sw_timer + timer_no;
	timer_ptr->counter = (new_count>SW_TIMER_MAX_COUNT_VALUE):SW_TIMER_MAX_COUNT_VALUE:new_count;
	return true;			// always successful at the moment
}

bool Pause_SW_Timer(TIMER_ID timer_no)
{
	SW_TIMER	*timer_ptr = sw_timer + timer_no;
	timer_ptr->counter &= ~SW_TIMER_RUNNING_MASK;
	return true;			// always successful at the moment
}

bool Play_SW_Timer(TIMER_ID timer_no)
{
	SW_TIMER	*timer_ptr = sw_timer + timer_no;
	timer_ptr->counter |= SW_TIMER_RUNNING_MASK;
	return true;			// always successful at the moment
}

uint32_t Read_SW_TIMER_Value(TIMER_ID timer_no)
{
	SW_TIMER	*timer_ptr = sw_timer + timer_no;
	uint32_t	value = timer_ptr->counter;

	return 		(value>0)?value:0;
}

void Raise_SW_TIMER_Reload_Flag(TIMER_ID timer_no)
{
	SW_TIMER	*ptr = sw_timer + timer_no;
	timer_ptr->reload |= SW_TIMER_TIMEOUT_MASK;
}

void Clear_SW_TIMER_Reload_Flag(uint8_t timer_no)
{
	SW_TIMER	*ptr = sw_timer + timer_no;
	timer_ptr->reload &= ~SW_TIMER_TIMEOUT_MASK;
}

bool Read_and_Clear_SW_TIMER_Reload_Flag(TIMER_ID timer_no)
{
	SW_TIMER	*ptr = sw_timer + timer_no;
	if((timer_ptr->reload & SW_TIMER_TIMEOUT_MASK) != SW_TIMER_TIMEOUT_FALSE)
	{
		Clear_SW_TIMER_Reload_Flag(timer_no);
		return	true;
	}
	else
	{
		return	false;
	}
}

/*
#define		SW_TIMER_RUNNING_MASK			(1L<<31)
#define		SW_TIMER_TIMEOUT_MASK			(1L<<30)
#define		SW_TIMER_COUNTER_FLAG_MASK		(SW_TIMER_RUNNING_MASK|SW_TIMER_TIMEOUT_MASK)	// 2 MSB is flag
#define		SW_TIMER_RUNNING_TRUE			(0)
#define		SW_TIMER_RUNNING_FALSE			(SW_TIMER_RUNNING_MASK)
#define		SW_TIMER_TIMEOUT_TRUE			(SW_TIMER_TIMEOUT_MASK)
#define		SW_TIMER_TIMEOUT_FALSE			(0)

#define		SW_TIMER_REPEAT_MASK			(1L<<31)
#define		SW_TIMER_COUNTDOWN_MASK			(1L<<30)
#define		SW_TIMER_RELOAD_FLAG_MASK		(SW_TIMER_REPEAT_MASK|SW_TIMER_COUNTDOWN_MASK)	// 2 MSB is flag
#define		SW_TIMER_REPEAT_TRUE			(0)
#define		SW_TIMER_REPEAT_FALSE			(SW_TIMER_REPEAT_MASK)
#define		SW_TIMER_COUNTDOWN_TRUE			(0)
#define		SW_TIMER_COUNTDOWN_FALSE		(SW_TIMER_COUNTDOWN_MASK)

#define		SW_TIMER_MAX_COUNT_VALUE		((~SW_TIMER_COUNTER_FLAG_MASK)/(SYSTICK_PER_SECOND/1000))	// Max count-value (in unit of ms)
#define		SW_TIMER_TICK_LEN				(3)															// 8000 ticks as 1S == 2^3 ticks each 1ms
#define		SW_TIMER_TICK_MASK				((1L<<(SW_TIMER_TICK_LEN))-1)								// 0b111 == 0b1000-1
#define		SW_TIMER_MAX_TICK_VALUE			(SW_TIMER_TICK_MASK)										// Max count-value (in unit of ms)

// Simple version is that all timers are down-counting
// For elapse-timer, please add a wrapping function.
typedef	struct {
	int32_t		counter;				// running(1) + counter (31) so 2,684,354.55 if 8000 ticks each second.
	int32_t		reload_value;			// repeat(1) + reload_value (29) + timeout(1); note: LSB flags are all set to 1 when reloading.
} SW_TIMER;
*/

void SysTick_Handler(void)
{
	// Experimenting new way of timer
	SW_TIMER	*timer_ptr = &sw_timer[SW_TIMER_MAX_NO-1];
	do
	{
		if(timer_ptr->counter >= 0)		// If > 0 means running & counter is non-zero
		{
			timer_ptr->counter--;
			if (timer_ptr->counter < 0)	// If < 0 after minus 1 --> means time-up
			{
				timer_ptr->reload |= SW_TIMER_TIMEOUT_MASK;

				// if repeat, reload value
				if ( ( timer_ptr->reload & SW_TIMER_REPEAT_MASK ) == SW_TIMER_REPEAT_TRUE )	// in fact bit_value == 0 means repeat
				{
					timer_ptr->counter = timer_ptr->reload;		// assumed that repeat-flag is 0 at MSB & 1 at LSB (just setup by OR SW_TIMER_TIMEOUT_MASK)
				}
			}
		}
	}
	while(timer_ptr-->sw_timer);		// if 0 (before minus 1) then end of loops
	SysTick_flag = true;
}

#else

const TICK_UNIT sw_reload_ticks_by_unit[] = {
		SYSTICK_COUNT_VALUE_MS(1),
		SYSTICK_COUNT_VALUE_MS(10),
		SYSTICK_COUNT_VALUE_MS(100),
		SYSTICK_COUNT_VALUE_MS(1000),
};


static inline uint32_t Covert_to_count(uint32_t timer_value, TIMER_UNIT_ID unit, bool upcount)
{
	if(upcount==false)
	{
		return ((timer_value+1)*(sw_reload_ticks_by_unit[unit])-1);
	}
	else
	{
		return ((timer_value)*(sw_reload_ticks_by_unit[unit]));
	}
}

static inline uint32_t Covert_to_timer_value(uint32_t count, TIMER_UNIT_ID unit, bool upcount)
{
	if(upcount==false)
	{
		return ((count+1)/(sw_reload_ticks_by_unit[unit]));
		// NOTE: When setting count-down, the input count is minus 1 but when reading back no-need to minus one
	}
	else
	{
		return ((count)/(sw_reload_ticks_by_unit[unit]));
	}
}

bool Start_SW_Timer(TIMER_ID timer_no, uint32_t default_count, uint32_t upper_value, TIMER_UNIT_ID unit, bool upcount, bool oneshot)
{
	SW_TIMER	*ptr = sw_timer + timer_no;
	ptr->counts = Covert_to_count(default_count,unit,upcount);
	ptr->reload_value = Covert_to_count(upper_value,unit,upcount);
//	ptr->ticks = sw_reload_ticks_by_unit[unit];
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
	ptr->counts = Covert_to_count(default_count,unit,upcount);
	ptr->reload_value = Covert_to_count(upper_value,unit,upcount);
//	ptr->ticks = sw_reload_ticks_by_unit[unit];
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
	ptr->counts = Covert_to_count(new_count,ptr->unit,ptr->count_up);
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
	return Covert_to_timer_value(ptr->counts,ptr->unit,ptr->count_up);
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
//			if(timer_ptr->ticks)
//			{
//				timer_ptr->ticks--;
//			}
//			else
//			{
				if(timer_ptr->count_up)
				{
					// Up-count
					if(timer_ptr->counts < timer_ptr->reload_value)
					{
	//					timer_ptr->ticks = sw_reload_ticks_by_unit[timer_ptr->unit];
						timer_ptr->counts++;
					}
					else
					{
						timer_ptr->timeup_flag = 1;
						if(timer_ptr->oneshot)
							timer_ptr->running = 0;
						else
						{
	//						timer_ptr->ticks = sw_reload_ticks_by_unit[timer_ptr->unit];
							timer_ptr->counts = 0;
						}
					}
				}
				else			// down-counter: from reload_value until 0
				{
					// Down-count
					if(timer_ptr->counts)
					{
	//					timer_ptr->ticks = sw_reload_ticks_by_unit[timer_ptr->unit];
						timer_ptr->counts--;
					}
					else
					{
						timer_ptr->timeup_flag = 1;
						if(timer_ptr->oneshot)
							timer_ptr->running = 0;
						else
						{
//							timer_ptr->ticks = sw_reload_ticks_by_unit[timer_ptr->unit];
							timer_ptr->counts = timer_ptr->reload_value;
						}
					}
				}
//			}
		}
	}
	while(timer_ptr-->sw_timer);		// if 0 (before minus 1) then end of loops
//	SysTick_flag = true;
}

#endif // #ifdef TIMER_IMPLEMENTATION_V3

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
