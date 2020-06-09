#ifndef _USER_IF_H_
#define _USER_IF_H_

//
// For build-date and build-time
//
// Example of __DATE__ string: "Jul 27 2012"
//                              01234567890
#define BUILD_YEAR_CH0 (__DATE__[ 7])
#define BUILD_YEAR_CH1 (__DATE__[ 8])
#define BUILD_YEAR_CH2 (__DATE__[ 9])
#define BUILD_YEAR_CH3 (__DATE__[10])


#define BUILD_MONTH_IS_JAN (__DATE__[0] == 'J' && __DATE__[1] == 'a' && __DATE__[2] == 'n')
#define BUILD_MONTH_IS_FEB (__DATE__[0] == 'F')
#define BUILD_MONTH_IS_MAR (__DATE__[0] == 'M' && __DATE__[1] == 'a' && __DATE__[2] == 'r')
#define BUILD_MONTH_IS_APR (__DATE__[0] == 'A' && __DATE__[1] == 'p')
#define BUILD_MONTH_IS_MAY (__DATE__[0] == 'M' && __DATE__[1] == 'a' && __DATE__[2] == 'y')
#define BUILD_MONTH_IS_JUN (__DATE__[0] == 'J' && __DATE__[1] == 'u' && __DATE__[2] == 'n')
#define BUILD_MONTH_IS_JUL (__DATE__[0] == 'J' && __DATE__[1] == 'u' && __DATE__[2] == 'l')
#define BUILD_MONTH_IS_AUG (__DATE__[0] == 'A' && __DATE__[1] == 'u')
#define BUILD_MONTH_IS_SEP (__DATE__[0] == 'S')
#define BUILD_MONTH_IS_OCT (__DATE__[0] == 'O')
#define BUILD_MONTH_IS_NOV (__DATE__[0] == 'N')
#define BUILD_MONTH_IS_DEC (__DATE__[0] == 'D')


#define BUILD_MONTH_CH0 \
    ((BUILD_MONTH_IS_OCT || BUILD_MONTH_IS_NOV || BUILD_MONTH_IS_DEC) ? '1' : '0')

#define BUILD_MONTH_CH1 \
    ( \
        (BUILD_MONTH_IS_JAN) ? '1' : \
        (BUILD_MONTH_IS_FEB) ? '2' : \
        (BUILD_MONTH_IS_MAR) ? '3' : \
        (BUILD_MONTH_IS_APR) ? '4' : \
        (BUILD_MONTH_IS_MAY) ? '5' : \
        (BUILD_MONTH_IS_JUN) ? '6' : \
        (BUILD_MONTH_IS_JUL) ? '7' : \
        (BUILD_MONTH_IS_AUG) ? '8' : \
        (BUILD_MONTH_IS_SEP) ? '9' : \
        (BUILD_MONTH_IS_OCT) ? '0' : \
        (BUILD_MONTH_IS_NOV) ? '1' : \
        (BUILD_MONTH_IS_DEC) ? '2' : \
        /* error default */    '?' \
    )

#define BUILD_DAY_CH0 ((__DATE__[4] >= '0') ? (__DATE__[4]) : '0')
#define BUILD_DAY_CH1 (__DATE__[ 5])



// Example of __TIME__ string: "21:06:19"
//                              01234567

#define BUILD_HOUR_CH0 (__TIME__[0])
#define BUILD_HOUR_CH1 (__TIME__[1])

#define BUILD_MIN_CH0 (__TIME__[3])
#define BUILD_MIN_CH1 (__TIME__[4])

#define BUILD_SEC_CH0 (__TIME__[6])
#define BUILD_SEC_CH1 (__TIME__[7])
//
// End
//

extern void Init_Value_From_EEPROM(void);
extern void Init_UpdateKitV2_variables(void);
extern void lcm_content_init(void);

#define	WELCOME_MESSAGE_DISPLAY_TIME_IN_S			(3)
#define OUTPUT_REMINDER_DISPLAY_TIME_IN_S			(6)
#define DEFAULT_MAX_FW_UPDATE_TIME_IN_S				(1800)
#define MINIMAL_TIMEOUT_VALUE						(2)
#define MAXIMAL_TIMEOUT_VALUE						(9999)
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

//#define POWERON_IS_DETECTING


extern UPDATE_STATE	current_system_proc_state;
extern uint8_t lcm_page_change_duration_in_sec;

// For branch -- No_Separate_Mode_Branch
extern uint8_t Get_Duty_from_Table(uint8_t current_step);


#define	WELCOME_MESSAGE_DISPLAY_TIME_IN_S			(3)
#define OUTPUT_REMINDER_DISPLAY_TIME_IN_S			(6)

#define DEFAULT_LCM_PAGE_CHANGE_S_WELCOME		(3)
#define DEFAULT_LCM_PAGE_CHANGE_S_OK			(5)
#define DEFAULT_OK_THRESHOLD					(5)			// 5 times ok

typedef enum
{
	LCM_WELCOME_PAGE = 0,
	LCM_PC_MODE,
	LCM_VR_MODE,
	LCM_MAX_PAGE_NO
} LCM_PAGE_ID;

//
// Button part of define
//

typedef struct {
	uint8_t 		port;
	uint8_t 		pin;
	TIMER_ID		state_change_timer;
	TIMER_UNIT_ID	state_change_timer_unit;
	TIMER_ID		repeat_timer;
	TIMER_UNIT_ID	repeat_timer_unit;
} Button_Data;

typedef enum
{
	BUTTON_SRC_ID = 0,
	BUTTON_DEC_ID,
	BUTTON_INC_ID,
	BUTTON_SEL_ID,
	BUTTON_TOTAL_ID
} ButtonID;

extern Button_Data const button_data[4];

extern void lcm_content_init(void);
extern bool lcm_text_buffer_cpy(LCM_PAGE_ID page_id, uint8_t row, uint8_t col, const void * restrict __s2, size_t len);

extern bool State_Proc_Button(ButtonID);
extern int Show_Resistor_Value(uint32_t value, char* result);
extern uint32_t Update_Resistor_Value_after_button(uint32_t previous_value, bool inc);

#endif // _USER_IF_H_
