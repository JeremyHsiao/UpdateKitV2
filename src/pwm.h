/*
 * pwm.h
 *
 *  Created on: 2018年11月26日
 *      Author: jeremy.hsiao
 */

#ifndef PWM_H_
#define PWM_H_

///* Systick is used for general timing */
//#define TICKRATE_HZ (33)	/* 33% rate of change per second */

/* PWM cycle time - time of a single PWM sweep */
#define DEFUALT_PWMCYCLERATE (30000UL)
#define default_duty_cycle	(100)

// PWM PIO
#define PWM0_GPIO_PORT		(1)
#define PWM0_GPIO_PIN		(13)
#define PWM0_PIN_MUX		(IOCON_FUNC2 | IOCON_MODE_INACT | (1L<<7))					// P1.0-2, 4-8. 10-21, 23-28, 30-31

extern void Init_PWM(void);
extern void DeInit_PWM(void);
extern void setPWMRate(int pwnNum, uint8_t percentage);
extern void MySetupPWMFrequency(uint32_t freq, uint8_t duty);

extern uint32_t pwm_freq;
extern uint8_t	pwm_duty;

#endif /* PWM_H_ */
