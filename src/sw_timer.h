/*
 * sw_timer.h
 *
 *  Created on: 2018年12月6日
 *      Author: jeremy.hsiao
 */

#ifndef SW_TIMER_H_
#define SW_TIMER_H_

//#define 	SYSTICK_PER_SECOND			(10000)		// 10000 ticks per second == 100us each tick --> make sure deviation of delay is M 50us
#define 	SYSTICK_PER_SECOND			(8000)		// 8000 ticks per second == 125us each tick
#define     SYSTICK_COUNT_VALUE_MS(x)	((SYSTICK_PER_SECOND*x/1000)-1)
#define     SYSTICK_COUNT_VALUE_US(x)	((SYSTICK_PER_SECOND*x/1000000)-1)
#define		SW_TIMER_NO_IN_USE	6

extern bool 		SysTick_1s_timeout;
extern bool 		SysTick_100ms_timeout;
extern bool 		SysTick_led_7seg_refresh_timeout;
extern bool			SW_delay_timeout;
extern bool			lcd_module_auto_switch_timer_timeout;

extern uint32_t		time_elapse;
extern uint32_t		SW_delay_cnt;

extern uint8_t time_elapse_str[];
extern void Update_Elapse_Timer(void);

#endif /* SW_TIMER_H_ */
