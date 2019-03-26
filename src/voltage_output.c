/*
 * voltage_output.c
 *
 *  Created on: 2019年3月14日
 *      Author: Jeremy.Hsiao
 */

#include "chip.h"
#include "board.h"
#include "string.h"
#include "event.h"
#include "sw_timer.h"
#include "lcd_module.h"
#include "uart_0_rb.h"
#include "pwm.h"
#include "gpio.h"
#include "build_defs.h"
#include "fw_version.h"
#include "UpdateKitV2.h"
#include "user_opt.h"
#include "voltage_output.h" // For voltage output branch
#include "cmd_interpreter.h" // For voltage output branch

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/
uint8_t				current_duty_cycle_selection; // 0 is pwm-off; 1-101 is duty-cycle 0-100
/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/

/*****************************************************************************
 * Private functions
 ****************************************************************************/

/*****************************************************************************
 * Public functions
 ****************************************************************************/
void Init_Value_From_EEPROM_for_voltage_output(void)
{
 	Load_PWM_Selection(&current_duty_cycle_selection);
}

void OutputVoltageCurrentViaUART_Task(void)
{
    uint8_t temp = 16;
    uint8_t *ptr = lcd_module_display_content[LCM_FW_UPGRADING_PAGE][1];

    do
    {
    	UART0_PutChar(*ptr++);
    }
    while(--temp>0);
    UART0_PutChar('\n');
    UART0_PutChar('\r');
}

//	LCM_PWM_WELCOME,
//	LCM_PWM_OUT_ON,
//	LCM_PWM_OUT_OFF,
//	LCM_PWM_USER_CTRL,
//	LCM_MAX_PAGE_NO
void lcm_content_init_for_voltage_output(void)
{
	const uint8_t				voltage_output_welcome_message_line2[] =
	{   'V', 'e', 'r', ':', FW_MAJOR, FW_MIDDLE, FW_MINOR, '_', // "Ver:x.x_" - total 8 chars
	   BUILD_MONTH_CH0, BUILD_MONTH_CH1, BUILD_DAY_CH0, BUILD_DAY_CH1, BUILD_HOUR_CH0, BUILD_HOUR_CH1,  BUILD_MIN_CH0, BUILD_MIN_CH1, // 8 chars
		'\0'};

	// Prepare firmware version for welcome page

	// PWM Welcome page	                                                1234567890123456
	memcpy((void *)&lcd_module_display_content[LCM_PWM_WELCOME][0][0], " Voltage Output ", LCM_DISPLAY_COL);
	memcpy((void *)&lcd_module_display_content[LCM_PWM_WELCOME][1][0], voltage_output_welcome_message_line2, LCM_DISPLAY_COL);

	// PWM Output ON page		     								   1234567890123456
	memcpy((void *)&lcd_module_display_content[LCM_PWM_OUT_ON][0][0], "PWM Duty is    %", LCM_DISPLAY_COL);
	memcpy((void *)&lcd_module_display_content[LCM_PWM_OUT_ON][1][0], "OUT: 0.00V 0.00A", LCM_DISPLAY_COL);

	// PWM Output OFF page		     							        1234567890123456
	memcpy((void *)&lcd_module_display_content[LCM_PWM_OUT_OFF][0][0], "PWM is disabled ", LCM_DISPLAY_COL);
	memcpy((void *)&lcd_module_display_content[LCM_PWM_OUT_OFF][1][0], "OUT: 0.00V 0.00A", LCM_DISPLAY_COL);

	// PWM user control mdoe page										  1234567890123456
	memcpy((void *)&lcd_module_display_content[LCM_PWM_USER_CTRL][0][0], " User Ctrl Mode ", LCM_DISPLAY_COL);
	memcpy((void *)&lcd_module_display_content[LCM_PWM_USER_CTRL][1][0], "OUT: 0.00V 0.00A", LCM_DISPLAY_COL);
}

static inline void Change_PWM_Selection(bool decrease_dir)
{
	if(!decrease_dir)
	{
		if(++current_duty_cycle_selection>(MAX_DUTY_SELECTION_VALUE+DUTY_SELECTION_OFFSET_VALUE))
		{
			current_duty_cycle_selection=PWM_OFF_DUTY_SELECTION_VALUE;
		}
	}
	else
	{
		if(current_duty_cycle_selection--==PWM_OFF_DUTY_SELECTION_VALUE)
		{
			current_duty_cycle_selection=(MAX_DUTY_SELECTION_VALUE+DUTY_SELECTION_OFFSET_VALUE);
		}
	}
}

