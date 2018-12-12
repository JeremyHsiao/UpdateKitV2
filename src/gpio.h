/*
 * gpio.h
 *
 *  Created on: 2018年11月26日
 *      Author: jeremy.hsiao
 */

#ifndef GPIO_H_
#define GPIO_H_

#ifdef _REAL_UPDATEKIT_V2_BOARD_

// PIO0_16 is SW2_WAKE
#define SWITCH_KEY_GPIO_PORT	(1)
#define SWITCH_KEY_GPIO_PIN		(24)
#define SWITCH_KEY_PIN_MUX		(IOCON_FUNC0 | (1L<<7))

// 5-8V enable
#define VOUT_ENABLE_GPIO_PORT	(2)
#define VOUT_ENABLE_GPIO_PIN	(7)
// LED-R
#define LED_R_GPIO_PORT	(0)
#define LED_R_GPIO_PIN	(11)
#define LED_R_GPIO_PIN_MUX		(IOCON_FUNC1 | IOCON_MODE_INACT | IOCON_DIGMODE_EN)
// LED-G
#define LED_G_GPIO_PORT	(0)
#define LED_G_GPIO_PIN	(13)
#define LED_G_GPIO_PIN_MUX		(IOCON_FUNC1 | IOCON_MODE_INACT | IOCON_DIGMODE_EN)
// LED-Y
#define LED_Y_GPIO_PORT	(0)
#define LED_Y_GPIO_PIN	(16)
#define LED_Y_GPIO_PIN_MUX		(IOCON_FUNC0 | IOCON_MODE_INACT | IOCON_DIGMODE_EN)

#else
// PIO0_16 is SW2_WAKE
#define SWITCH_KEY_GPIO_PORT	(0)
#define SWITCH_KEY_GPIO_PIN		(16)
#define SWITCH_KEY_PIN_MUX		(IOCON_FUNC0 | IOCON_MODE_INACT | IOCON_DIGMODE_EN)
// PIO0_1 is SW1 ISP
#define ISP_KEY_GPIO_PORT		(0)
#define ISP_KEY_GPIO_PIN		(1)

#define VOUT_ENABLE_GPIO_PORT	(2)
#define VOUT_ENABLE_GPIO_PIN	(7)
#endif // #ifdef _REAL_UPDATEKIT_V2_BOARD_

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

#define LED_R_LOW	Chip_GPIO_SetPinOutLow(LPC_GPIO, LED_R_GPIO_PORT, LED_R_GPIO_PIN)
#define LED_G_LOW	Chip_GPIO_SetPinOutLow(LPC_GPIO, LED_G_GPIO_PORT, LED_G_GPIO_PIN)
#define LED_Y_LOW	Chip_GPIO_SetPinOutLow(LPC_GPIO, LED_Y_GPIO_PORT, LED_Y_GPIO_PIN)
#define LED_R_HIGH	Chip_GPIO_SetPinOutHigh(LPC_GPIO, LED_R_GPIO_PORT, LED_R_GPIO_PIN)
#define LED_G_HIGH	Chip_GPIO_SetPinOutHigh(LPC_GPIO, LED_G_GPIO_PORT, LED_G_GPIO_PIN)
#define LED_Y_HIGH	Chip_GPIO_SetPinOutHigh(LPC_GPIO, LED_Y_GPIO_PORT, LED_Y_GPIO_PIN)

#endif /* GPIO_H_ */
