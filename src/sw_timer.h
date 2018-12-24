/*
 * sw_timer.h
 *
 *  Created on: 2018年12月6日
 *      Author: jeremy.hsiao
 */

#ifndef SW_TIMER_H_
#define SW_TIMER_H_

//#define 	SYSTICK_PER_SECOND			(10000)		// 10000 ticks per second == 100us each tick --> make sure deviation of delay is M 50us
//#define 	SYSTICK_PER_SECOND			(4000)		// 4000 ticks per second == 250us each tick
#define 	SYSTICK_PER_SECOND			(12500)		// 12500 ticks per second == 80us each tick
#define     SYSTICK_COUNT_VALUE_MS(x)	((SYSTICK_PER_SECOND*x/1000)-1)
#define     SYSTICK_COUNT_VALUE_US(x)	((SYSTICK_PER_SECOND*x/1000000)-1)

extern bool 		SysTick_1s_timeout;
extern bool 		SysTick_100ms_timeout;
extern bool 		SysTick_led_7seg_refresh_timeout;
extern bool			SW_delay_timeout;
extern bool			lcd_module_auto_switch_timer_timeout;
extern bool			lcd_g_toggle_timeout;
extern bool			lcd_r_toggle_timeout;
extern bool			lcd_y_toggle_timeout;
extern bool			LED_Voltage_Current_Refresh_in_sec_timeout;
extern bool			lcd_module_wait_finish_timeout;
extern bool			System_State_Proc_timer_timeout;

typedef		uint16_t	TICK_UNIT;
typedef	struct {
	uint32_t	counts;
	uint32_t	reload_value;
	TICK_UNIT	ticks;
	unsigned	unit:2;				// 0: 1ms, 1: 10ms, 2: 100ms, 3: 1000ms
	unsigned	count_up:1;
	unsigned	oneshot:1;
	unsigned	running:1;
	unsigned	timeup_flag:1;			// countdown to 0 or countup to reload_value
} SW_TIMER;

enum
{
	UPGRADE_ELAPSE_IN_100MS = 0,
	LED_G_TIMER_IN_100MS,
	LED_Y_TIMER_IN_100MS,
	LED_R_TIMER_IN_100MS,
	SYSTEM_TIME_ELAPSE_IN_SEC,
	SW_TIMER_MAX_NO
};

enum
{
	TIMER_1MS = 0,
	TIMER_10MS,
	TIMER_100MS,
	TIMER_1000MS,
	TIMER_UNIT_MAX_NO
};

extern SW_TIMER	sw_timer[];

//extern uint32_t		time_elapse_in_sec;
//#define				time_elapse_in_sec	(sw_timer[SYSTEM_TIME_ELAPSE_IN_SEC].counts)
//#define				Upgrade_elapse_in_100ms	(sw_timer[UPGRADE_ELAPSE_IN_100MS].counts)

//extern uint32_t		Upgrade_elapse_in_100ms;
extern uint32_t		SW_delay_sys_tick_cnt;
extern uint16_t		lcd_module_auto_switch_in_ms;
//extern uint32_t		led_g_toggle_timer_in_100ms;
extern uint32_t		led_r_toggle_timer_in_100ms;
//extern uint32_t		led_y_toggle_timer_in_100ms;
//extern uint32_t		led_g_toggle_timer_reload;
extern uint32_t		led_r_toggle_timer_reload;
//extern uint32_t		led_y_toggle_timer_reload;

extern uint8_t		LED_Voltage_Current_Refresh_in_sec;
extern uint8_t		LED_Voltage_Current_Refresh_reload;
extern uint8_t		lcd_module_wait_finish_in_tick;

extern uint32_t		System_State_Proc_timer_in_ms;

extern uint8_t time_elapse_str[];
extern void Update_Elapse_Timer(void);

extern bool Start_SW_Timer(uint8_t timer_no, uint32_t default_count, uint32_t upper_value, uint8_t unit, bool upcount, bool oneshot);
extern bool Reset_SW_Timer(uint8_t timer_no, uint32_t default_count, uint32_t upper_value, uint8_t unit, bool upcount, bool oneshot);
extern bool Pause_SW_Timer(uint8_t timer_no);
extern bool Play_SW_Timer(uint8_t timer_no);
extern uint32_t Read_SW_TIMER_Value(uint8_t timer_no);
extern bool Read_and_Clear_SW_TIMER_Reload_Flag(uint8_t timer_no);
extern void Clear_SW_TIMER_Reload_Flag(uint8_t timer_no);

#endif /* SW_TIMER_H_ */