void PWMOutputSetting(uint8_t current_pwm_sel)
{
	if(current_pwm_sel==PWM_OFF_DUTY_SELECTION_VALUE)
	{
		pwm_duty = MAX_DUTY_SELECTION_VALUE;
		Chip_GPIO_SetPinOutLow(LPC_GPIO, VOUT_ENABLE_GPIO_PORT, VOUT_ENABLE_GPIO_PIN);
		setPWMRate(0, pwm_duty);
	}
	else
	{
		pwm_duty = current_pwm_sel - DUTY_SELECTION_OFFSET_VALUE;
		if(pwm_duty>MAX_DUTY_SELECTION_VALUE)
		{
			pwm_duty = MAX_DUTY_SELECTION_VALUE;
		}
		Chip_GPIO_SetPinOutHigh(LPC_GPIO, VOUT_ENABLE_GPIO_PORT, VOUT_ENABLE_GPIO_PIN);
		setPWMRate(0, pwm_duty);
	}
}

// To-be-checked
bool Event_Proc_State_Independent_for_voltage_output(void)
{
	bool	bRet = false;

	return bRet;
}

//	// For voltage output branch
//	US_PWM_WELCOME,
//	US_PWM_CHECK_SEL,
//	US_PWM_OUT_ON,
//	US_PWM_OUT_OFF,
//	US_PWM_USER_CTRL,
//	// For voltage output branch	-- END
UPDATE_STATE Event_Proc_by_System_State_for_voltage_output(UPDATE_STATE current_state)
{
	UPDATE_STATE return_next_state = current_state;

	// Apply to specific state
	switch(current_state)
	{
		case US_PWM_CHECK_SEL:
			if(EVENT_UART_CMD_Received)
			{
				EVENT_UART_CMD_Received = false;
				if(CheckIfUserCtrlModeCommand(command_string))
				{
					EVENT_Enter_User_Ctrl_Mode = true;
				}
			}

			if(EVENT_Enter_User_Ctrl_Mode)
			{
				EVENT_Enter_User_Ctrl_Mode = false;
				return_next_state = US_PWM_USER_CTRL;
			}
			else if(EVENT_Button_pressed_debounced)
			{
				EVENT_Button_pressed_debounced = false;
			}
			break;
		case US_PWM_USER_CTRL:
			if(EVENT_Button_pressed_debounced)
			{
				EVENT_Button_pressed_debounced = false;
			}
			if(EVENT_UART_CMD_Received)
			{
				char 	*ret_str;

				EVENT_UART_CMD_Received = false;
				if(CommandInterpreter(command_string, &received_cmd_packet))
				{
					CommandExecution(received_cmd_packet,&ret_str);
				}
				else
				{
					ret_str = "Command Error";
				}
				if(CheckEchoEnableStatus())
					OutputString_with_newline(ret_str);					// Echo incoming command (if echo_enabled)
				OutputHexValue_with_newline(received_cmd_packet);		// debug purpose and can be removed later
			}
			if(EVENT_Leave_User_Ctrl_Mode)
			{
				EVENT_Leave_User_Ctrl_Mode = false;
				SetUserCtrlModeFlag(false);
				return_next_state = US_PWM_CHECK_SEL;
			}
			break;
		case US_PWM_WELCOME:
			if(EVENT_Button_pressed_debounced)
			{
				EVENT_Button_pressed_debounced = false;
				return_next_state = US_PWM_CHECK_SEL;
			}
			break;
		case US_PWM_OUT_ON:
		case US_PWM_OUT_OFF:
			if(EVENT_UART_CMD_Received)
			{
				EVENT_UART_CMD_Received = false;
				if(CheckIfUserCtrlModeCommand(command_string))
				{
					EVENT_Enter_User_Ctrl_Mode = true;
				}
			}

			if(EVENT_Enter_User_Ctrl_Mode)
			{
				EVENT_Enter_User_Ctrl_Mode = false;
				return_next_state = US_PWM_USER_CTRL;
			}
			else if(EVENT_Button_pressed_debounced)
			{
				EVENT_Button_pressed_debounced = false;
				Change_PWM_Selection(EVENT_2nd_key_pressed_debounced);		// 2nd key is for decrease -- so the input parameter (decrease_dir) is the 2nd key event
				return_next_state = US_PWM_CHECK_SEL;
			}
			break;
		default:
			break;
	}
	return return_next_state;
}

