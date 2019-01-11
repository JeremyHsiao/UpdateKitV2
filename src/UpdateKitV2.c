/*

	UpdateKitV2 main code

 */

#include "chip.h"
#include "board.h"
#include "pwm.h"
#include "uart_0_rb.h"
#include "gpio.h"
#include "LED_7seg.h"
#include "adc.h"
#include "string_detector.h"
#include "lcd_module.h"
#include "sw_timer.h"
#include "string.h"
#include "UpdateKitV2.h"
#include "build_defs.h"
#include "event.h"
#include "user_opt.h"

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/

uint8_t		current_output_stage;
const uint8_t		pwm_table[POWER_OUTPUT_STEP_TOTAL_NO] = { 100, 59,  51, 43, 35, 27, 20, 12, 4, 0};
const char *pwm_voltage_table [POWER_OUTPUT_STEP_TOTAL_NO] = { "0.0", "6.0", "6.5", "7.0", "7.5", "8.0", "8.5", "9.0", "9.5", "9.7" };
const uint8_t		default_no_current_threshold_lut[POWER_OUTPUT_STEP_TOTAL_NO] = { 9, 9, 9, 9, 9, 9, 9, 9, 9, 9 };
const uint16_t		target_voltage_table[POWER_OUTPUT_STEP_TOTAL_NO] = { 0, 6000, 6500, 7000, 7500, 8000, 8500, 9000, 9500, 9800 };

#define	CURRENT_HISTORY_DATA_SIZE	64
RINGBUFF_T 	current_history;
uint16_t 	current_history_data[CURRENT_HISTORY_DATA_SIZE];
uint32_t	total_current_value = 0;

#define	VOLTAGE_HISTORY_DATA_SIZE	64
RINGBUFF_T 	voltage_history;
uint16_t 	voltage_history_data[VOLTAGE_HISTORY_DATA_SIZE];
uint32_t	total_voltage_value;

uint16_t			raw_voltage;			//  0.00v ~ 9.99v --> 0-999
uint16_t			raw_current;			// .000A ~ .999A --> 0-999
uint16_t			filtered_voltage;		//  0.00v ~ 9.99v --> 0-999
uint16_t			filtered_current;		// .000A ~ .999A --> 0-999

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/

UPDATE_STATE	current_system_proc_state;
uint16_t		max_upgrade_time_in_S;
uint8_t			lcm_page_change_duration_in_sec;

/*****************************************************************************
 * Private functions
 ****************************************************************************/

/*****************************************************************************
 * Public functions
 ****************************************************************************/
void init_filtered_input_current(void)
{
	total_current_value = 0;
	filtered_current = 0;

	RingBuffer_Init(&current_history, current_history_data, sizeof(uint16_t), CURRENT_HISTORY_DATA_SIZE);
	RingBuffer_Flush(&current_history);
}

void init_filtered_input_voltage(void)
{
	total_voltage_value = 0;
	filtered_voltage = 0;

	RingBuffer_Init(&voltage_history, voltage_history_data, sizeof(uint16_t), VOLTAGE_HISTORY_DATA_SIZE);
	RingBuffer_Flush(&voltage_history);
}

void Init_UpdateKitV2_variables(void)
{
	current_system_proc_state = US_SYSTEM_BOOTUP_STATE;
	max_upgrade_time_in_S = DEFAULT_MAX_FW_UPDATE_TIME_IN_S;
	lcm_page_change_duration_in_sec = DEFAULT_LCM_PAGE_CHANGE_S_WELCOME;
	raw_voltage = 0;			//  0.00v ~ 9.99v --> 0-999
	raw_current = 0;			// .000A ~ .999A --> 0-999
	filtered_voltage = 0;		//  0.00v ~ 9.99v --> 0-999
	filtered_current = 0;		// .000A ~ .999A --> 0-999
	total_current_value = 0;
	init_filtered_input_current();
	init_filtered_input_voltage();
	current_output_stage = DEFAULT_POWER_OUTPUT_STEP;
}

void lcm_content_init_old(void)
{
//    strcpy((void *)&lcd_module_display_content[0][0][0], "TPV UpdateKit V2" __TIME__);
    //lcd_module_display_enable[3] = 0x00;
     //memset((void *)lcd_module_display_enable, 0x00, 4);
     //lcd_module_display_enable[1]=1;
}

// Across several pages
#define ELAPSE_TIME_LEN 	(4)
#define MAX_FW_VER_LEN		(LCM_DISPLAY_COL-3)

// LCM_FW_UPGRADING_PAGE
#define ELAPSE_TIME_POS		(8)
#define ELAPSE_TIME_ROW 	(0)

// LCM_FW_OK_VER_PAGE
#define OK_TIME_POS			(8)
#define OK_TIME_ROW 		(0)
#define CURRENT_FW_POS 		(3)
#define CURRENT_FW_ROW 		(1)

// LCM_FW_OK_VER_PAGE_PREVIOUS_UPDATE_INFO
#define PREVIOUS_TIME_POS	(8)
#define PREVIOUS_TIME_ROW 	(0)
#define PREVIOUS_FW_POS 	(3)
#define PREVIOUS_FW_ROW 	(1)

// LCM_FW_UPGRADE_TOO_LONG_PAGE
#define TIMEOUT_TIME_POS	(8)
#define TIMEOUT_TIME_ROW 	(0)

