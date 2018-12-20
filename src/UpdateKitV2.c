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

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/

/*****************************************************************************
 * Private functions
 ****************************************************************************/

/*****************************************************************************
 * Public functions
 ****************************************************************************/

void lcm_content_init(void)
{
//    strcpy((void *)&lcd_module_display_content[0][0][0], "TPV UpdateKit V2" __TIME__);
    strcpy((void *)&lcd_module_display_content[0][0][0], __DATE__);    // xxx xx xxxx
    strcpy((void *)&lcd_module_display_content[0][0][7], __TIME__ " ");    // tt:tt:tt
    strcpy((void *)&lcd_module_display_content[0][1][0], "Elapse: 0000 Sec");
    strcpy((void *)&lcd_module_display_content[1][0][0], "OUT: 0.00V 0.00A");
    strcpy((void *)&lcd_module_display_content[1][1][0], "PWM Duty:100    ");
    strcpy((void *)&lcd_module_display_content[2][0][0], "Ver:            ");
    strcpy((void *)&lcd_module_display_content[2][1][0], "detecting...    ");
    strcpy((void *)&lcd_module_display_content[3][0][0], "PWR detecting...");
    strcpy((void *)&lcd_module_display_content[3][1][0], "OK detecting... ");
    //													  1234567890123456
    memset((void *)lcd_module_display_enable, 0x01, 4);
    //lcd_module_display_enable[3] = 0x00;
     //memset((void *)lcd_module_display_enable, 0x00, 4);
     //lcd_module_display_enable[1]=1;
}