UPDATE_STATE System_State_Running_Proc_for_voltage_output(UPDATE_STATE current_state)
{
	UPDATE_STATE return_next_state = current_state;
	switch(current_state)
	{
		case US_SYSTEM_BOOTUP_STATE:
			return_next_state = US_PWM_WELCOME;
			break;
		case US_PWM_WELCOME:
		case US_PWM_CHECK_SEL:
		case US_PWM_USER_CTRL:
			break;
		case US_PWM_OUT_ON:
		case US_PWM_OUT_OFF:
			// Save User Selection after 5 seconds
			if(Check_if_different_from_last_PWM_ReadWrite(current_duty_cycle_selection))
			{
				if(Read_SW_TIMER_Value(SYSTEM_STATE_PROC_TIMER)>=USER_PWM_EEPROM_STORE_DELAY)
				{
					Save_PWM_Selection(current_duty_cycle_selection);
				}
			}
			break;
		default:
			break;
	}
	return return_next_state;
}

UPDATE_STATE System_State_End_Proc_for_voltage_output(UPDATE_STATE current_state)
{
	UPDATE_STATE return_next_state = current_state;
	switch(current_state)
	{
		case US_SYSTEM_BOOTUP_STATE:
			return_next_state = US_PWM_WELCOME;
			break;
		case US_PWM_WELCOME:
			return_next_state = US_PWM_CHECK_SEL;
			break;
		case US_PWM_OUT_ON:
		case US_PWM_OUT_OFF:
			break;
		case US_PWM_CHECK_SEL:
			if(current_duty_cycle_selection==PWM_OFF_DUTY_SELECTION_VALUE)
				return_next_state = US_PWM_OUT_OFF;
			else
				return_next_state = US_PWM_OUT_ON;
			break;
		case US_PWM_USER_CTRL:
			return_next_state = US_PWM_CHECK_SEL;
			break;
		default:
			break;
	}
	return return_next_state;
}

UPDATE_STATE System_State_Begin_Proc_for_voltage_output(UPDATE_STATE current_state)
{
	UPDATE_STATE return_next_state = current_state;
	uint32_t	pwm_duty = 	current_duty_cycle_selection-DUTY_SELECTION_OFFSET_VALUE;

	switch(current_state)
	{
		case US_SYSTEM_BOOTUP_STATE:
			break;
		case US_PWM_WELCOME:
			Countdown_Once(SYSTEM_STATE_PROC_TIMER,(PWM_WELCOME_MESSAGE_IN_MS),TIMER_MS);
			lcd_module_display_enable_only_one_page(LCM_PWM_WELCOME);
			EVENT_Button_pressed_debounced = false;
			break;
		case US_PWM_CHECK_SEL:
			Raise_SW_TIMER_Reload_Flag(SYSTEM_STATE_PROC_TIMER);		// enter next state without timer down to 0
			break;
		case US_PWM_OUT_ON:
			itoa_10_fixed_position(pwm_duty, (char*)&lcd_module_display_content[LCM_PWM_OUT_ON][0][12], 3);
			lcd_module_display_enable_only_one_page(LCM_PWM_OUT_ON);
			PWMOutputSetting(current_duty_cycle_selection);
			Start_SW_Timer(SYSTEM_STATE_PROC_TIMER,0,~1,TIMER_S, true, false);		// endless timer 0->max repeating countdown from max
			break;
		case US_PWM_OUT_OFF:
			lcd_module_display_enable_only_one_page(LCM_PWM_OUT_OFF);
			PWMOutputSetting(current_duty_cycle_selection);
			Start_SW_Timer(SYSTEM_STATE_PROC_TIMER,0,~1,TIMER_S, true, false);		// endless timer 0->max repeating countdown from max
			break;
		case US_PWM_USER_CTRL:
			lcd_module_display_enable_only_one_page(LCM_PWM_USER_CTRL);
			Start_SW_Timer(SYSTEM_STATE_PROC_TIMER,0,~1,TIMER_S, true, false);		// endless timer 0->max repeating countdown from max
			break;
		default:
			// fall-back code
			break;
	}

	return return_next_state;
}