// LCM_REMINDER_BEFORE_OUTPUT
#define OUTPUT_SELECT_LEN 	(3)
#define OUTPUT_SELECT_POS	(12)
#define OUTPUT_SELECT_ROW	(0)
#define COUNTDOWN_TIME_LEN 	(1)
#define COUNTDOWN_TIME_POS	(10)
#define COUNTDOWN_TIME_ROW	(1)

void lcm_reset_FW_VER_Content(void)
{
	// FW upgrade is done and show software version page				   						 0123456789012345
	memcpy((void *)&lcd_module_display_content[LCM_FW_OK_VER_PAGE][ELAPSE_TIME_ROW][0], 		"Upgrade:   0 Sec", LCM_DISPLAY_COL);
	memcpy((void *)&lcd_module_display_content[LCM_FW_OK_VER_PAGE][CURRENT_FW_ROW][0], 			"FW:             ", LCM_DISPLAY_COL);
}

void lcm_reset_Previous_FW_VER_Content(void)
{
	// FW upgrade info of previous update                				                         				 0123456789012345
	memcpy((void *)&lcd_module_display_content[LCM_FW_OK_VER_PAGE_PREVIOUS_UPDATE_INFO][PREVIOUS_TIME_ROW][0], 	"LastUPG:   0 Sec", LCM_DISPLAY_COL);
	memcpy((void *)&lcd_module_display_content[LCM_FW_OK_VER_PAGE_PREVIOUS_UPDATE_INFO][PREVIOUS_FW_ROW][0], 	"FW:             ", LCM_DISPLAY_COL);
}

void lcm_content_init(void)
{
	char const		*welcome_message_line1 =  "TPV UpdateKit V2";
	const uint8_t	welcome_message_line2[] =
	{   'F', 'W', ':',
	    BUILD_YEAR_CH2, BUILD_YEAR_CH3, BUILD_MONTH_CH0, BUILD_MONTH_CH1, BUILD_DAY_CH0, BUILD_DAY_CH1, '-',
		BUILD_HOUR_CH0, BUILD_HOUR_CH1, BUILD_MIN_CH0, BUILD_MIN_CH1, BUILD_SEC_CH0, BUILD_SEC_CH1, '\0'};

	// Prepare firmware version for welcome page

	// Welcome page														 1234567890123456
	memcpy((void *)&lcd_module_display_content[LCM_WELCOME_PAGE][0][0], welcome_message_line1, LCM_DISPLAY_COL);
	memcpy((void *)&lcd_module_display_content[LCM_WELCOME_PAGE][1][0], welcome_message_line2, LCM_DISPLAY_COL);

	// PC Mode page		     										1234567890123456
	memcpy((void *)&lcd_module_display_content[LCM_PC_MODE][0][0], "PC Mode: Press  ", LCM_DISPLAY_COL);
	memcpy((void *)&lcd_module_display_content[LCM_PC_MODE][1][0], "button to change", LCM_DISPLAY_COL);

	// FW Upgrade mode reminder page											   0123456789012345
	memcpy((void *)&lcd_module_display_content[LCM_REMINDER_BEFORE_OUTPUT][0][0], "TV Output:  0.0V", LCM_DISPLAY_COL);
	memcpy((void *)&lcd_module_display_content[LCM_REMINDER_BEFORE_OUTPUT][1][0], "Starts in 5 Sec.", LCM_DISPLAY_COL);

	// FW Upgrading page						  					          1234567890123456
	memcpy((void *)&lcd_module_display_content[LCM_FW_UPGRADING_PAGE][0][0], "Upgrade:   0 Sec", LCM_DISPLAY_COL);
    memcpy((void *)&lcd_module_display_content[LCM_FW_UPGRADING_PAGE][1][0], "OUT: 0.00V 0.00A", LCM_DISPLAY_COL);

	// FW Upgrade too long page						  					             0123456789012345
	memcpy((void *)&lcd_module_display_content[LCM_FW_UPGRADE_TOO_LONG_PAGE][0][0], "Upgrade:   0 Sec", LCM_DISPLAY_COL);
    memcpy((void *)&lcd_module_display_content[LCM_FW_UPGRADE_TOO_LONG_PAGE][1][0], " ** Timeout **  ", LCM_DISPLAY_COL);

	// FW upgrade is done and show software version page				   1234567890123456
    lcm_reset_FW_VER_Content();

	// FW upgrade info of previous update                				   1234567890123456
    lcm_reset_Previous_FW_VER_Content();

	// TV in standby page		     										   1234567890123456
	memcpy((void *)&lcd_module_display_content[LCM_TV_IN_STANDBY_PAGE][0][0], "TV is in Standby", LCM_DISPLAY_COL);
	memcpy((void *)&lcd_module_display_content[LCM_TV_IN_STANDBY_PAGE][1][0], "Pls power on TV ", LCM_DISPLAY_COL);

	// TV is entering ISP mode page		     							   1234567890123456
	memcpy((void *)&lcd_module_display_content[LCM_ENTER_ISP_PAGE][0][0], "Enter ISP mode  ", LCM_DISPLAY_COL);
	memcpy((void *)&lcd_module_display_content[LCM_ENTER_ISP_PAGE][1][0], "Off-On after ISP", LCM_DISPLAY_COL);
	// enable/disable some page/
	memset((void *)lcd_module_display_enable, 0x00, LCM_MAX_PAGE_NO);	// Initial only - later sw determine which page is to be displayed

//    memcpy((void *)&lcd_module_display_content[LCM_DEV_TITLE_PAGE][0][0], 		welcome_message_line2, LCM_DISPLAY_COL);
//    memcpy((void *)&lcd_module_display_content[LCM_DEV_TITLE_PAGE][1][0], 		"Elapse: 0000 Sec", LCM_DISPLAY_COL);
//    memcpy((void *)&lcd_module_display_content[LCM_DEV_MEASURE_PAGE][0][0], 	"OUT: 0.00V 0.00A", LCM_DISPLAY_COL);
//    memcpy((void *)&lcd_module_display_content[LCM_DEV_MEASURE_PAGE][1][0], 	"PWM Duty:100    ", LCM_DISPLAY_COL);
//    memcpy((void *)&lcd_module_display_content[LCM_DEV_UPGRADE_VER_PAGE][0][0], "Ver:            ", LCM_DISPLAY_COL);
//    memcpy((void *)&lcd_module_display_content[LCM_DEV_UPGRADE_VER_PAGE][1][0], "detecting...    ", LCM_DISPLAY_COL);
//    memcpy((void *)&lcd_module_display_content[LCM_DEV_OK_DETECT_PAGE][0][0], 	"PWR detecting...", LCM_DISPLAY_COL);
//    memcpy((void *)&lcd_module_display_content[LCM_DEV_OK_DETECT_PAGE][1][0], 	"OK detecting... ", LCM_DISPLAY_COL);
    //													                      	 1234567890123456
    //memset((void *)lcd_module_display_enable+(uint8_t)LCM_DEV_TITLE_PAGE, 0x01, 4);


}

