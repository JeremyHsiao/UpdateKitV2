/*
 * UpdateKitV2.h
 *
 *  Created on: 2018年12月14日
 *      Author: jeremy.hsiao
 */

#ifndef UPDATEKITV2_H_
#define UPDATEKITV2_H_

extern void SetDisplayVoltageCurrent(uint16_t voltage_new, uint16_t current_new);
extern void UpdateKitV2_LED_7_ToggleDisplayVoltageCurrent(void);
extern void UpdateKitV2_LED_7_UpdateDisplayValueAfterADC_Task(void);
extern void ButtonPressedTask(void);
extern void PowerOutputSetting(uint8_t current_step);

#define	DEFAULT_POWER_OUTPUT_STEP	(0)
#define DEFAULT_VOLTAGE_CURRENT_REFRESH_SEC	(2)

#endif /* UPDATEKITV2_H_ */
