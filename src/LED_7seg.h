/*
 * LED_7seg.h
 *
 *  Created on: 2018年11月30日
 *      Author: jeremy.hsiao
 */

#ifndef LED_7SEG_H_
#define LED_7SEG_H_

#ifdef _REAL_UPDATEKIT_V2_BOARD_

	#define LED_7SEG_SEGa_PORT		(0)
	#define LED_7SEG_SEGa_PIN		(17)	//a
	#define	LED_7SEG_SEGa_IO_FUNC	(IOCON_FUNC0 | IOCON_MODE_INACT  | (1L<<7))				// P0-3; 6-10; 17-21

	#define LED_7SEG_SEGb_PORT		(0)
	#define LED_7SEG_SEGb_PIN		(21)	// b
	#define	LED_7SEG_SEGb_IO_FUNC	(IOCON_FUNC0 | IOCON_MODE_INACT | (1L<<7))				// P0-3; 6-10; 17-21

	#define LED_7SEG_SEGc_PORT		(2)		// c
	#define LED_7SEG_SEGc_PIN		(1)
	#define	LED_7SEG_SEGc_IO_FUNC	(IOCON_FUNC0 | IOCON_MODE_INACT | IOCON_DIGMODE_EN)		//P2.0-1

	#define LED_7SEG_SEGd_PORT		(0)		// d
	#define LED_7SEG_SEGd_PIN		(20)
	#define	LED_7SEG_SEGd_IO_FUNC	(IOCON_FUNC0 | IOCON_MODE_INACT | (1L<<7))				// P0.0-3; 6-10; 17-21

	#define LED_7SEG_SEGe_PORT		(2)		//e
	#define LED_7SEG_SEGe_PIN		(0)
	#define	LED_7SEG_SEGe_IO_FUNC	(IOCON_FUNC0 | IOCON_MODE_INACT  | IOCON_DIGMODE_EN)	//P2.0-1

	#define LED_7SEG_SEGf_PORT		(0)
	#define LED_7SEG_SEGf_PIN		(4)
	#define	LED_7SEG_SEGf_IO_FUNC	(IOCON_FUNC0 | (1UL<<8))								// P0.4-5 GPIO

	#define LED_7SEG_SEGg_PORT		(2)		// g
	#define LED_7SEG_SEGg_PIN		(5)
	#define	LED_7SEG_SEGg_IO_FUNC	(IOCON_FUNC0 | IOCON_MODE_INACT | (1L<<7))				//P2.2-23

	#define LED_7SEG_SEGdp_PORT		(0)		// dp
	#define LED_7SEG_SEGdp_PIN		(22)
	#define	LED_7SEG_SEGdp_IO_FUNC	(IOCON_FUNC0 | IOCON_MODE_INACT | IOCON_DIGMODE_EN)		// P0.11-16; 22-23

	#define LED_7SEG_SEG1_PORT		(2)		// dig 1
	#define LED_7SEG_SEG1_PIN		(2)
	#define	LED_7SEG_SEG1_IO_FUNC	(IOCON_FUNC0 | IOCON_MODE_INACT  | (1L<<7))				//P2.2-23

	#define LED_7SEG_SEG2_PORT		(1)		// dig 2
	#define LED_7SEG_SEG2_PIN		(23)
	#define	LED_7SEG_SEG2_IO_FUNC	(IOCON_FUNC0 | IOCON_MODE_INACT | (1L<<7))			// P1.0-2, 4-8. 10-21, 23-28, 30-31

	#define LED_7SEG_SEG3_PORT		(0)
	#define LED_7SEG_SEG3_PIN		(5)
	#define	LED_7SEG_SEG3_IO_FUNC	(IOCON_FUNC0 | (1UL<<8))								// P0.4-5 GPIO

	#define LED_7SEG_SEG4_PORT		(0)
	#define LED_7SEG_SEG4_PIN		(2)		// 4
	#define	LED_7SEG_SEG4_IO_FUNC	(IOCON_FUNC0 | IOCON_MODE_INACT | (1L<<7))			// P0-3; 6-10; 17-21