//void lcd_module_update_message_by_state(uint8_t lcm_msg_state)
//{
//	switch(lcm_msg_state)
//	{
////		case LCM_WELCOME_PAGE:
////			break;
////		case LCM_PC_MODE:
////			break;
//		case LCM_REMINDER_BEFORE_OUTPUT:
//			break;
//		case LCM_FW_UPGRADING_PAGE:
//			break;
//		case LCM_FW_OK_VER_PAGE:
//			break;
//		case LCM_WAIT_NEXT_UPDATE_PAGE:
//			break;
//		case LCM_ENTER_ISP_PAGE:
//			break;
//		default:
//			break;
//	}
//}
//
/**
 * @brief
 * @return
 */
//bool		LED_7_SEG_showing_current = false;

//const char *pwm_voltage_table [POWER_OUTPUT_STEP_TOTAL_NO] = { "0.0", "6.0", "6.5", "7.0", "7.5", "8.0", "8.5", "9.0", "9.5", "9.7" };
const uint8_t StandbyCurrentThresholdLUT[POWER_OUTPUT_STEP_TOTAL_NO] =
	{ 0xff, (DEFAULT_STANDBY_POWER_THRESHOLD*10/60+1), (DEFAULT_STANDBY_POWER_THRESHOLD*10/65+1),
			(DEFAULT_STANDBY_POWER_THRESHOLD*10/70+1), (DEFAULT_STANDBY_POWER_THRESHOLD*10/75+1),
			(DEFAULT_STANDBY_POWER_THRESHOLD*10/80+1), (DEFAULT_STANDBY_POWER_THRESHOLD*10/85+1),
			(DEFAULT_STANDBY_POWER_THRESHOLD*10/90+1), (DEFAULT_STANDBY_POWER_THRESHOLD*10/95+1), (DEFAULT_STANDBY_POWER_THRESHOLD*10/97+1),
	};
static inline uint8_t CalculateStandyCurrent(void)
{
	return (StandbyCurrentThresholdLUT[current_output_stage]);
}

void ResetAllCurrentDebounceTimer(void)
{
	Countdown_Once(FILTER_CURRENT_TV_STANDBY_DEBOUNCE_IN_100MS,(DEFAULT_TV_STANDBY_DEBOUNCE_IN_100MS-1),TIMER_100MS);		// one-shot count down
	Countdown_Once(FILTER_CURRENT_NO_OUTPUT_DEBOUNCE_IN_100MS,(DEFAULT_NO_OUTPUT_DEBOUNCE_IN_100MS-1),TIMER_100MS);			// one-shot count down
	Countdown_Once(FILTER_CURRENT_GOES_NORMAL_DEBOUNCE_IN_100MS,(DEFAULT_OUTPUT_NORMAL_DEBOUNCE_IN_100MS-1),TIMER_100MS);		// one-shot count down
}

void SetRawVoltage(uint16_t voltage_new)
{
	filtered_voltage = Filtered_Input_voltage(voltage_new);
	raw_voltage = voltage_new;
}

