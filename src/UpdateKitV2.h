/*
 * UpdateKitV2.h
 *
 *  Created on: 2018年12月14日
 *      Author: jeremy.hsiao
 */

#ifndef UPDATEKITV2_H_
#define UPDATEKITV2_H_

extern void SetDisplayVoltage(uint16_t voltage_new);
extern void SetDisplayCurrent(uint16_t current_new);
extern void UpdateKitV2_LED_7_ToggleDisplayVoltageCurrent(void);
extern void UpdateKitV2_LED_7_UpdateDisplayValueAfterADC_Task(void);
extern void ButtonPressedTask(void);
extern void PowerOutputSetting(uint8_t current_step);
extern void init_filtered_input_current(void);
extern uint16_t Filtered_Input_current(uint16_t latest_current);

#define	DEFAULT_POWER_OUTPUT_STEP	(0)
#define DEFAULT_VOLTAGE_CURRENT_REFRESH_SEC	(2)

#endif /* UPDATEKITV2_H_ */
