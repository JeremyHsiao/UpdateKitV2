/*
 * sw_timer.h
 *
 *  Created on: 2018年12月6日
 *      Author: jeremy.hsiao
 */

#ifndef SW_TIMER_H_
#define SW_TIMER_H_

//#define 	SYSTICK_PER_SECOND			(10000)		// 10000 ticks per second == 100us each tick --> make sure deviation of delay is M 50us
#define 	SYSTICK_PER_SECOND			(2000)		// 4000 ticks per second == 250us each tick
//#define 	SYSTICK_PER_SECOND			(8000)		// 8000 ticks per second == 125us each tick
#define     SYSTICK_COUNT_VALUE_S(x)	((SYSTICK_PER_SECOND*x)-1)
#define     SYSTICK_COUNT_VALUE_MS(x)	((SYSTICK_PER_SECOND*x/1000)-1)
#define     SYSTICK_COUNT_VALUE_US(x)	((SYSTICK_PER_SECOND*x/1000000)-1)

typedef		uint16_t	TICK_UNIT;		// range is now between 8~8000 (minus 1)
typedef	struct {
	uint32_t	counts;
	uint32_t	reload_value;
//	TICK_UNIT	ticks;
	unsigned	unit:2;				// 0: 1ms, 1: 10ms, 2: 100ms, 3: 1000ms
	unsigned	count_up:1;
	unsigned	oneshot:1;
	unsigned	running:1;
	unsigned	timeup_flag:1;			// countdown to 0 or countup to reload_value
} SW_TIMER;

/*
typedef	struct {
	int32_t		counter;				// running(1) + timeout(1) + counter (30) so 1,342,177.275 if 8000 ticks each second.
	int32_t		reload_value;			// downcount(1) +  repeat(1) + reload_value
} SW_TIMER;
*/
typedef enum
{
	UPGRADE_ELAPSE_IN_S = 0,
	LED_G_TIMER_IN_100MS,
	LED_Y_TIMER_IN_100MS,
	LED_R_TIMER_IN_100MS,
	LCD_MODULE_PAGE_CHANGE_TIMER_IN_S,
	SYSTEM_STATE_PROC_TIMER,
	LCD_MODULE_INTERNAL_DELAY_IN_MS,
	SHIFT_REGISTER_NEXT_SHIFT_TIMER_IN_S,
	BUTTON_SRC_STATE_IN_10MS,
	BUTTON_DEC_STATE_IN_10MS,
	BUTTON_INC_STATE_IN_10MS,
	BUTTON_SEL_STATE_IN_10MS,
	BUTTON_ISP_STATE_IN_10MS,
	BUTTON_SRC_REPEAT_IN_10MS,
	BUTTON_DEC_REPEAT_IN_10MS,
	BUTTON_INC_REPEAT_IN_10MS,
	BUTTON_SEL_REPEAT_IN_10MS,
	BUTTON_ISP_REPEAT_IN_10MS,
	WELCOME_MESSAGE_IN_S,
	RELAY_SETUP_HYSTERSIS_IN_100MS,
	EEPROM_UPDATE_TIMER_IN_S,
	SW_TIMER_MAX_NO
} TIMER_ID;

typedef enum
{
	TIMER_MS = 0,
	TIMER_10MS,
	TIMER_100MS,
	TIMER_S,
	TIMER_UNIT_MAX_NO
} TIMER_UNIT_ID;

extern SW_TIMER	sw_timer[];
//extern bool			SysTick_flag;

//extern uint8_t time_elapse_str[];

extern void Update_Elapse_Timer(void);
extern bool Start_SW_Timer(TIMER_ID timer_no, uint32_t default_count, uint32_t upper_value, TIMER_UNIT_ID unit, bool upcount, bool oneshot);
//extern bool Init_SW_Timer(TIMER_ID timer_no, uint32_t default_count, uint32_t upper_value, TIMER_UNIT_ID unit, bool upcount, bool oneshot);
extern bool Pause_SW_Timer(TIMER_ID timer_no);
extern bool Play_SW_Timer(TIMER_ID timer_no);
extern uint32_t Read_SW_TIMER_Value(TIMER_ID timer_no);
extern bool Read_and_Clear_SW_TIMER_Reload_Flag(TIMER_ID timer_no);
extern void Clear_SW_TIMER_Reload_Flag(TIMER_ID timer_no);
extern void Raise_SW_TIMER_Reload_Flag(TIMER_ID timer_no);
extern bool Set_SW_Timer_Count(TIMER_ID timer_no, uint32_t new_count);
extern bool Set_SW_TIMER_Count_and_Play(TIMER_ID timer_no, uint32_t new_count);

#define Countdown_Once(id,duration,unit)			Start_SW_Timer(id,duration,0,unit, false, true)
#define Repeat_DownCounter(id,duration,unit)		Start_SW_Timer(id,duration,duration,unit, false, false)

#endif /* SW_TIMER_H_ */