void SetRawCurrent(uint16_t current_new)
{
	filtered_current = Filtered_Input_current(current_new);

	// Event checker
	if(filtered_current>=(CalculateStandyCurrent()+20))			// Add 20mA as lower-bound of fw upgrade current
	{
		// output is normal
		if (Read_and_Clear_SW_TIMER_Reload_Flag(FILTER_CURRENT_GOES_NORMAL_DEBOUNCE_IN_100MS))
		{
			EVENT_filtered_current_above_fw_upgrade_threshold = true;
		}
		// reset standby / no-output debounce timer
		Countdown_Once(FILTER_CURRENT_TV_STANDBY_DEBOUNCE_IN_100MS,(DEFAULT_TV_STANDBY_DEBOUNCE_IN_100MS-1),TIMER_100MS);		// one-shot count down
		Countdown_Once(FILTER_CURRENT_NO_OUTPUT_DEBOUNCE_IN_100MS,(DEFAULT_NO_OUTPUT_DEBOUNCE_IN_100MS-1),TIMER_100MS);			// one-shot count down
	}
	else
	{
		// reset output goes normal debounce timer
		Countdown_Once(FILTER_CURRENT_GOES_NORMAL_DEBOUNCE_IN_100MS,(DEFAULT_OUTPUT_NORMAL_DEBOUNCE_IN_100MS-1),TIMER_100MS);		// one-shot count down

		if(filtered_current<default_no_current_threshold_lut[current_output_stage])
		{
			// looks like no output
			// reset standby debounce timer
			Countdown_Once(FILTER_CURRENT_TV_STANDBY_DEBOUNCE_IN_100MS,(DEFAULT_TV_STANDBY_DEBOUNCE_IN_100MS-1),TIMER_100MS);		// Clear Standby debounce timer because it is no_output now
		}
		else
		{
			// looks like standby
			// reset no-output debounce timer
			Countdown_Once(FILTER_CURRENT_NO_OUTPUT_DEBOUNCE_IN_100MS,(DEFAULT_NO_OUTPUT_DEBOUNCE_IN_100MS-1),TIMER_100MS);		// Clear No-output debounce timer because it is standby now
		}

		// Set event if timeout
		if (Read_and_Clear_SW_TIMER_Reload_Flag(FILTER_CURRENT_TV_STANDBY_DEBOUNCE_IN_100MS))
		{
			EVENT_filtered_current_TV_standby_debounced = true;
		}
		if (Read_and_Clear_SW_TIMER_Reload_Flag(FILTER_CURRENT_NO_OUTPUT_DEBOUNCE_IN_100MS))
		{
			EVENT_filtered_current_unplugged_debounced = true;
		}
	}

	raw_current = current_new;
}

uint16_t GetFilteredVoltage(void)
{
	return filtered_voltage;
}

uint16_t GetFilteredCurrent(void)
{
	return filtered_current;
}

void UpdateKitV2_UpdateDisplayValueForADC_Task(void)
{
	char 	 temp_voltage_str[5+1], temp_current_str[5+1], final_voltage_str[5+1], final_current_str[5+1];		// For storing 0x0 at the end of string by +1
	int 	 temp_voltage_str_len, temp_current_str_len;
	//uint16_t temp_value;
	uint8_t	 dp_point;

	// Generate string
	// showing voltage // 0.00v ~ 9.99v
	{
//		temp_value = filtered_voltage;			// 0.001V as unit
//		if(temp_value>9999)
//		{
//			temp_value = 9999;
//		}
		temp_voltage_str_len = itoa_10(filtered_voltage, temp_voltage_str);
	}
	// showing current 0.00A~9.99A -- but current is 0.001A as unit
	{
//		temp_value = filtered_current;
//		if(temp_value>999)		// protection //  showing // 0.00A~9.99A	// 0.001A as unit
//		{
//			temp_value = 999;
//		}
		temp_current_str_len = itoa_10(filtered_current, temp_current_str);
	}

	//
	// Update LCD module display
	//
	// filtered_voltage
	final_voltage_str[4] = 'V';
	switch(temp_voltage_str_len)
	{
		case 1:
			final_voltage_str[0] = '0';
			final_voltage_str[1] = '.';
			final_voltage_str[2] = '0';
			final_voltage_str[3] = '0';
			break;
		case 2:
			final_voltage_str[0] = '0';
			final_voltage_str[1] = '.';
			final_voltage_str[2] = '0';
			final_voltage_str[3] = temp_voltage_str[0];
			break;
		case 3:
			final_voltage_str[0] = '0';
			final_voltage_str[1] = '.';
			final_voltage_str[2] = temp_voltage_str[0];
			final_voltage_str[3] = temp_voltage_str[1];
			break;
		case 4:
			final_voltage_str[0] = temp_voltage_str[0];
			final_voltage_str[1] = '.';
			final_voltage_str[2] = temp_voltage_str[1];
			final_voltage_str[3] = temp_voltage_str[2];
			break;
		default:
			final_voltage_str[0] = '9';
			final_voltage_str[1] = '.';
			final_voltage_str[2] = '9';
			final_voltage_str[3] = '9';
			break;
	}
//	memcpy((void *)&lcd_module_display_content[LCM_DEV_MEASURE_PAGE][0][5], final_voltage_str, 5);
	memcpy((void *)&lcd_module_display_content[LCM_FW_UPGRADING_PAGE][1][5], final_voltage_str, 5);

	// filtered_current
	final_current_str[1] = '.';
	final_current_str[4] = 'A';
	switch(temp_current_str_len)
	{
		case 1:
			final_current_str[0] = '0';
			final_current_str[2] = '0';
			final_current_str[3] = '0';
			break;
		case 2:
			final_current_str[0] = '0';
			final_current_str[2] = '0';
			final_current_str[3] = temp_current_str[0];
			break;
		case 3:
			final_current_str[0] = '0';
			final_current_str[2] = temp_current_str[0];
			final_current_str[3] = temp_current_str[1];
			break;
		case 4:
			final_current_str[0] = temp_current_str[0];
			final_current_str[2] = temp_current_str[1];
			final_current_str[3] = temp_current_str[2];
			break;
		default:
			final_current_str[0] = '9';
			final_current_str[1] = '.';
			final_current_str[2] = '9';
			final_current_str[3] = '9';
			break;
	}
//	memcpy((void *)&lcd_module_display_content[LCM_DEV_MEASURE_PAGE][0][11], final_current_str, 5);
	memcpy((void *)&lcd_module_display_content[LCM_FW_UPGRADING_PAGE][1][11], final_current_str, 5);

	//
	// Update LED 7-segment
	//
//	if(LED_7_SEG_showing_current==false)		// showing voltage // 0.00v ~ 9.99v
	{
		memcpy((void *)&final_voltage_str[1], final_voltage_str+2, 2);	// overwrite '.'
		final_voltage_str[3] = 'U';										// Change 'V' to 'U'
		dp_point = 1;
		Update_LED_7SEG_Message_Buffer(LED_VOLTAGE_PAGE,(uint8_t*)final_voltage_str,dp_point);
	}
//	else
	{
		memcpy((void *)&final_current_str[1], final_current_str+2, 3); // overwrite '.'
		dp_point = 1;
		Update_LED_7SEG_Message_Buffer(LED_CURRENT_PAGE,(uint8_t*)final_current_str,dp_point);
	}
}

