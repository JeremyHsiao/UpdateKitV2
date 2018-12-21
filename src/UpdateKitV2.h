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
	US_SYSTEM_STARTUP_WELCOME_MESSAGE = 0,
	US_DETERMINE_PCMODE_OR_COUNTDOWN_FOR_VOUT,
	US_OUTPUT_REMINDER_COUNTDOWN_NOW,
	US_PC_MODE_NO_VOLTAGE_OUTPUT,
	US_PC_MODE_NO_VOLTAGE_OUTPUT_PAGE2,
	US_OUTPUT_ENABLE,
	US_WAIT_FW_UPGRADE_OK_VER_STRING,
	US_FW_UPGRADE_DONE,
	US_FW_UPGRADE_DONE_PAGE2,
	US_TV_IN_STANDBY,
	US_MAX_STATE_NO
} UPDATE_STATE;

extern void lcm_content_init(void);

extern void SetRawVoltage(uint16_t voltage_new);
extern void SetRawCurrent(uint16_t current_new);
extern uint16_t GetFilteredVoltage(void);
extern uint16_t GetFilteredCurrent(void);
extern void UpdateKitV2_LED_7_ToggleDisplayVoltageCurrent(void);
extern void UpdateKitV2_LED_7_StartDisplayVoltage(void);
extern void UpdateKitV2_UpdateDisplayValueForADC_Task(void);
extern void ButtonPressedTask(void);
extern void PowerOutputSetting(uint8_t current_step);
extern void init_filtered_input_current(void);
extern uint16_t Filtered_Input_current(uint16_t latest_current);
extern void init_filtered_input_voltage(void);
extern uint16_t Filtered_Input_voltage(uint16_t latest_voltage);
extern UPDATE_STATE System_State_Proc(UPDATE_STATE current_state);
extern bool UART_input_processor(uint8_t key);

extern bool		EVENT_raw_current_goes_above_threshold;
extern bool		EVENT_raw_current_goes_below_threshold;
extern bool		EVENT_filtered_current_goes_above_threshold;
extern bool		EVENT_filtered_current_goes_below_threshold;
extern bool		EVENT_OK_string_confirmed;
extern bool		EVENT_Version_string_confirmed;
extern bool		EVENT_POWERON_string_confirmed;
extern bool		EVENT_Button_pressed_debounced;
extern UPDATE_STATE	upcoming_system_state;

#define	DEFAULT_POWER_OUTPUT_STEP			(0)
#define DEFAULT_VOLTAGE_CURRENT_REFRESH_SEC	(3-1)
#define DEFAULT_OK_THRESHOLD				(5)
#define DEFAULT_INPUT_CURRENT_THRESHOLD		(10)		// 10ma
enum
{
	LCM_WELCOME_PAGE = 0,
	LCM_PC_MODE,
	LCM_REMINDER_BEFORE_OUTPUT,
	LCM_FW_UPGRADING_PAGE,
	LCM_FW_OK_VER_PAGE,
	LCM_FW_OK_VER_PAGE_PREVIOUS_UPDATE_INFO,
	LCM_TV_IN_STANDBY_PAGE,
	LCM_ENTER_ISP_PAGE,
	LCM_DEV_TITLE_PAGE,
	LCM_DEV_MEASURE_PAGE,
	LCM_DEV_UPGRADE_VER_PAGE,
	LCM_DEV_OK_DETECT_PAGE,
	LCM_MAX_PAGE_NO
};


#endif /* UPDATEKITV2_H_ */