#else

	#define LED_7SEG_SEG1_PORT		(1)
	#define LED_7SEG_SEG1_PIN		(18) 		// P1_18
	#define	LED_7SEG_SEG1_IO_FUNC	(IOCON_FUNC0 | IOCON_MODE_INACT)

	#define LED_7SEG_SEG2_PORT		(1)
	#define LED_7SEG_SEG2_PIN		(25) 		// P1_25
	#define	LED_7SEG_SEG2_IO_FUNC	(IOCON_FUNC0 | IOCON_MODE_INACT)

	#define LED_7SEG_SEG3_PORT		(1)
	#define LED_7SEG_SEG3_PIN		(28) 		// P1_28
	#define	LED_7SEG_SEG3_IO_FUNC	(IOCON_FUNC0 | IOCON_MODE_INACT)

	#define LED_7SEG_SEG4_PORT		(2)
	#define LED_7SEG_SEG4_PIN		(12) 		// P2_12
	#define	LED_7SEG_SEG4_IO_FUNC	(IOCON_FUNC0 | IOCON_MODE_INACT)

	#define LED_7SEG_SEGa_PORT		(1)
	#define LED_7SEG_SEGa_PIN		(24) 		// P1_24
	#define	LED_7SEG_SEGa_IO_FUNC	(IOCON_FUNC0 | IOCON_MODE_INACT)

	#define LED_7SEG_SEGb_PORT		(2)
	#define LED_7SEG_SEGb_PIN		(3) 		// P2_3
	#define	LED_7SEG_SEGb_IO_FUNC	(IOCON_FUNC0 | IOCON_MODE_INACT)

	#define LED_7SEG_SEGc_PORT		(1)
	#define LED_7SEG_SEGc_PIN		(29) 		// P1_29
	#define	LED_7SEG_SEGc_IO_FUNC	(IOCON_FUNC0 | IOCON_MODE_INACT)

	#define LED_7SEG_SEGd_PORT		(0)
	#define LED_7SEG_SEGd_PIN		(9) 		// P0_9
	#define	LED_7SEG_SEGd_IO_FUNC	(IOCON_FUNC0 | IOCON_MODE_INACT)

	#define LED_7SEG_SEGdp_PORT		(0)
	#define LED_7SEG_SEGdp_PIN		(8) 		// P0_8
	#define	LED_7SEG_SEGdp_IO_FUNC	(IOCON_FUNC0 | IOCON_MODE_INACT)

	#define LED_7SEG_SEGe_PORT		(0)
	#define LED_7SEG_SEGe_PIN		(2) 		// P0_2
	#define	LED_7SEG_SEGe_IO_FUNC	(IOCON_FUNC0 | IOCON_MODE_INACT)

	#define LED_7SEG_SEGf_PORT		(1)
	#define LED_7SEG_SEGf_PIN		(19) 		// P1_19
	#define	LED_7SEG_SEGf_IO_FUNC	(IOCON_FUNC0 | IOCON_MODE_INACT)

	#define LED_7SEG_SEGg_PORT		(2)
	#define LED_7SEG_SEGg_PIN		(11) 		// P2_11
	#define	LED_7SEG_SEGg_IO_FUNC	(IOCON_FUNC0 | IOCON_MODE_INACT)

#endif // #ifdef _REAL_UPDATEKIT_V2_BOARD_

	extern void Init_LED_7seg_GPIO(void);
	extern void Update_LED_7SEG_Message_Buffer(uint8_t page, uint8_t *msg, uint8_t new_dp_point);
	extern void refresh_LED_7SEG_periodic_task(void);
	extern void LED_Demo_Elapse_Timer(void);
	extern void LED_7seg_self_test(void);
	extern bool LED_7SEG_GoToNextVisiblePage(void);
	extern bool LED_7SEG_ForceToSpecificPage(uint8_t page);

#define LED_DISPLAY_PAGE		(2)
#define LED_DISPLAY_COL			(4)
#define	LED_GPIO_NO				(3)
extern uint8_t led_7SEG_display_content[LED_DISPLAY_PAGE][LED_DISPLAY_COL];
extern uint8_t led_7SEG_display_enable[LED_DISPLAY_PAGE];
extern uint8_t led_7SEG_display_dp[LED_DISPLAY_PAGE];			// 1-4, 0 means no dp

enum {
	LED_VOLTAGE_PAGE = 0,
	LED_CURRENT_PAGE
};

#endif /* LED_7SEG_H_ */