void PowerOutputSetting(uint8_t current_step)
{
	if(current_step==0)
	{
		setPWMRate(0, pwm_table[0]);
		Chip_GPIO_SetPinOutLow(LPC_GPIO, VOUT_ENABLE_GPIO_PORT, VOUT_ENABLE_GPIO_PIN);
	}
	else
	{
		setPWMRate(0, pwm_table[current_step]);
		Chip_GPIO_SetPinOutHigh(LPC_GPIO, VOUT_ENABLE_GPIO_PORT, VOUT_ENABLE_GPIO_PIN);
	}
}

uint16_t Filtered_Input_current(uint16_t latest_current)
{
	uint16_t	temp;

	if(RingBuffer_IsFull(&current_history))
	{
		RingBuffer_Pop(&current_history, &temp);
		total_current_value -= temp;
	}
	total_current_value += latest_current;
	RingBuffer_Insert(&current_history, &latest_current);

	return (total_current_value/CURRENT_HISTORY_DATA_SIZE);
}

uint16_t Filtered_Input_voltage(uint16_t latest_voltage)
{
	uint16_t	temp;

	if(RingBuffer_IsFull(&voltage_history))
	{
		RingBuffer_Pop(&voltage_history, &temp);
		total_voltage_value -= temp;
	}
	total_voltage_value += latest_voltage;
	RingBuffer_Insert(&voltage_history, &latest_voltage);

	return (total_voltage_value/VOLTAGE_HISTORY_DATA_SIZE);
}

static inline void Copy_Existing_FW_Upgrade_Info_to_Previous_Info(void)
{
	// Move existing FW upgrade info to previous update info page               				                        1234567890123456
	memcpy((void *)&lcd_module_display_content[LCM_FW_OK_VER_PAGE_PREVIOUS_UPDATE_INFO][PREVIOUS_TIME_ROW][PREVIOUS_TIME_POS], (void *)&lcd_module_display_content[LCM_FW_OK_VER_PAGE][OK_TIME_ROW][OK_TIME_POS], ELAPSE_TIME_LEN);
	memcpy((void *)&lcd_module_display_content[LCM_FW_OK_VER_PAGE_PREVIOUS_UPDATE_INFO][PREVIOUS_FW_ROW][PREVIOUS_FW_POS], (void *)&lcd_module_display_content[LCM_FW_OK_VER_PAGE][CURRENT_FW_ROW][CURRENT_FW_POS], MAX_FW_VER_LEN);
	lcm_reset_FW_VER_Content();
}

static inline void LCM_Fill_Version_String(void)
{
	char	*temp_str = (char *) Get_VER_string();
	uint8_t	temp_len = strlen(temp_str);
	if (temp_len>MAX_FW_VER_LEN)
	{
		temp_len = MAX_FW_VER_LEN;
	}
	memcpy((void *)&lcd_module_display_content[LCM_FW_OK_VER_PAGE][CURRENT_FW_ROW][CURRENT_FW_POS], temp_str, temp_len);
	memset((void *)&lcd_module_display_content[LCM_FW_OK_VER_PAGE][CURRENT_FW_ROW][CURRENT_FW_POS+temp_len], ' ', MAX_FW_VER_LEN-(temp_len));
}

static inline void Change_Output_Selection(void)
{
	if(++current_output_stage>=POWER_OUTPUT_STEP_TOTAL_NO)
	{
		current_output_stage = 0;
	}
}

bool Event_Proc_State_Independent(void)
{
	bool	bRet = false;

#ifdef POWERON_IS_DETECTING
	if(EVENT_POWERON_string_confirmed)
	{
		Clear_POWERON_pattern();
		EVENT_POWERON_string_confirmed = false;
		bRet = true;
	}
#endif // #ifdef POWERON_IS_DETECTING

	if(EVENT_Button_pressed_debounced)
	{
		// Always no matter which system state
		LED_7SEG_ForceToSpecificPage(LED_VOLTAGE_PAGE);
		Set_SW_Timer_Count(LED_VOLTAGE_CURRENT_DISPLAY_SWAP_IN_SEC,(DEFAULT_LED_DATA_CHANGE_SEC-1));
		Clear_SW_TIMER_Reload_Flag(LED_VOLTAGE_CURRENT_DISPLAY_SWAP_IN_SEC);
		// Do not clear this event because it is used by Event_Proc_by_System_State()
		bRet = true;
	}

	return bRet;
}

