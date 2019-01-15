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
	US_WAIT_FW_UPGRADE_OK_STRING,
	US_FW_UPGRADE_DONE,
	US_UPGRADE_TOO_LONG,
	US_READY_FOR_NEXT_UPDATE,
	US_TV_IN_STANDBY,
	US_MAX_STATE_NO						// 11
} UPDATE_STATE;

extern void Init_User_Selection_From_EEPROM(void);
extern void Init_UpdateKitV2_variables(void);
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
extern void SimpleOutputVoltageCalibration(void);
extern void init_filtered_input_current(void);
extern uint16_t Filtered_Input_current(uint16_t latest_current);
extern void init_filtered_input_voltage(void);
extern uint16_t Filtered_Input_voltage(uint16_t latest_voltage);
extern void ResetAllCurrentDebounceTimerEvent(void);
extern bool UART_input_processor(uint8_t key);
extern bool Event_Proc_State_Independent(void);
extern UPDATE_STATE Event_Proc_by_System_State(UPDATE_STATE current_state);
extern UPDATE_STATE System_State_Begin_Proc(UPDATE_STATE current_state);
extern UPDATE_STATE System_State_Running_Proc(UPDATE_STATE current_state);
extern UPDATE_STATE System_State_End_Proc(UPDATE_STATE current_state);

#define	WELCOME_MESSAGE_DISPLAY_TIME_IN_S			(3)
#define OUTPUT_REMINDER_DISPLAY_TIME_IN_S			(6)
#define DEFAULT_MAX_FW_UPDATE_TIME_IN_S				(1800)
#define LED_STATUS_TOGGLE_DURATION_IN_100MS_FAST	(1)			// 500ms
#define LED_STATUS_TOGGLE_DURATION_IN_100MS			(5)			// 500ms

#define	DEFAULT_POWER_OUTPUT_STEP				(0)
#define POWER_OUTPUT_STEP_TOTAL_NO				(10)
//#define DEFAULT_POWER_OUTPUT_DEBOUNCE_TIME_MS	(800)
//#define DEFAULT_LEAVE_STANDBY_DEBOUNCE_TIME_MS	(800)
//#define DEFAULT_HIGH_TO_LOW_DEBOUNCE_TIME_MS	(800)
#define DEFAULT_UPDATE_VOLTAGE_CURRENT_DATA_100MS	(4)		// 400 ms
#define DEFAULT_LED_DATA_CHANGE_SEC				(3)
#define DEFAULT_LED_REFRESH_EACH_DIGIT_MS		(2)
#define DEFAULT_LCM_PAGE_CHANGE_S_WELCOME		(3)
#define DEFAULT_LCM_PAGE_CHANGE_S_OK			(5)
#define DEFAULT_OK_THRESHOLD					(5)			// 5 times ok
//#define DEFAULT_STANDBY_CURRENT_THRESHOLD		(100)		// 80ma
#define DEFAULT_STANDBY_POWER_THRESHOLD			(500)		// V*A < 0.5W
//#define DEFAULT_FW_UPGRADE_CURRENT_THRESHOLD	(120)
//#define DEFAULT_NO_CURRENT_THRESHOLD			(9)			// 9ma
#define DEFAULT_TV_STANDBY_DEBOUNCE_IN_100MS	(50)		// 5S
#define DEFAULT_NO_OUTPUT_DEBOUNCE_IN_100MS		(20)		// 2S
#define DEFAULT_OUTPUT_NORMAL_DEBOUNCE_IN_100MS	(20)		// 2S

#define CHANGE_FW_MAX_UPDATE_TIME_AFTER_OK(x)		(((x*3)+1)/2)
#define CHANGE_FW_MAX_UPDATE_TIME_AFTER_TOO_LONG(x)	(((x*3)+1)/2)

// Slower ADC sample clock
#define ADC_SAMPLE_CLK_DIVIDER	(10)

//#define POWERON_IS_DETECTING

typedef enum
{
	LCM_WELCOME_PAGE = 0,
	LCM_PC_MODE,
	LCM_REMINDER_BEFORE_OUTPUT,
	LCM_FW_UPGRADING_PAGE,
	LCM_FW_OK_VER_PAGE,
	LCM_FW_OK_VER_PAGE_PREVIOUS_UPDATE_INFO,
	LCM_FW_UPGRADE_TOO_LONG_PAGE,
	LCM_TV_IN_STANDBY_PAGE,
	LCM_ENTER_ISP_PAGE,
//	LCM_DEV_TITLE_PAGE,
//	LCM_DEV_MEASURE_PAGE,
//	LCM_DEV_UPGRADE_VER_PAGE,
//	LCM_DEV_OK_DETECT_PAGE,
	LCM_MAX_PAGE_NO
} LCM_PAGE_ID;


extern UPDATE_STATE	current_system_proc_state;
extern uint8_t lcm_page_change_duration_in_sec;

#endif /* UPDATEKITV2_H_ */
