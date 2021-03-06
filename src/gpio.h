/*
 * gpio.h
 *
 *  Created on: 2018年11月26日
 *      Author: jeremy.hsiao
 */

#ifndef GPIO_H_
#define GPIO_H_

#ifdef _REAL_UPDATEKIT_V2_BOARD_

//// PIO1_24 Button
//#define SWITCH_KEY_GPIO_PORT	(1)
//#define SWITCH_KEY_GPIO_PIN		(24)
//#define SWITCH_KEY_PIN_MUX		(IOCON_FUNC0 | IOCON_MODE_PULLUP | IOCON_HYS_EN | (1L<<7))					// P1.0-2, 4-8. 10-21, 23-28, 30-31

// PIO0_1 Button
#define SWITCH_KEY_GPIO_PORT	(0)
#define SWITCH_KEY_GPIO_PIN		(1)
#define SWITCH_KEY_PIN_MUX		(IOCON_FUNC0 | IOCON_MODE_PULLUP | IOCON_DIGMODE_EN)					// P0.0-3; 6-10; 17-21);

// 5-8V enable P2_7
#define VOUT_ENABLE_GPIO_PORT	(2)
#define VOUT_ENABLE_GPIO_PIN	(7)
#define VOUT_ENABLE_PIN_MUX		(IOCON_FUNC0 | IOCON_MODE_INACT | IOCON_DIGMODE_EN )		//P2.2-23
// LED-R
#define LED_R_GPIO_PORT	(0)
#define LED_R_GPIO_PIN	(11)
#define LED_R_GPIO_PIN_MUX		(IOCON_FUNC1 | IOCON_MODE_INACT | IOCON_DIGMODE_EN)			// P0.11-16; 22-23
// LED-G
#define LED_G_GPIO_PORT	(0)
#define LED_G_GPIO_PIN	(13)
#define LED_G_GPIO_PIN_MUX		(IOCON_FUNC1 | IOCON_MODE_INACT | IOCON_DIGMODE_EN)			// P0.11-16; 22-23
// LED-Y
#define LED_Y_GPIO_PORT	(0)
#define LED_Y_GPIO_PIN	(16)
#define LED_Y_GPIO_PIN_MUX		(IOCON_FUNC0 | IOCON_MODE_INACT | IOCON_DIGMODE_EN)			// P0.11-16; 22-23

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

extern void Init_GPIO(void);
extern void DeInit_GPIO(void);
extern bool Get_GPIO_Switch_Key(void);
extern bool Debounce_Button(void);
#define DEBOUNCE_COUNT	SYSTICK_COUNT_VALUE_MS(20)		// It should be in fact less but not so much less

extern void LED_Status_Set_Value(uint32_t LED_status_value);
extern void LED_Status_Set_Auto_Toggle(uint32_t LED_status_pin, uint8_t flashing_100ms, uint32_t flashing_cnt);
extern void LED_Status_Clear_Auto_Toggle(uint32_t LED_status_pin);
extern void LED_Status_Update_Process(void);

#define	LED_STATUS_G		(1L<<LED_G_GPIO_PIN)
#define	LED_STATUS_Y		(1L<<LED_Y_GPIO_PIN)
#define	LED_STATUS_R		(1L<<LED_R_GPIO_PIN)
#define LED_STATUS_ALL		(LED_STATUS_G|LED_STATUS_Y|LED_STATUS_R)
#define LED_STATUS_MASK		(~(LED_STATUS_G|LED_STATUS_Y|LED_STATUS_R))

#define LED_R_LOW		Chip_GPIO_SetPinOutLow(LPC_GPIO, LED_R_GPIO_PORT, LED_R_GPIO_PIN)
#define LED_G_LOW		Chip_GPIO_SetPinOutLow(LPC_GPIO, LED_G_GPIO_PORT, LED_G_GPIO_PIN)
#define LED_Y_LOW		Chip_GPIO_SetPinOutLow(LPC_GPIO, LED_Y_GPIO_PORT, LED_Y_GPIO_PIN)
#define LED_R_HIGH		Chip_GPIO_SetPinOutHigh(LPC_GPIO, LED_R_GPIO_PORT, LED_R_GPIO_PIN)
#define LED_G_HIGH		Chip_GPIO_SetPinOutHigh(LPC_GPIO, LED_G_GPIO_PORT, LED_G_GPIO_PIN)
#define LED_Y_HIGH		Chip_GPIO_SetPinOutHigh(LPC_GPIO, LED_Y_GPIO_PORT, LED_Y_GPIO_PIN)
#define LED_R_TOGGLE	Chip_GPIO_SetPinToggle(LPC_GPIO, LED_R_GPIO_PORT, LED_R_GPIO_PIN)
#define LED_G_TOGGLE	Chip_GPIO_SetPinToggle(LPC_GPIO, LED_G_GPIO_PORT, LED_G_GPIO_PIN)
#define LED_Y_TOGGLE	Chip_GPIO_SetPinToggle(LPC_GPIO, LED_Y_GPIO_PORT, LED_Y_GPIO_PIN)

#endif /* GPIO_H_ */