UPDATE_STATE Event_Proc_by_System_State(UPDATE_STATE current_state)
{
	UPDATE_STATE return_next_state = current_state;

	// Apply to specific state
	switch(current_state)
	{
		case US_SYSTEM_WELCOME:
			if(EVENT_Button_pressed_debounced)
			{
				EVENT_Button_pressed_debounced = false;
				return_next_state = US_CHECK_USER_SELECTION;
			}
			break;
//		case US_CHECK_USER_SELECTION:
//			break;
		case US_COUNTDOWN_BEFORE_OUTPUT:
		case US_PC_MODE_VOLTAGE_LOW:
			if(EVENT_Button_pressed_debounced)
			{
				EVENT_Button_pressed_debounced = false;
				Change_Output_Selection();
				return_next_state = US_CHECK_USER_SELECTION;
			}
			break;
		case US_START_OUTPUT:
			if (EVENT_filtered_current_TV_standby_debounced)
			{
				EVENT_filtered_current_TV_standby_debounced = false;
				return_next_state = US_TV_IN_STANDBY;
			}
			else if(EVENT_filtered_current_above_fw_upgrade_threshold)
			{
				EVENT_filtered_current_above_fw_upgrade_threshold = false;
				return_next_state = US_WAIT_FW_UPGRADE_OK_STRING;
			}
			else if(EVENT_filtered_current_unplugged_debounced)
			{
				EVENT_filtered_current_unplugged_debounced = false;
				Pause_SW_Timer(UPGRADE_ELAPSE_IN_S);
				Set_SW_Timer_Count(UPGRADE_ELAPSE_IN_S,0);
			}
			break;
		case US_WAIT_FW_UPGRADE_OK_STRING:
		case US_UPGRADE_TOO_LONG:
			if(EVENT_Version_string_confirmed)
			{
				LCM_Fill_Version_String();
				EVENT_Version_string_confirmed = false;
			}
			if(EVENT_OK_string_confirmed)
			{
				return_next_state = US_FW_UPGRADE_DONE;
				EVENT_OK_string_confirmed = false;
			}
			else if (EVENT_filtered_current_TV_standby_debounced)
			{
				EVENT_filtered_current_TV_standby_debounced = false;
				return_next_state = US_TV_IN_STANDBY;
			}
			else if (EVENT_filtered_current_unplugged_debounced)
			{
				EVENT_filtered_current_unplugged_debounced = false;
				return_next_state = US_READY_FOR_NEXT_UPDATE;
			}
//			else if(EVENT_filtered_current_above_fw_upgrade_threshold)
			{
				EVENT_filtered_current_above_fw_upgrade_threshold = false;
			}
			break;
		case US_FW_UPGRADE_DONE:
			if(EVENT_Version_string_confirmed)
			{
				LCM_Fill_Version_String();
				EVENT_Version_string_confirmed = false;
			}
			// Next update is triggered only after no_output -- no changed if standby
			if (EVENT_filtered_current_unplugged_debounced)
			{
				EVENT_filtered_current_unplugged_debounced = false;
				Copy_Existing_FW_Upgrade_Info_to_Previous_Info();		// Save current successful FW upgrade info before it is cleared in next state
				return_next_state = US_READY_FOR_NEXT_UPDATE;
			}
//			else if (EVENT_filtered_current_TV_standby_debounced)
			{
				EVENT_filtered_current_TV_standby_debounced = false;
			}
//			else if(EVENT_filtered_current_above_fw_upgrade_threshold)
			{
				EVENT_filtered_current_above_fw_upgrade_threshold = false;
			}
			break;
		case US_READY_FOR_NEXT_UPDATE:
			if (EVENT_filtered_current_TV_standby_debounced)
			{
				EVENT_filtered_current_TV_standby_debounced = false;
				return_next_state = US_TV_IN_STANDBY;
			}
			else if(EVENT_filtered_current_above_fw_upgrade_threshold)
			{
				EVENT_filtered_current_above_fw_upgrade_threshold = false;
				return_next_state = US_WAIT_FW_UPGRADE_OK_STRING;
			}
//			else if (EVENT_filtered_current_unplugged_debounced)
			{
				EVENT_filtered_current_unplugged_debounced = false;
			}
			break;
		case US_TV_IN_STANDBY:
//			if(EVENT_filtered_current_above_threshold)
//				return_next_state = US_WAIT_FW_UPGRADE_OK_STRING;
//			else if (EVENT_filtered_current_no_output)
//				return_next_state = US_TV_LEAVE_STANDBY;
			if (EVENT_filtered_current_unplugged_debounced)
			{
				EVENT_filtered_current_unplugged_debounced = false;
				return_next_state = US_READY_FOR_NEXT_UPDATE;
			}
			else if(EVENT_filtered_current_above_fw_upgrade_threshold)
			{
				EVENT_filtered_current_above_fw_upgrade_threshold = false;
				return_next_state = US_WAIT_FW_UPGRADE_OK_STRING;
			}
//			else if (EVENT_filtered_current_TV_standby_debounced)
			{
				EVENT_filtered_current_TV_standby_debounced = false;
			}
			break;
		default:
			break;
	}
	return return_next_state;
}

static inline void Update_FW_Upgrading_Elapse_Time(void)
{
	itoa_10_fixed_position(	Read_SW_TIMER_Value(UPGRADE_ELAPSE_IN_S),
							(char*)&lcd_module_display_content[LCM_FW_UPGRADING_PAGE][ELAPSE_TIME_ROW][ELAPSE_TIME_POS],
							ELAPSE_TIME_LEN);
}

