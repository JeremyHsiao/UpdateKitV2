/*

	UpdateKitV2 main code

 */

#include "chip.h"
#include "board.h"
#include "uart_0_rb.h"
#include "gpio.h"
#include "string_detector.h"
#include "lcd_module.h"
#include "sw_timer.h"
#include "string.h"
#include "UpdateKitV2.h"
#include "build_defs.h"
#include "event.h"
#include "user_opt.h"
#include "fw_version.h"

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/
uint8_t				current_output_stage;
const uint8_t		pwm_table[POWER_OUTPUT_STEP_TOTAL_NO] = 		  {   100,   100,     52,    44,    37,    29,    21,    15,     4,    0  };
const char 			*pwm_voltage_table [POWER_OUTPUT_STEP_TOTAL_NO] = { "0.0", "6.0", "6.5", "7.0", "7.5", "8.0", "8.5", "9.0", "9.5", "9.8" };
const uint8_t		default_no_current_threshold_lut[POWER_OUTPUT_STEP_TOTAL_NO] = { 9, 9, 9, 9, 9, 9, 9, 9, 9, 9 };
const uint16_t		target_voltage_table[POWER_OUTPUT_STEP_TOTAL_NO] = { 0, 6000, 6500, 7000, 7500, 8000, 8500, 9000, 9500, 9800 };

#define	CURRENT_HISTORY_DATA_SIZE	32
RINGBUFF_T 	current_history;
uint16_t 	current_history_data[CURRENT_HISTORY_DATA_SIZE];
uint32_t	total_current_value = 0;

#define	VOLTAGE_HISTORY_DATA_SIZE	32
RINGBUFF_T 	voltage_history;
uint16_t 	voltage_history_data[VOLTAGE_HISTORY_DATA_SIZE];
uint32_t	total_voltage_value;

uint16_t			raw_voltage;			//  0.00v ~ 9.99v --> 0-999
uint16_t			raw_current;			// .000A ~ .999A --> 0-999
uint16_t			filtered_voltage;		//  0.00v ~ 9.99v --> 0-999
uint16_t			filtered_current;		// .000A ~ .999A --> 0-999

#define ELAPSE_TIME_MAX_DISPLAY_VALUE		(9999)
/*
#define			ROW_FOR_VER		((MAX_VER_NO_LEN%LCM_DISPLAY_COL)!=0)?((MAX_VER_NO_LEN/LCM_DISPLAY_COL)+1):(MAX_VER_NO_LEN/LCM_DISPLAY_COL)
uint8_t			lcd_ok_full_info[(ROW_FOR_VER*2+2)*LCM_DISPLAY_COL];
uint32_t		lcd_ok_full_info_str_current_pos;
*/
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

void Init_Value_From_EEPROM(void)
{
	Load_User_Selection(&current_output_stage);
	Load_System_Timeout_v2(current_output_stage,&max_upgrade_time_in_S);
}

void Init_UpdateKitV2_variables(void)
{
	current_system_proc_state = US_SYSTEM_BOOTUP_STATE;
	//max_upgrade_time_in_S = DEFAULT_MAX_FW_UPDATE_TIME_IN_S;
	lcm_page_change_duration_in_sec = DEFAULT_LCM_PAGE_CHANGE_S_WELCOME;
	raw_voltage = 0;			//  0.00v ~ 9.99v --> 0-999
	raw_current = 0;			// .000A ~ .999A --> 0-999
	filtered_voltage = 0;		//  0.00v ~ 9.99v --> 0-999
	filtered_current = 0;		// .000A ~ .999A --> 0-999
	total_current_value = 0;
	//current_output_stage = DEFAULT_POWER_OUTPUT_STEP;
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
/*
static inline uint32_t full_page_string_copy(uint32_t starting_pos, const void * restrict __s2, size_t len)
{
	memcpy((void *)&lcd_ok_full_info[starting_pos],__s2, len);
	return starting_pos+len;
}
*/
static inline bool lcm_text_buffer_cpy(LCM_PAGE_ID page_id, uint8_t row, uint8_t col, const void * restrict __s2, size_t len)
{
	// If row/col is out-of-range, skip
	if((row>=LCM_DISPLAY_ROW)||(col>=LCM_DISPLAY_COL))
	{
		return false;
	}
	// If size is larger, simply skip out-of-boundary text
	if((col+len-1)>=LCM_DISPLAY_COL)
	{
		len=LCM_DISPLAY_COL-col;
	}
	// execute actual memcpy
	memcpy((void *)&lcd_module_display_content[page_id][row][col],__s2, len);
	return true;
}

void lcm_content_init(void)
{
	char const		*welcome_message_line1 =  "Hot Spring Board";
	const uint8_t	welcome_message_line2[] =
	{   'F', 'W', ':', 'V', FW_MAJOR, FW_MIDDLE, FW_MINOR, '_', // "FW:Vx.x-" - total 8 chars
	   BUILD_MONTH_CH0, BUILD_MONTH_CH1, BUILD_DAY_CH0, BUILD_DAY_CH1, BUILD_HOUR_CH0, BUILD_HOUR_CH1,  BUILD_MIN_CH0, BUILD_MIN_CH1, // 8 chars
		'\0'};

	// Prepare firmware version for welcome page

	// Welcome page														 1234567890123456
	lcm_text_buffer_cpy(LCM_WELCOME_PAGE,0,0,welcome_message_line1,LCM_DISPLAY_COL);
	lcm_text_buffer_cpy(LCM_WELCOME_PAGE,1,0,welcome_message_line2,LCM_DISPLAY_COL);

//	// PC Mode page		     			 1234567890123456
	lcm_text_buffer_cpy(LCM_PC_MODE,0,0,"PC Mode: Press  ", LCM_DISPLAY_COL);
	lcm_text_buffer_cpy(LCM_PC_MODE,1,0,"button to change", LCM_DISPLAY_COL);

	// enable/disable some page/
	memset((void *)lcd_module_display_enable, 0x00, LCM_MAX_PAGE_NO);	// Initial only - later sw determine which page is to be displayed

}

