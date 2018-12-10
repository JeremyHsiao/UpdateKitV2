/*
 * sw_timer.h
 *
 *  Created on: 2018年12月6日
 *      Author: jeremy.hsiao
 */

#ifndef SW_TIMER_H_
#define SW_TIMER_H_

#define 	SYSTICK_PER_SECOND			(40000)		// 40000 ticks per second == 25us each tick
#define     SYSTICK_COUNT_VALUE_MS(x)	((SYSTICK_PER_SECOND*x/1000)-1)
#define     SYSTICK_COUNT_VALUE_US(x)	((SYSTICK_PER_SECOND*x/1000000)-1)

extern bool 		SysTick_1s_timeout;
extern bool 		SysTick_100ms_timeout;
extern bool 		SysTick_led_7seg_refresh_timeout;
extern bool			SW_delay_timeout;
extern bool			lcd_module_auto_switch_timer_timeout;

extern uint32_t		time_elapse;
extern uint32_t		SW_delay_cnt;

#endif /* SW_TIMER_H_ */