static inline void Update_FW_OK_Upgrade_Time(void)
{
	itoa_10_fixed_position(	Read_SW_TIMER_Value(UPGRADE_ELAPSE_IN_S),
							(char*)&lcd_module_display_content[LCM_FW_OK_VER_PAGE][OK_TIME_ROW][OK_TIME_POS],
							ELAPSE_TIME_LEN);
}

UPDATE_STATE System_State_Running_Proc(UPDATE_STATE current_state)
{
	UPDATE_STATE return_next_state = current_state;
	switch(current_state)
	{
		case US_SYSTEM_BOOTUP_STATE:
			return_next_state = US_SYSTEM_WELCOME;
			break;
		case US_COUNTDOWN_BEFORE_OUTPUT:
			// update countdown seconds to output on LCM
			lcd_module_display_content[LCM_REMINDER_BEFORE_OUTPUT][COUNTDOWN_TIME_ROW][COUNTDOWN_TIME_POS] = (Read_SW_TIMER_Value(SYSTEM_STATE_PROC_TIMER))+'0';	// Timer here should be 1000ms as unit
			break;
		case US_PC_MODE_VOLTAGE_LOW:
			// Save User Selection after 5 seconds
			if(Check_if_different_from_last_ReadWrite(current_output_stage))
			{
				if(Read_SW_TIMER_Value(SYSTEM_STATE_PROC_TIMER)>=5)
				{
					Save_User_Selection(current_output_stage);
				}
			}
			break;
		case US_WAIT_FW_UPGRADE_OK_STRING:
			Update_FW_Upgrading_Elapse_Time();
			break;
		case US_UPGRADE_TOO_LONG:
			// Update Upgrade-elapse-time
			{
				uint8_t	elapse_time = Read_SW_TIMER_Value(UPGRADE_ELAPSE_IN_S);
				if(elapse_time<=9999)
				{
					Update_FW_Upgrading_Elapse_Time();
				}
			}
			break;
		default:
			break;
	}
	return return_next_state;
}

UPDATE_STATE System_State_End_Proc(UPDATE_STATE current_state)
{
	UPDATE_STATE return_next_state = current_state;
	switch(current_state)
	{
		case US_SYSTEM_BOOTUP_STATE:
			return_next_state = US_SYSTEM_WELCOME;
			break;
		case US_SYSTEM_WELCOME:
			return_next_state = US_CHECK_USER_SELECTION;
			break;
		case US_CHECK_USER_SELECTION:
			if(current_output_stage!=0)
				return_next_state = US_COUNTDOWN_BEFORE_OUTPUT;
			else
				return_next_state = US_PC_MODE_VOLTAGE_LOW;
			break;
		case US_COUNTDOWN_BEFORE_OUTPUT:
			return_next_state = US_START_OUTPUT;
			break;
		case US_WAIT_FW_UPGRADE_OK_STRING:
			return_next_state = US_UPGRADE_TOO_LONG;
			break;
		case US_PC_MODE_VOLTAGE_LOW:
		case US_START_OUTPUT:
		case US_FW_UPGRADE_DONE:
		case US_UPGRADE_TOO_LONG:
		case US_READY_FOR_NEXT_UPDATE:
		case US_TV_IN_STANDBY:
			break;
		default:
			break;
	}
	return return_next_state;
}

