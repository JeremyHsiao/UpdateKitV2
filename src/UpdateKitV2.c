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

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/
bool		EVENT_raw_current_goes_above_threshold = false;
bool		EVENT_raw_current_goes_below_threshold = false;
bool		EVENT_filtered_current_goes_above_threshold = false;
bool		EVENT_filtered_current_goes_below_threshold = false;
bool		EVENT_OK_string_confirmed = false;
bool		EVENT_Version_string_confirmed = false;
bool		EVENT_POWERON_string_confirmed = false;
bool		EVENT_Button_pressed_debounced = false;

UPDATE_STATE	upcoming_system_state = US_SYSTEM_STARTUP_WELCOME_MESSAGE;

/*****************************************************************************
 * Private functions
 ****************************************************************************/

/*****************************************************************************
 * Public functions
 ****************************************************************************/

void lcm_content_init_old(void)
{
//    strcpy((void *)&lcd_module_display_content[0][0][0], "TPV UpdateKit V2" __TIME__);
    //lcd_module_display_enable[3] = 0x00;
     //memset((void *)lcd_module_display_enable, 0x00, 4);
     //lcd_module_display_enable[1]=1;
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

	// FW Upgrade mode reminder page											   1234567890123456
	memcpy((void *)&lcd_module_display_content[LCM_REMINDER_BEFORE_OUTPUT][0][0], "TV Output:  0.0V", LCM_DISPLAY_COL);
	memcpy((void *)&lcd_module_display_content[LCM_REMINDER_BEFORE_OUTPUT][1][0], "Starts in 5 Sec.", LCM_DISPLAY_COL);

	// FW Upgrading page						  					          1234567890123456
	memcpy((void *)&lcd_module_display_content[LCM_FW_UPGRADING_PAGE][0][0], "Upgrade: 000 Sec", LCM_DISPLAY_COL);
    memcpy((void *)&lcd_module_display_content[LCM_FW_UPGRADING_PAGE][1][0], "OUT: 0.00V 0.00A", LCM_DISPLAY_COL);

	// FW upgrade is done and show software version page				   1234567890123456
    memcpy((void *)&lcd_module_display_content[LCM_FW_OK_VER_PAGE][0][0], "Upgrade: 000 Sec", LCM_DISPLAY_COL);
	memcpy((void *)&lcd_module_display_content[LCM_FW_OK_VER_PAGE][1][0], "FW:             ", LCM_DISPLAY_COL);

	// TV in standby page		     										   1234567890123456
	memcpy((void *)&lcd_module_display_content[LCM_TV_IN_STANDBY_PAGE][0][0], "TV is in Standby", LCM_DISPLAY_COL);
	memcpy((void *)&lcd_module_display_content[LCM_TV_IN_STANDBY_PAGE][1][0], "Pls power on TV ", LCM_DISPLAY_COL);

	// TV is entering ISP mode page		     							   1234567890123456
	memcpy((void *)&lcd_module_display_content[LCM_ENTER_ISP_PAGE][0][0], "Enter ISP mode  ", LCM_DISPLAY_COL);
	memcpy((void *)&lcd_module_display_content[LCM_ENTER_ISP_PAGE][1][0], "Off-On after ISP", LCM_DISPLAY_COL);
	// enable/disable some page/
	memset((void *)lcd_module_display_enable, 0x00, LCM_MAX_PAGE_NO);	// Initial only - later sw determine which page is to be displayed

    memcpy((void *)&lcd_module_display_content[LCM_DEV_TITLE_PAGE][0][0], 		welcome_message_line2, LCM_DISPLAY_COL);
    memcpy((void *)&lcd_module_display_content[LCM_DEV_TITLE_PAGE][1][0], 		"Elapse: 0000 Sec", LCM_DISPLAY_COL);
    memcpy((void *)&lcd_module_display_content[LCM_DEV_MEASURE_PAGE][0][0], 	"OUT: 0.00V 0.00A", LCM_DISPLAY_COL);
    memcpy((void *)&lcd_module_display_content[LCM_DEV_MEASURE_PAGE][1][0], 	"PWM Duty:100    ", LCM_DISPLAY_COL);
    memcpy((void *)&lcd_module_display_content[LCM_DEV_UPGRADE_VER_PAGE][0][0], "Ver:            ", LCM_DISPLAY_COL);
    memcpy((void *)&lcd_module_display_content[LCM_DEV_UPGRADE_VER_PAGE][1][0], "detecting...    ", LCM_DISPLAY_COL);
    memcpy((void *)&lcd_module_display_content[LCM_DEV_OK_DETECT_PAGE][0][0], 	"PWR detecting...", LCM_DISPLAY_COL);
    memcpy((void *)&lcd_module_display_content[LCM_DEV_OK_DETECT_PAGE][1][0], 	"OK detecting... ", LCM_DISPLAY_COL);
    //													                      	 1234567890123456
    //memset((void *)lcd_module_display_enable+(uint8_t)LCM_DEV_TITLE_PAGE, 0x01, 4);


}

