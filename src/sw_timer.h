/*
 * sw_timer.h
 *
 *  Created on: 2018年12月6日
 *      Author: jeremy.hsiao
 */

#ifndef SW_TIMER_H_
#define SW_TIMER_H_

//#define 	SYSTICK_PER_SECOND			(10000)		// 10000 ticks per second == 100us each tick --> make sure deviation of delay is M 50us
#define 	SYSTICK_PER_SECOND			(4000)		// 4000 ticks per second == 250us each tick
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

extern uint32_t		time_elapse_in_sec;
extern uint32_t		SW_delay_sys_tick_cnt;
extern uint32_t		lcd_module_auto_switch_in_ms;
extern uint32_t		led_g_toggle_timer_in_100ms;
extern uint32_t		led_r_toggle_timer_in_100ms;
extern uint32_t		led_y_toggle_timer_in_100ms;
extern uint32_t		led_g_toggle_timer_reload;
extern uint32_t		led_r_toggle_timer_reload;
extern uint32_t		led_y_toggle_timer_reload;

extern uint8_t time_elapse_str[];
extern void Update_Elapse_Timer(void);

#endif /* SW_TIMER_H_ */
