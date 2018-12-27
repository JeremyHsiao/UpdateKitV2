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
	US_SYSTEM_BOOTUP_STATE = 0,
	US_SYSTEM_WELCOME,
	US_CHECK_USER_SELECTION,
	US_COUNTDOWN_BEFORE_OUTPUT,
	US_PC_MODE_VOLTAGE_LOW,
	US_START_OUTPUT,
	US_WAIT_FOR_CURRENT_HIGH,
	US_WAIT_FW_UPGRADE_OK_STRING,
	US_FW_UPGRADE_DONE,
	US_UPGRADE_TOO_LONG,
	US_WAIT_FOR_NEXT_UPDATE,
	US_MAX_STATE_NO
} UPDATE_STATE;

extern void lcm_reset_FW_VER_Content(void);
extern void lcm_reset_Previous_FW_VER_Content(void);
extern void lcm_content_init(void);

extern void SetRawVoltage(uint16_t voltage_new);
extern void SetRawCurrent(uint16_t current_new);
extern uint16_t GetFilteredVoltage(void);
extern uint16_t GetFilteredCurrent(void);
extern void UpdateKitV2_LED_7_ToggleDisplayVoltageCurrent(void);
extern void UpdateKitV2_LED_7_StartDisplayVoltage(void);
extern void UpdateKitV2_UpdateDisplayValueForADC_Task(void);
extern void PowerOutputSetting(uint8_t current_step);
extern void init_filtered_input_current(void);
extern uint16_t Filtered_Input_current(uint16_t latest_current);
extern void init_filtered_input_voltage(void);
extern uint16_t Filtered_Input_voltage(uint16_t latest_voltage);
extern bool UART_input_processor(uint8_t key);
extern bool Event_Proc_State_Independent(void);
extern UPDATE_STATE Event_Proc_by_System_State(UPDATE_STATE current_state);
extern UPDATE_STATE System_State_Begin_Proc(UPDATE_STATE current_state);
extern UPDATE_STATE System_State_Running_Proc(UPDATE_STATE current_state);
extern UPDATE_STATE System_State_End_Proc(UPDATE_STATE current_state);

extern UPDATE_STATE	current_system_proc_state;

#define	WELCOME_MESSAGE_DISPLAY_TIME_IN_S		(3)
#define OUTPUT_REMINDER_DISPLAY_TIME_IN_S		(6)
#define DEFAULT_MAX_FW_UPDATE_TIME_IN_S			(150)

#define	DEFAULT_POWER_OUTPUT_STEP				(0)
#define DEFAULT_POWER_OUTPUT_DEBOUNCE_TIME_MS	(300)
#define DEFAULT_UPDATE_VOLTAGE_CURRENT_DATA_MS	(100)
#define DEFAULT_LED_DATA_CHANGE_SEC				(3)
#define DEFAULT_LED_REFRESH_EACH_DIGIT_MS		(1)
#define DEFAULT_LCM_PAGE_CHANGE_100MS			(3)
#define DEFAULT_OK_THRESHOLD					(5)			// 5 times ok
#define DEFAULT_INPUT_CURRENT_THRESHOLD			(10)		// 10ma

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
