/*
 * gpio.h
 *
 *  Created on: 2018年11月26日
 *      Author: jeremy.hsiao
 */

#ifndef GPIO_H_
#define GPIO_H_

// PIO0_16 is SW2_WAKE
#define SWITCH_KEY_GPIO_PORT	(0)
#define SWITCH_KEY_GPIO_PIN		(16)
// PIO0_1 is SW1 ISP
#define ISP_KEY_GPIO_PORT		(0)
#define ISP_KEY_GPIO_PIN		(1)

/* These two inputs for GROUP GPIO Interrupt 0. */
#define GINT0_GPIO_PORT0    (SWITCH_KEY_GPIO_PORT)
#define GINT0_GPIO_BIT0     (SWITCH_KEY_GPIO_PIN)
// We are using one GPIO at the moment
#define GINT0_GPIO_PORT1    (SWITCH_KEY_GPIO_PORT)
#define GINT0_GPIO_BIT1     (SWITCH_KEY_GPIO_PIN)
// end
//#define GINT0_GPIO_PORT1    (ISP_KEY_GPIO_PORT)
//#define GINT0_GPIO_BIT1     (ISP_KEY_GPIO_PIN)

extern bool GPIOGoup0_Int;

void Init_GPIO(void);
void DeInit_GPIO(void);
bool Get_GPIO_Switch_Key(void);

#endif /* GPIO_H_ */
