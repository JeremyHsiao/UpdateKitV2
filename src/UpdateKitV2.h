/*
 * UpdateKitV2.h
 *
 *  Created on: 2018年12月14日
 *      Author: jeremy.hsiao
 */

#ifndef UPDATEKITV2_H_
#define UPDATEKITV2_H_

typedef enum
{
	US_SYSTEM_STARTUP = 0,
	US_WELCOME_MESSAGE,
	US_PC_MODE_NO_VOLTAGE_OUTPUT,
	US_REMINDER_BEFORE_VOLTAGE_OUTPUT,
	US_WAITING_FW_UPGRADE,
	US_FW_UPGRADE_DONE,
	US_TV_IN_STANDBY,
	US_MAX_STATE_NO
} UPDATE_STATE;



extern void lcm_content_init(void);

extern void SetDisplayVoltage(uint16_t voltage_new);
extern void SetDisplayCurrent(uint16_t current_new);
extern uint16_t GetDisplayVoltage(void);
extern uint16_t GetDisplayCurrent(void);
extern void UpdateKitV2_LED_7_ToggleDisplayVoltageCurrent(void);
extern void UpdateKitV2_UpdateDisplayValueForADC_Task(void);
extern void ButtonPressedTask(void);
extern void PowerOutputSetting(uint8_t current_step);
extern void init_filtered_input_current(void);
extern uint16_t Filtered_Input_current(uint16_t latest_current);
extern void init_filtered_input_voltage(void);
extern uint16_t Filtered_Input_voltage(uint16_t latest_voltage);
extern UPDATE_STATE System_State_Proc(UPDATE_STATE current_state);

#define	DEFAULT_POWER_OUTPUT_STEP	(0)
#define DEFAULT_VOLTAGE_CURRENT_REFRESH_SEC	(3-1)

enum
{
	LCM_WELCOME_PAGE = 0,
	LCM_PC_MODE,
	LCM_REMINDER_BEFORE_OUTPUT,
	LCM_INPUT_MEASURE_PAGE,
	LCM_VERSION_PAGE,
	LCM_TV_IN_STANDBY_PAGE,
	LCM_ENTER_ISP_PAGE,
	LCM_MAX_PAGE_NO
};


#endif /* UPDATEKITV2_H_ */