void lcm_content_init_new(void)
{
	char const	*date = __DATE__,   							// mmm dd yyyy
				*time = __TIME__,								// xx:xx:xx
				*welcome_message_line1 =  "TPV UpdateKit V2";

	uint8_t		welcome_message_line2[17];

	// Prepare firmware version for welcome page
	memcpy((void *)&welcome_message_line2[0], (date), 3);						// mmm
	memcpy((void *)&welcome_message_line2[3], (date+4), 3);						// mmmdd
	if(welcome_message_line2[3]==' ') {welcome_message_line2[3]='0';}
	memcpy((void *)&welcome_message_line2[5], (date+7), 4);						// mmmddyyyy
	welcome_message_line2[9] = '@';
	memcpy((void *)&welcome_message_line2[10], time, 2);							// mmmddyyyy@hh
	memcpy((void *)&welcome_message_line2[12], (time+3), 2);						// mmmddyyyy@hhmm
	memcpy((void *)&welcome_message_line2[14], (time+6), 2);						// mmmddyyyy@hhmmss

	// Welcome page														 1234567890123456
	memcpy((void *)&lcd_module_display_content[LCM_WELCOME_PAGE][0][0], welcome_message_line1, LCM_DISPLAY_COL);
	memcpy((void *)&lcd_module_display_content[LCM_WELCOME_PAGE][1][0], welcome_message_line2, LCM_DISPLAY_COL);

	// PC Mode page		     										1234567890123456
	memcpy((void *)&lcd_module_display_content[LCM_PC_MODE][0][0], "PC Mode: Press  ", LCM_DISPLAY_COL);
	memcpy((void *)&lcd_module_display_content[LCM_PC_MODE][1][0], "button to change", LCM_DISPLAY_COL);

	// Input Measurement page							  					   1234567890123456
	memcpy((void *)&lcd_module_display_content[LCM_INPUT_MEASURE_PAGE][0][0], "Input:         V", LCM_DISPLAY_COL);
	memcpy((void *)&lcd_module_display_content[LCM_INPUT_MEASURE_PAGE][1][0], "Current:       A", LCM_DISPLAY_COL);

	// Show software version page						  				 1234567890123456
	memcpy((void *)&lcd_module_display_content[LCM_VERSION_PAGE][0][0], "Firmware updated", LCM_DISPLAY_COL);
	memset((void *)&lcd_module_display_content[LCM_VERSION_PAGE][1][0], ' ', LCM_DISPLAY_COL);

	// TV in standby page		     										   1234567890123456
	memcpy((void *)&lcd_module_display_content[LCM_TV_IN_STANDBY_PAGE][0][0], "TV is in Standby", LCM_DISPLAY_COL);
	memcpy((void *)&lcd_module_display_content[LCM_TV_IN_STANDBY_PAGE][1][0], "Turn-on your TV ", LCM_DISPLAY_COL);

	// TV is entering ISP mode page		     							   1234567890123456
	memcpy((void *)&lcd_module_display_content[LCM_ENTER_ISP_PAGE][0][0], "Enter ISP mode  ", LCM_DISPLAY_COL);
	memcpy((void *)&lcd_module_display_content[LCM_ENTER_ISP_PAGE][1][0], "Off-On after ISP", LCM_DISPLAY_COL);

	memset((void *)lcd_module_display_enable, 0x00, LCM_MAX_PAGE_NO);	// Initial only - later sw determine which page is to be displayed
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
		case LCM_INPUT_MEASURE_PAGE:
			break;
		case LCM_VERSION_PAGE:
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
uint16_t	voltage = 0;		//  0.00v ~ 9.99v --> 0-999
uint16_t	current = 0;		// .000A ~ .999A --> 0-999
bool		LED_7_SEG_showing_current = false;

void SetDisplayVoltage(uint16_t voltage_new)
{
	voltage = voltage_new;
}

void SetDisplayCurrent(uint16_t current_new)
{
	current = current_new;
}

uint16_t GetDisplayVoltage(void)
{
	return voltage;
}

uint16_t GetDisplayCurrent(void)
{
	return current;
}

void UpdateKitV2_LED_7_ToggleDisplayVoltageCurrent(void)
{
	LED_7_SEG_showing_current = !LED_7_SEG_showing_current;
}

void UpdateKitV2_UpdateDisplayValueForADC_Task(void)
{
	char 	 temp_voltage_str[5+1], temp_current_str[5+1], final_voltage_str[5+1], final_current_str[5+1];		// For storing 0x0 at the end of string by +1
	int 	 temp_voltage_str_len, temp_current_str_len;
	//uint16_t temp_value;
	uint8_t	 dp_point;

	// Update LED-Y for current >= 10ma or not
	if(current>=DEFAULT_INPUT_CURRENT_THRESHOLD)
	{
		LED_Y_setting(5);  // 500ms as half-period (toggle period)
	}
	else
	{
		LED_Y_setting(0);
	}

	// Generate string
	// showing voltage // 0.00v ~ 9.99v
	{
//		temp_value = voltage;			// 0.001V as unit
//		if(temp_value>9999)
//		{
//			temp_value = 9999;
//		}
		temp_voltage_str_len = itoa_10(voltage, temp_voltage_str);
	}
	// showing current 0.00A~0.99A -- but current is 0.0001A as unit
	{
//		temp_value = current;
//		if(temp_value>999)		// protection //  showing // 0.00A~0.99A	// 0.001A as unit
//		{
//			temp_value = 999;
//		}
		temp_current_str_len = itoa_10(current, temp_current_str);
	}

	//
	// Update LCD module display
	//
	// Voltage
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
	memcpy((void *)&lcd_module_display_content[1][0][5], final_voltage_str, 5);

	// current
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
	memcpy((void *)&lcd_module_display_content[1][0][11], final_current_str, 5);

	//
	// Update LED 7-segment
	//
	if(LED_7_SEG_showing_current!=false)		// showing voltage // 0.00v ~ 9.99v
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
	if(++current_output_stage>=(sizeof(pwm_table)/sizeof(uint8_t)))
	{
		current_output_stage = 0;

	}

	PowerOutputSetting(current_output_stage);
	// temp for debug purpose
	{
					char temp_str[LCM_DISPLAY_COL+1];
					int  temp_str_len;
					temp_str_len = itoa_10(pwm_table[current_output_stage], temp_str);
					memcpy((void *)&lcd_module_display_content[1][1][9], temp_str, temp_str_len);
					memset((void *)&lcd_module_display_content[1][1][9+temp_str_len], ' ', LCM_DISPLAY_COL-9-temp_str_len);
					lcm_force_to_display_page(1);
	}
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
}

UPDATE_STATE System_State_Proc(UPDATE_STATE current_state)
{
	UPDATE_STATE return_next_state;

	switch(current_state)
	{
		case US_SYSTEM_STARTUP:
			lcd_module_display_enable_only_one_page(LCM_WELCOME_PAGE);
			SetDisplayCurrent(LCM_WELCOME_PAGE);
			System_State_Proc_timer_in_ms = (2000-1);		// show this message for 2 second
			return_next_state = US_WELCOME_MESSAGE;
			break;
		case US_WELCOME_MESSAGE:
			lcd_module_display_enable_only_one_page(LCM_PC_MODE);
			SetDisplayCurrent(LCM_PC_MODE);
			System_State_Proc_timer_in_ms = (2000-1);		// show this message for 2 second
			return_next_state = US_PC_MODE_NO_VOLTAGE_OUTPUT;
			break;
		case US_PC_MODE_NO_VOLTAGE_OUTPUT:
			lcd_module_display_enable_only_one_page(LCM_REMINDER_BEFORE_OUTPUT);
			SetDisplayCurrent(LCM_REMINDER_BEFORE_OUTPUT);
			System_State_Proc_timer_in_ms = (2000-1);		// show this message for 2 second
			return_next_state = US_REMINDER_BEFORE_VOLTAGE_OUTPUT;
			break;
		case US_REMINDER_BEFORE_VOLTAGE_OUTPUT:
			lcd_module_display_enable_only_one_page(LCM_INPUT_MEASURE_PAGE);
			SetDisplayCurrent(LCM_INPUT_MEASURE_PAGE);
			System_State_Proc_timer_in_ms = (2000-1);		// show this message for 2 second
			return_next_state = US_WAITING_FW_UPGRADE;
			break;
		case US_WAITING_FW_UPGRADE:
			lcd_module_display_enable_only_one_page(LCM_VERSION_PAGE);
			SetDisplayCurrent(LCM_VERSION_PAGE);
			System_State_Proc_timer_in_ms = (2000-1);		// show this message for 2 second
			return_next_state = US_FW_UPGRADE_DONE;
			break;
		case US_FW_UPGRADE_DONE:
			lcd_module_display_enable_only_one_page(LCM_TV_IN_STANDBY_PAGE);
			SetDisplayCurrent(LCM_TV_IN_STANDBY_PAGE);
			System_State_Proc_timer_in_ms = (2000-1);		// show this message for 2 second
			return_next_state = US_TV_IN_STANDBY;
			break;
		case US_TV_IN_STANDBY:
			lcd_module_display_enable_only_one_page(LCM_ENTER_ISP_PAGE);
			SetDisplayCurrent(LCM_ENTER_ISP_PAGE);
			System_State_Proc_timer_in_ms = (2000-1);		// show this message for 2 second
			return_next_state = US_SYSTEM_STARTUP;
			break;
		default:
			break;
	}

	return return_next_state;
}
