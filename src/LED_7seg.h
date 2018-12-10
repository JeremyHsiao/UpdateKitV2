/*
 * LED_7seg.h
 *
 *  Created on: 2018年11月30日
 *      Author: jeremy.hsiao
 */

#ifndef LED_7SEG_H_
#define LED_7SEG_H_

	#define LED_7SEG_SEG1_PORT		(1)
	#define LED_7SEG_SEG1_PIN		(18) 		// P1_18
	#define	LED_7SEG_SEG1_IO_FUNC	(IOCON_FUNC0)

	#define LED_7SEG_SEG2_PORT		(1)
	#define LED_7SEG_SEG2_PIN		(25) 		// P1_25
	#define	LED_7SEG_SEG2_IO_FUNC	(IOCON_FUNC0)

	#define LED_7SEG_SEG3_PORT		(1)
	#define LED_7SEG_SEG3_PIN		(28) 		// P1_28
	#define	LED_7SEG_SEG3_IO_FUNC	(IOCON_FUNC0)

	#define LED_7SEG_SEG4_PORT		(2)
	#define LED_7SEG_SEG4_PIN		(12) 		// P2_12
	#define	LED_7SEG_SEG4_IO_FUNC	(IOCON_FUNC0)

	#define LED_7SEG_SEGa_PORT		(1)
	#define LED_7SEG_SEGa_PIN		(24) 		// P1_24
	#define	LED_7SEG_SEGa_IO_FUNC	(IOCON_FUNC0)

	#define LED_7SEG_SEGb_PORT		(2)
	#define LED_7SEG_SEGb_PIN		(3) 		// P2_3
	#define	LED_7SEG_SEGb_IO_FUNC	(IOCON_FUNC0)

	#define LED_7SEG_SEGc_PORT		(1)
	#define LED_7SEG_SEGc_PIN		(29) 		// P1_29
	#define	LED_7SEG_SEGc_IO_FUNC	(IOCON_FUNC0)

	#define LED_7SEG_SEGd_PORT		(0)
	#define LED_7SEG_SEGd_PIN		(9) 		// P0_9
	#define	LED_7SEG_SEGd_IO_FUNC	(IOCON_FUNC0)

	#define LED_7SEG_SEGdp_PORT		(0)
	#define LED_7SEG_SEGdp_PIN		(8) 		// P0_8
	#define	LED_7SEG_SEGdp_IO_FUNC	(IOCON_FUNC0)

	#define LED_7SEG_SEGe_PORT		(0)
	#define LED_7SEG_SEGe_PIN		(2) 		// P0_2
	#define	LED_7SEG_SEGe_IO_FUNC	(IOCON_FUNC0)

	#define LED_7SEG_SEGf_PORT		(1)
	#define LED_7SEG_SEGf_PIN		(19) 		// P1_19
	#define	LED_7SEG_SEGf_IO_FUNC	(IOCON_FUNC0)

	#define LED_7SEG_SEGg_PORT		(2)
	#define LED_7SEG_SEGg_PIN		(11) 		// P2_11
	#define	LED_7SEG_SEGg_IO_FUNC	(IOCON_FUNC0)

	extern void Init_LED_7seg_GPIO(void);
	extern void Update_LED_7SEG_Message_Buffer(uint8_t *msg, uint8_t new_dp_point);
	extern void refresh_LED_7SEG_periodic_task(void);
	extern void LED_Demo_Elapse_Timer(void);

#endif /* LED_7SEG_H_ */