void lcd_module_update_message_by_state(uint8_t lcm_msg_state)
{
	switch(lcm_msg_state)
	{
//		case LCM_WELCOME_PAGE:
//			break;
//		case LCM_PC_MODE:
//			break;
		case LCM_REMINDER_BEFORE_OUTPUT:
			break;
		case LCM_FW_UPGRADING_PAGE:
			break;
		case LCM_FW_OK_VER_PAGE:
			break;
		case LCM_TV_IN_STANDBY_PAGE:
			break;
		case LCM_ENTER_ISP_PAGE:
			break;
		default:
			break;
	}
}

/**
 * @brief
 * @return
 */
uint16_t	raw_voltage = 0;			//  0.00v ~ 9.99v --> 0-999
uint16_t	raw_current = 0;			// .000A ~ .999A --> 0-999
uint16_t	filtered_voltage = 0;		//  0.00v ~ 9.99v --> 0-999
uint16_t	filtered_current = 0;		// .000A ~ .999A --> 0-999

bool		LED_7_SEG_showing_current = false;

void SetRawVoltage(uint16_t voltage_new)
{
	filtered_voltage = Filtered_Input_voltage(voltage_new);
	raw_voltage = voltage_new;
}

void SetRawCurrent(uint16_t current_new)
{
	uint16_t	previous_filtered_current;

	// Event checker
	if((current_new>=DEFAULT_INPUT_CURRENT_THRESHOLD)&&(raw_current<DEFAULT_INPUT_CURRENT_THRESHOLD))
	{
		EVENT_raw_current_goes_above_threshold = true;
	}
	// Event checker
	if((current_new<DEFAULT_INPUT_CURRENT_THRESHOLD)&&(raw_current>=DEFAULT_INPUT_CURRENT_THRESHOLD))
	{
		EVENT_raw_current_goes_below_threshold = true;
	}

	previous_filtered_current = filtered_current;
	filtered_current = Filtered_Input_current(current_new);
	// Event checker
	if((filtered_current>=DEFAULT_INPUT_CURRENT_THRESHOLD)&&(previous_filtered_current<DEFAULT_INPUT_CURRENT_THRESHOLD))
	{
		EVENT_filtered_current_goes_above_threshold = true;
	}
	// Event checker
	if((filtered_current<DEFAULT_INPUT_CURRENT_THRESHOLD)&&(previous_filtered_current>=DEFAULT_INPUT_CURRENT_THRESHOLD))
	{
		EVENT_filtered_current_goes_below_threshold = true;
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

void UpdateKitV2_LED_7_ToggleDisplayVoltageCurrent(void)
{
	LED_7_SEG_showing_current = !LED_7_SEG_showing_current;
}

void UpdateKitV2_LED_7_StartDisplayVoltage(void)
{
	LED_Voltage_Current_Refresh_in_sec = LED_Voltage_Current_Refresh_reload;
	LED_Voltage_Current_Refresh_in_sec_timeout = true;	// force to update at next tick
	LED_7_SEG_showing_current = true;					// Toggle to showing voltage at next tick
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
	// showing current 0.00A~0.99A -- but current is 0.0001A as unit
	{
//		temp_value = filtered_current;
//		if(temp_value>999)		// protection //  showing // 0.00A~0.99A	// 0.001A as unit
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
			final_voltage_str[3] = '0';;
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
	memcpy((void *)&lcd_module_display_content[LCM_DEV_MEASURE_PAGE][0][5], final_voltage_str, 5);
	memcpy((void *)&lcd_module_display_content[LCM_FW_UPGRADING_PAGE][1][5], final_voltage_str, 5);

	// filtered_current
	final_current_str[0] = '0';
	final_current_str[1] = '.';
	final_current_str[4] = 'A';
	switch(temp_current_str_len)
	{
		case 1:
			final_current_str[2] = '0';
			final_current_str[3] = '0';
			break;
		case 2:
			final_current_str[2] = '0';
			final_current_str[3] = temp_current_str[0];
			break;
		case 3:
			final_current_str[2] = temp_current_str[0];
			final_current_str[3] = temp_current_str[1];
			break;
		default:
			final_current_str[2] = '9';
			final_current_str[3] = '9';
			break;
	}
	memcpy((void *)&lcd_module_display_content[LCM_DEV_MEASURE_PAGE][0][11], final_current_str, 5);
	memcpy((void *)&lcd_module_display_content[LCM_FW_UPGRADING_PAGE][1][11], final_current_str, 5);

	//
	// Update LED 7-segment
	//
	if(LED_7_SEG_showing_current==false)		// showing voltage // 0.00v ~ 9.99v
	{
		memcpy((void *)&final_voltage_str[1], final_voltage_str+2, 2);	// overwrite '.'
		final_voltage_str[3] = 'U';										// Change 'V' to 'U'
		dp_point = 1;
		Update_LED_7SEG_Message_Buffer((uint8_t*)final_voltage_str,dp_point);
	}
	else
	{
		memcpy((void *)&final_current_str[1], final_current_str+2, 3); // overwrite '.'
		dp_point = 1;
		Update_LED_7SEG_Message_Buffer((uint8_t*)final_current_str,dp_point);
	}
}


uint8_t		current_output_stage = DEFAULT_POWER_OUTPUT_STEP;
//uint8_t		pwm_table[25] = { 100, 77, 76, 75, 74,   73, 72,  71, 70, 65,    64, 63, 54, 53, 52,     34, 33, 32, 31, 5,   4, 3, 2, 1, 0};
uint8_t		pwm_table[10] = { 100, 58,  49, 41,   33,   26,    19,   12,   4,   0};
//Key toggle :				 0V, 6.0V ,6.5V, 7V,  7.5V, 8V,   8.5V, 9V,  9.5V, 10V
						//	0 / 680 / 702 / 749 / 799 / 852 / 909 / 948/ 980

char *pwm_voltage_table [] = { "0.0", "6.0", "6.5", "7.0", "7.5", "8.0", "8.5", "9.0", "9.5", "9.7" };

void PowerOutputSetting(uint8_t current_step)
{
	if(current_step==0)
	{
		setPWMRate(0, pwm_table[current_output_stage]);
		Chip_GPIO_SetPinOutLow(LPC_GPIO, VOUT_ENABLE_GPIO_PORT, VOUT_ENABLE_GPIO_PIN);
	}
	else
	{
		setPWMRate(0, pwm_table[current_output_stage]);
		Chip_GPIO_SetPinOutHigh(LPC_GPIO, VOUT_ENABLE_GPIO_PORT, VOUT_ENABLE_GPIO_PIN);
	}
}

void ButtonPressedTask(void)
{
	if(upcoming_system_state==US_DETERMINE_PCMODE_OR_COUNTDOWN_FOR_VOUT)	// It means we are either at welcome message or we are in the middle of checking new current_output_stage
	{
		System_State_Proc_timer_timeout = true;								// start to determine output mode immediately
	}
	// It means we are either at pc mode or counting down now
	else if((upcoming_system_state==US_PC_MODE_NO_VOLTAGE_OUTPUT)||(upcoming_system_state==US_PC_MODE_NO_VOLTAGE_OUTPUT_PAGE2)||(upcoming_system_state==US_OUTPUT_ENABLE))
	{
		if(++current_output_stage>=(sizeof(pwm_table)/sizeof(uint8_t)))
		{
			current_output_stage = 0;
		}
		upcoming_system_state = US_DETERMINE_PCMODE_OR_COUNTDOWN_FOR_VOUT;
		System_State_Proc_timer_timeout = true;			// Force to check output selection at next tick
	}
	// Skip button-press if it occurs just after checking current_output_stage & before starting countdown
	else if (upcoming_system_state==US_OUTPUT_REMINDER_COUNTDOWN_NOW)
	{
		// No action at the momment
	}
	else
	{
		// No action at the momment
	}

	// Always no matter which system state
	UpdateKitV2_LED_7_StartDisplayVoltage();
	EVENT_Button_pressed_debounced = false;
}

#define	CURRENT_HISTORY_DATA_SIZE	64
static RINGBUFF_T current_history;
static uint16_t current_history_data[CURRENT_HISTORY_DATA_SIZE];
static uint32_t	total_current_value;

void init_filtered_input_current(void)
{
	total_current_value = 0;
	RingBuffer_Init(&current_history, current_history_data, sizeof(uint16_t), CURRENT_HISTORY_DATA_SIZE);
	RingBuffer_Flush(&current_history);
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

#define	VOLTAGE_HISTORY_DATA_SIZE	64
static RINGBUFF_T voltage_history;
static uint16_t voltage_history_data[VOLTAGE_HISTORY_DATA_SIZE];
static uint32_t	total_voltage_value;

void init_filtered_input_voltage(void)
{
	total_voltage_value = 0;
	RingBuffer_Init(&voltage_history, voltage_history_data, sizeof(uint16_t), VOLTAGE_HISTORY_DATA_SIZE);
	RingBuffer_Flush(&voltage_history);
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

void lcd_module_display_enable_only_one_page(uint8_t enabled_page)
{
	uint8_t	temp_page = LCM_MAX_PAGE_NO;
	do
	{
		temp_page--;
		lcd_module_display_enable[temp_page] = (enabled_page==temp_page)?1:0;
	}
	while(temp_page>0);
	lcm_force_to_display_page(enabled_page);
}

#define	WELCOME_MESSAGE_DISPLAY_TIME_IN_MS		3000
#define OUTPUT_REMINDER_DISPLAY_TIME_IN_MS		6000

UPDATE_STATE System_State_Proc(UPDATE_STATE current_state)
{
	UPDATE_STATE return_next_state;

	switch(current_state)
	{
		case US_SYSTEM_STARTUP_WELCOME_MESSAGE:
			lcd_module_display_enable_only_one_page(LCM_WELCOME_PAGE);
			System_State_Proc_timer_in_ms = (WELCOME_MESSAGE_DISPLAY_TIME_IN_MS-1);						// Enter next state after 3 second
			return_next_state = US_DETERMINE_PCMODE_OR_COUNTDOWN_FOR_VOUT;
			break;
		case US_DETERMINE_PCMODE_OR_COUNTDOWN_FOR_VOUT:
			if(current_output_stage!=0)
			{
				System_State_Proc_timer_timeout = true;						// Enter next state at next tick
				return_next_state = US_OUTPUT_REMINDER_COUNTDOWN_NOW;
			}
			else
			{
				System_State_Proc_timer_timeout = true;						// Enter next state at next tick
				return_next_state = US_PC_MODE_NO_VOLTAGE_OUTPUT;
			}
			System_State_Proc_timer_in_ms = ~1; // this state always goes to next state so clear count-down timer here
			break;
		case US_PC_MODE_NO_VOLTAGE_OUTPUT:
			if(current_output_stage!=0)
			{
				System_State_Proc_timer_timeout = true;						// Enter next state at next tick of current_output_stage has been changed
				return_next_state = US_DETERMINE_PCMODE_OR_COUNTDOWN_FOR_VOUT;
			}
			else
			{
				lcd_module_display_enable_only_one_page(LCM_PC_MODE);
				return_next_state = US_PC_MODE_NO_VOLTAGE_OUTPUT_PAGE2;
				System_State_Proc_timer_in_ms = (WELCOME_MESSAGE_DISPLAY_TIME_IN_MS-1);						// Enter next state after 3 second
			}
			break;
		case US_PC_MODE_NO_VOLTAGE_OUTPUT_PAGE2:
			if(current_output_stage!=0)
			{
				System_State_Proc_timer_timeout = true;						// Enter next state at next tick of current_output_stage has been changed
				return_next_state = US_DETERMINE_PCMODE_OR_COUNTDOWN_FOR_VOUT;
			}
			else
			{
				lcd_module_display_enable_only_one_page(LCM_WELCOME_PAGE);
				System_State_Proc_timer_in_ms = (WELCOME_MESSAGE_DISPLAY_TIME_IN_MS-1);						// Enter next state after 3 second
				return_next_state = US_PC_MODE_NO_VOLTAGE_OUTPUT;
			}
			break;
		case US_OUTPUT_REMINDER_COUNTDOWN_NOW:
			if(current_output_stage!=0)
			{
				memcpy((void *)&lcd_module_display_content[LCM_REMINDER_BEFORE_OUTPUT][0][12], pwm_voltage_table[current_output_stage], 3);
				lcd_module_display_enable_only_one_page(LCM_REMINDER_BEFORE_OUTPUT);
				System_State_Proc_timer_in_ms = (OUTPUT_REMINDER_DISPLAY_TIME_IN_MS-1);				// Enter next state after > 5 sec
				return_next_state = US_OUTPUT_ENABLE;
			}
			else
			{
				System_State_Proc_timer_timeout = true;							// Enter next state at next tick if switch to no output
				return_next_state = US_DETERMINE_PCMODE_OR_COUNTDOWN_FOR_VOUT;	// Measure again
			}
			break;

		case US_OUTPUT_ENABLE:
			if(current_output_stage!=0)
			{
				lcd_module_display_enable_only_one_page(LCM_FW_UPGRADING_PAGE);
				PowerOutputSetting(current_output_stage);
				System_State_Proc_timer_timeout = true;						// Enter next state at next tick
				return_next_state = US_WAIT_FW_UPGRADE_OK_VER_STRING;
			}
			else
			{
				System_State_Proc_timer_timeout = true;							// Enter next state at next tick
				return_next_state = US_DETERMINE_PCMODE_OR_COUNTDOWN_FOR_VOUT;	// Measure again
			}
			break;

		case US_WAIT_FW_UPGRADE_OK_VER_STRING:
			lcd_module_display_enable_only_one_page(LCM_FW_OK_VER_PAGE);
			System_State_Proc_timer_in_ms = (2000-1);		// show this message for 2 second
			return_next_state = US_FW_UPGRADE_DONE;
			break;
		case US_FW_UPGRADE_DONE:
			lcd_module_display_enable_only_one_page(LCM_TV_IN_STANDBY_PAGE);
			System_State_Proc_timer_in_ms = (2000-1);		// show this message for 2 second
			return_next_state = US_TV_IN_STANDBY;
			break;
		case US_TV_IN_STANDBY:
			lcd_module_display_enable_only_one_page(LCM_ENTER_ISP_PAGE);
			System_State_Proc_timer_in_ms = (2000-1);		// show this message for 2 second
			return_next_state = US_SYSTEM_STARTUP_WELCOME_MESSAGE;
			break;
		default:
			break;
	}

	return return_next_state;
}

bool UART_input_processor(uint8_t key)
{
	uint8_t	temp_ok_cnt;
	bool	bRet_any_event_raised = false;

	/* Wrap value back around */
	//UART0_PutChar((char)key);

	// To identify 10x OK
	temp_ok_cnt=locate_OK_pattern_process(key);
	if(temp_ok_cnt==DEFAULT_OK_THRESHOLD)
	{
		EVENT_OK_string_confirmed = true;
		bRet_any_event_raised = true;
	}

	// To identify @POWERON
	locate_POWERON_pattern_process(key);
	if(Get_POWERON_pattern()==true)
	{
		//OutputString_with_newline("POWER_ON_DETECTED");
		memcpy((void *)&lcd_module_display_content[LCM_DEV_OK_DETECT_PAGE][0][0], "POWERON detected", LCM_DISPLAY_COL);
		lcm_force_to_display_page(LCM_DEV_OK_DETECT_PAGE);
		Clear_POWERON_pattern();
		EVENT_Version_string_confirmed = true;
		bRet_any_event_raised = true;
	}

	// To identify @VER
	locate_VER_pattern_process(key);
	if(Found_VER_string()==true)
	{
		EVENT_POWERON_string_confirmed = true;
		bRet_any_event_raised = true;
	}

	return bRet_any_event_raised;
}