UPDATE_STATE System_State_Begin_Proc(UPDATE_STATE current_state)
{
	UPDATE_STATE return_next_state = current_state;

	switch(current_state)
	{
		case US_SYSTEM_BOOTUP_STATE:
		case US_SYSTEM_WELCOME:
			Load_User_Selection(&current_output_stage);
			lcm_page_change_duration_in_sec = DEFAULT_LCM_PAGE_CHANGE_S_WELCOME;
			lcd_module_display_enable_only_one_page(LCM_WELCOME_PAGE);
			// Clear events if we want to check it at this state
			EVENT_Button_pressed_debounced = false;
			Countdown_Once(SYSTEM_STATE_PROC_TIMER,(WELCOME_MESSAGE_DISPLAY_TIME_IN_S-1),TIMER_S);
			break;
		case US_CHECK_USER_SELECTION:
			Raise_SW_TIMER_Reload_Flag(SYSTEM_STATE_PROC_TIMER);		// enter next state without timer down to 0
			break;
		case US_COUNTDOWN_BEFORE_OUTPUT:
			memcpy((void *)&lcd_module_display_content[LCM_REMINDER_BEFORE_OUTPUT][OUTPUT_SELECT_ROW][OUTPUT_SELECT_POS], pwm_voltage_table[current_output_stage], OUTPUT_SELECT_LEN);
			lcd_module_display_enable_only_one_page(LCM_REMINDER_BEFORE_OUTPUT);
			Countdown_Once(SYSTEM_STATE_PROC_TIMER,OUTPUT_REMINDER_DISPLAY_TIME_IN_S,TIMER_S);		// one-shot count down
			break;
		case US_PC_MODE_VOLTAGE_LOW:
			lcd_module_display_enable_only_one_page(LCM_PC_MODE);
			Start_SW_Timer(SYSTEM_STATE_PROC_TIMER,~1,~1,TIMER_S, false, false);		// endless timer max->0 repeating countdown from max
			break;
		case US_START_OUTPUT:
			LED_Status_Set_Value(LED_STATUS_ALL);
			LED_Status_Set_Auto_Toggle(LED_STATUS_ALL,LED_STATUS_TOGGLE_DURATION_IN_100MS_FAST,6);
			PowerOutputSetting(current_output_stage);
			Start_SW_Timer(UPGRADE_ELAPSE_IN_S,0,~1,TIMER_S, true, true);
			if(Check_if_different_from_last_ReadWrite(current_output_stage))
			{
				Save_User_Selection(current_output_stage);
			}
			ResetAllCurrentDebounceTimer();	// Force to init after first output
			lcd_module_display_enable_only_one_page(LCM_FW_UPGRADING_PAGE);
			Start_SW_Timer(SYSTEM_STATE_PROC_TIMER,~1,~1,TIMER_S, false, false);		// endless timer max->0 repeating countdown from max
			break;
		case US_WAIT_FW_UPGRADE_OK_STRING:
			Play_SW_Timer(UPGRADE_ELAPSE_IN_S);
			// Clear events if we want to check it at this state
			EVENT_OK_string_confirmed = false;
			EVENT_Version_string_confirmed = false;
			Clear_OK_pattern_state();
			Clear_POWERON_pattern();
			Clear_VER_string();
			lcd_module_display_enable_only_one_page(LCM_FW_UPGRADING_PAGE);
			LED_Status_Clear_Auto_Toggle(LED_STATUS_ALL);
			LED_Status_Set_Value(LED_STATUS_Y);		// only LED_Y
			LED_Status_Set_Auto_Toggle(LED_STATUS_Y,LED_STATUS_TOGGLE_DURATION_IN_100MS,~1);
			Countdown_Once(SYSTEM_STATE_PROC_TIMER,(max_upgrade_time_in_S-1),TIMER_S);		// one-shot count down
			break;
		case US_FW_UPGRADE_DONE:
			Pause_SW_Timer(UPGRADE_ELAPSE_IN_S);
			Update_FW_OK_Upgrade_Time();
			max_upgrade_time_in_S = CHANGE_FW_MAX_UPDATE_TIME_AFTER_OK(Read_SW_TIMER_Value(UPGRADE_ELAPSE_IN_S));
			lcd_module_display_enable_only_one_page(LCM_FW_OK_VER_PAGE);
			lcd_module_display_enable_page(LCM_FW_OK_VER_PAGE_PREVIOUS_UPDATE_INFO);
			lcm_page_change_duration_in_sec = DEFAULT_LCM_PAGE_CHANGE_S_OK;
			LED_Status_Clear_Auto_Toggle(LED_STATUS_ALL);
			LED_Status_Set_Value(LED_STATUS_G);		// only LED_G
			Start_SW_Timer(SYSTEM_STATE_PROC_TIMER,~1,~1,TIMER_S, false, false);		// endless timer max->0 repeating countdown from max
			break;
		case US_UPGRADE_TOO_LONG:
			LED_Status_Clear_Auto_Toggle(LED_STATUS_ALL);
			LED_Status_Set_Value(LED_STATUS_R);		// only LED_R
			lcd_module_display_enable_only_one_page(LCM_FW_UPGRADE_TOO_LONG_PAGE);
			Start_SW_Timer(SYSTEM_STATE_PROC_TIMER,~1,~1,TIMER_S, false, false);		// endless timer max->0 repeating countdown from max
			break;
		case US_READY_FOR_NEXT_UPDATE:
			Pause_SW_Timer(UPGRADE_ELAPSE_IN_S);
			Set_SW_Timer_Count(UPGRADE_ELAPSE_IN_S,0);
			Update_FW_Upgrading_Elapse_Time();
			LED_Status_Set_Value(LED_STATUS_ALL);
			LED_Status_Set_Auto_Toggle(LED_STATUS_ALL,LED_STATUS_TOGGLE_DURATION_IN_100MS_FAST,6);
			lcd_module_display_enable_only_one_page(LCM_FW_UPGRADING_PAGE);
			Start_SW_Timer(SYSTEM_STATE_PROC_TIMER,~1,~1,TIMER_S, false, false);		// endless timer max->0 repeating countdown from max
			break;
		case US_TV_IN_STANDBY:
			Pause_SW_Timer(UPGRADE_ELAPSE_IN_S);
			Set_SW_Timer_Count(UPGRADE_ELAPSE_IN_S,0);
			LED_Status_Clear_Auto_Toggle(LED_STATUS_ALL);
			LED_Status_Set_Value(LED_STATUS_R);		// only LED_R
			lcd_module_display_enable_only_one_page(LCM_TV_IN_STANDBY_PAGE);
			Start_SW_Timer(SYSTEM_STATE_PROC_TIMER,~1,~1,TIMER_S, false, false);		// endless timer max->0 repeating countdown from max
			break;
		default:
			// fall-back code
			Raise_SW_TIMER_Reload_Flag(SYSTEM_STATE_PROC_TIMER);		// enter next state without timer down to 0
			break;
	}

	return return_next_state;
}

bool UART_input_processor(uint8_t key)
{
	bool	bRet_any_event_raised = false;

	if(locate_OK_pattern_process(key))
	{
		EVENT_OK_string_confirmed = true;
		bRet_any_event_raised = true;
	}

#ifdef POWERON_IS_DETECTING
	// To identify @POWERON
	locate_POWERON_pattern_process(key);
	if(Get_POWERON_pattern()==true)
	{
		Clear_POWERON_pattern();
		EVENT_POWERON_string_confirmed = true;
		bRet_any_event_raised = true;
	}
#endif //#ifdef POWERON_IS_DETECTING

	// To identify @VER
	locate_VER_pattern_process(key);
	if(Found_VER_string()==true)
	{
		EVENT_Version_string_confirmed = true;
		bRet_any_event_raised = true;
	}

	return bRet_any_event_raised;
}
