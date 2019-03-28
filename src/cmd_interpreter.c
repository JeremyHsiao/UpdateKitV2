/*
 * cmd_interpreter.c
 *
 *  Created on: 2018年11月27日
 *      Author: Jeremy.Hsiao
 */
#include "chip.h"
#include "string.h"
#include "stdlib.h"
#include "ctype.h"
#include "UpdateKitV2.h"
#include "gpio.h"
#include "uart_0_rb.h"
#include "event.h"
#include "build_defs.h"
#include "fw_version.h"
#include "voltage_output.h"
#include "cmd_interpreter.h"

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/
char 			*ptr_str;
bool 			EchoEnabled;
char 			serial_gets_return_string[MAX_SERIAL_GETS_LEN+1];	// Extra one is for '\0'
char 			command_return_string[17];

// internal structure for execution: 24-bit value + 6-bit object + 2-bit cmd
// get obj
// set obj value
// reserved_1
// reserved_2

#define		CmdCodeBitPos		(0)
#define     CmdCodeBitLen		(2)
#define		CmdObjBitPos		(CmdCodeBitPos+CmdCodeBitLen)
#define		CmdOjbBitLen		(6)
#define		CmdValueBitPos		(CmdObjBitPos+CmdOjbBitLen)
#define		CmdValueBitLen		(CmdPacketLen-CmdOjbBitLen-CmdCodeBitLen)
#define		CmdCodeBitMask		( ( ( ((CmdExecutionPacket)1UL) << (CmdCodeBitLen+1) )  - 1)  << CmdCodeBitPos )
#define     CmdOjbBitMask		( ( ( ((CmdExecutionPacket)1UL) << (CmdOjbBitLen+1) )   - 1)  << CmdObjBitPos )
#define		CmdValueBitMask		( ( ( ((CmdExecutionPacket)1UL) << (CmdValueBitLen+1) ) - 1 ) << CmdValueBitPos )

enum
{
	CMD_CODE_NONE_COMMAND = 0,
	CMD_CODE_GET,
	CMD_CODE_SET,
//	CMD_CODE_RESERVED_2,
	CMD_CODE_MAX_NO
};

static const char *command_code_list[CMD_CODE_MAX_NO-1] =
{
		// not checking non-command --> so the enum number is +1 larger than the sequence of command_code_list
		"get",
		"set",
//		"reserved_2",
};

static const char enter_user_ctrl_mode_str[] = "x&Vht&GD";

enum
{
	CMD_OBJECT_NONE_OBJECT = 0,
	CMD_OBJECT_USERMODE,
	CMD_OBJECT_PWM_DUTY_PERCENTAGE,
	CMD_OBJECT_PWM_OUTPUT,
	CMD_OBJECT_PWM_USE_TABLE,
	CMD_OBJECT_ECHO,
	CMD_OBJECT_FW_VER,
	CMD_OBJECT_PWM_DUTY_VALUE,
	CMD_OBJECT_PWM_DUTY_RANGE,
	CMD_OBJECT_PWM_FREQ_VALUE,
	CMD_OBJECT_PWM_FREQ_RANGE,
	CMD_OBJECT_MAX_NO,
};

static const char *command_object_list[CMD_OBJECT_MAX_NO-1] =
{
	// not checking non-object--> so the enum number is +1 larger than the sequence of command_object_list
	"user_mode",
	"pwm_percent",
	"pwm_output",
	"pwm_use_table",
	"echo",
	"fw_ver",
	"pwm_duty_value",
	"pwm_duty_range",
	"pwm_freq_value",
	"pwm_freq_range"
};

#define CMD_DEFINE_PACK_CMD(cmd)				( CmdCodeBitMask  & (((CmdExecutionPacket)cmd)    << CmdCodeBitPos ) )
#define CMD_DEFINE_GET							CMD_DEFINE_PACK_CMD(CMD_CODE_GET)
#define CMD_DEFINE_SET							CMD_DEFINE_PACK_CMD(CMD_CODE_SET)

#define CMD_DEFINE_PACK_OBJ(obj)				( CmdOjbBitMask   & (((CmdExecutionPacket)obj)    << CmdObjBitPos  ) )
#define CMD_GET_OBJECT_VALUE(OBJECT)			( CMD_DEFINE_GET | CMD_DEFINE_PACK_OBJ(OBJECT) )
#define CMD_SET_OBJECT_VALUE(OBJECT)	    	( CMD_DEFINE_SET | CMD_DEFINE_PACK_OBJ(OBJECT) )

#define CMD_DEFINE_PACK_PARAMETER(param)		( CmdValueBitMask & (((CmdExecutionPacket)param) << CmdValueBitPos) )

#define CMD_EXTRACT_CODE_WITH_OBJECT(PACKET)	(PACKET&(~CmdValueBitMask))
#define CMD_EXTRACT_PARAMETER(PACKET)			((PACKET&CmdValueBitMask)>>CmdValueBitPos)

typedef enum {
	DEFAULT_NON_CMD			= 0,
	//  No GET
	SET_USER_MODE 			= CMD_SET_OBJECT_VALUE(CMD_OBJECT_USERMODE),			// Enter/Leave user control mode
	GET_PWM_DUTY_VALUE		= CMD_GET_OBJECT_VALUE(CMD_OBJECT_PWM_DUTY_VALUE),		// get duty cycle value
	SET_PWM_DUTY_VALUE		= CMD_SET_OBJECT_VALUE(CMD_OBJECT_PWM_DUTY_VALUE),		// set duty cycle value
	GET_PWM_DUTY_PERCENTAGE	= CMD_GET_OBJECT_VALUE(CMD_OBJECT_PWM_DUTY_PERCENTAGE),	// get duty cycle value
	SET_PWM_DUTY_PERCENTAGE	= CMD_SET_OBJECT_VALUE(CMD_OBJECT_PWM_DUTY_PERCENTAGE),	// set duty cycle value
	GET_PWM_DUTY_RANGE		= CMD_GET_OBJECT_VALUE(CMD_OBJECT_PWM_DUTY_RANGE),		// get duty cycle range
	//  No SET
	GET_PWM_FREQ 			= CMD_GET_OBJECT_VALUE(CMD_OBJECT_PWM_FREQ_VALUE),		// get duty cycle value
	SET_PWM_FREQ 			= CMD_SET_OBJECT_VALUE(CMD_OBJECT_PWM_FREQ_VALUE),		// set duty cycle value
	GET_PWM_FREQ_RANGE		= CMD_GET_OBJECT_VALUE(CMD_OBJECT_PWM_FREQ_RANGE),		// get frequency range
	//  No SET
	GET_PWM_OUTPUT 			= CMD_GET_OBJECT_VALUE(CMD_OBJECT_PWM_OUTPUT),			// get if pwm output is enabled
	SET_PWM_OUTPUT 			= CMD_SET_OBJECT_VALUE(CMD_OBJECT_PWM_OUTPUT),			// set pwm output enable
	//  No GET
	SET_PWM_USE_TABLE		= CMD_SET_OBJECT_VALUE(CMD_OBJECT_PWM_USE_TABLE),		// use table value in code as output value
	GET_FW_VERSION			= CMD_GET_OBJECT_VALUE(CMD_OBJECT_FW_VER),				// get FW version
	//  No SET
	GET_ECHO 				= CMD_GET_OBJECT_VALUE(CMD_OBJECT_ECHO),				// get echo status
	SET_ECHO 				= CMD_SET_OBJECT_VALUE(CMD_OBJECT_ECHO),				// set echo status
} CMD_LIST;

char *error_parameter  = "Parameter error.";
char *error_command    = "Command error.";
char *error_developing = "Under development";
char *message_ok       = "OK";
char *message_On       = "ON";
char *message_Off      = "OFF";

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/
CmdExecutionPacket		received_cmd_packet;
char					*command_string;

/*****************************************************************************
 * Private functions
 ****************************************************************************/
char *trimwhitespace(char *str)
{
  char *end;

  // Trim leading space
  while(isspace((unsigned char)*str)) str++;

  if(*str == 0)  // All spaces?
    return str;

  // Trim trailing space
  end = str + strlen(str) - 1;
  while(end > str && isspace((unsigned char)*end)) end--;

  // Write new null terminator character
  *++end = '\0';

  return str;
}


/*****************************************************************************
 * Public functions
 ****************************************************************************/
void init_cmd_interpreter(void)
{
	*serial_gets_return_string = '\0';
	ptr_str = serial_gets_return_string;
	EchoEnabled = true; //
	received_cmd_packet = 0;
	command_string = (char *)NULL;
}

void EchoEnable(bool enabled)
{
	EchoEnabled = enabled;
}

bool CheckEchoEnableStatus(void)
{
	return EchoEnabled;
}

char *serial_gets(char input_ch)
{
	char *ret_ptr;

	switch (input_ch)
	{
		case '\r':
		case '\n':
			// Append a '\0' to the end of return string'; then reset ptr_str for next line of command.
			*ptr_str = '\0';
			ptr_str = serial_gets_return_string;
			ret_ptr = serial_gets_return_string;
			break;
		case '\b':
			// remove previous char (if any)
			if(ptr_str>(serial_gets_return_string))
			{
				ptr_str--;
			}
			ret_ptr = (char *)(NULL);
			break;
		case 3:		//ctrl-c
		case 4:		//ctrl-d
		case 26:	//ctrl-z
			// reset ptr_str for next line of command.
			ptr_str = serial_gets_return_string;
			ret_ptr = (char *)(NULL);
			break;
		default:
			// Fill input string buffer until last 1 slot left (for storing end-of-string '\0')
			if(ptr_str<(serial_gets_return_string+MAX_SERIAL_GETS_LEN))
			{
				*ptr_str++ = input_ch;
			}
			ret_ptr = (char *)(NULL);
			break;
	}

	return ret_ptr;
}

bool CheckIfUserCtrlModeCommand(char *input_str)
{
	// OutputString_with_newline(input_str);				 // debug purpose
	// Must be the exact string to enter user control mode
	if (strcmp(trimwhitespace(input_str),enter_user_ctrl_mode_str)==0)
		return true;
	else
		return false;
}

bool CommandInterpreter(char *input_str, CmdExecutionPacket *ptr_packet)
{
	char 				*token = strtok(trimwhitespace(input_str), " ");

	*ptr_packet = 0;			// clear for further OR after command/object/parameter are decoded

	// 1st command
	if(token==NULL)
	{
		return false;
	}
	else
	{
		int	index = CMD_CODE_MAX_NO-1;
		while(index-->0)
		{
			if (strcmp(token,command_code_list[index])==0)
			{
				*ptr_packet = CMD_DEFINE_PACK_CMD(index+1);
				// OutputString_with_newline(token);	// debug purpose
				break;
			}
		}
	}

	// 2nd object
	token = strtok(NULL, " ");
	if(token==NULL)
	{
		return false;
	}
	else
	{
		int	index = CMD_OBJECT_MAX_NO-1;
		while(index-->0)
		{
			if (strcmp(token,command_object_list[index])==0)
			{
				*ptr_packet |= CMD_DEFINE_PACK_OBJ(index+1);
				// OutputString_with_newline(token);	// debug purpose
				break;
			}
		}
	}

	// 3rd parameter
	token = strtok(NULL, " ");
	if(token!=NULL)
	{
		int val;
		val = atoi(token);
		// OutputString_with_newline(token);	// debug purpose
		// OutputHexValue_with_newline(val);	// debug purpose
		*ptr_packet |= CMD_DEFINE_PACK_PARAMETER(val);
	}

	return true;
}

//
// test commands:
//
// x&Vht&GD
// set user_mode 1
// set pwm_percent 100
// set pwm_percent 50
// set pwm_percent 0
// set pwm_output 0
// set pwm_output 1
// get pwm_output
// set pwm_use_table 0
// set pwm_use_table 9
// set pwm_use_table 5
// set user_mode 0
// x&Vht&GD
// get fw_ver

bool CommandExecution(CmdExecutionPacket cmd_packet, char **return_string_ptr)
{
	uint32_t	param = CMD_EXTRACT_PARAMETER(cmd_packet);
	bool		ret_value = false;

	switch(CMD_EXTRACT_CODE_WITH_OBJECT(cmd_packet))
	{
		case SET_USER_MODE:
			if(param!=0)
			{
				EVENT_Enter_User_Ctrl_Mode = true;
			}
			else
			{
				EVENT_Leave_User_Ctrl_Mode = true;
			}
			*return_string_ptr = message_ok;
			ret_value = true;
			break;
		case SET_PWM_DUTY_PERCENTAGE:
			if(param<=MAX_DUTY_SELECTION_VALUE)
			{
				uint8_t	pwm_sel_value;
				pwm_sel_value = Set_PWM_Selection_by_Duty_Cycle(param);	// input param is duty cycle and return value is pwm_sel_value (duty cycle with some offset)
				PWMOutputSetting(pwm_sel_value);
				*return_string_ptr = message_ok;
				ret_value = true;
			}
			else
				*return_string_ptr = error_parameter;
			break;
		case GET_PWM_OUTPUT:
			if(Get_PWM_Sel_Value()!=PWM_OFF_DUTY_SELECTION_VALUE)
			{
				*return_string_ptr = message_On;
			}
			else
			{
				*return_string_ptr = message_Off;
			}
			ret_value = true;
			break;
		case SET_PWM_OUTPUT:
			if(param==0)
			{
				if(Get_PWM_Sel_Value()!=PWM_OFF_DUTY_SELECTION_VALUE)
				{
					pwm_selection_before_off_command = Get_PWM_Sel_Value();
					Set_PWM_Selection_Value(PWM_OFF_DUTY_SELECTION_VALUE);
					PWMOutputSetting(PWM_OFF_DUTY_SELECTION_VALUE);
				}
			}
			else
			{
				if(Get_PWM_Sel_Value()==PWM_OFF_DUTY_SELECTION_VALUE)
				{
					Set_PWM_Selection_Value(pwm_selection_before_off_command);
					PWMOutputSetting(PWM_OFF_DUTY_SELECTION_VALUE);
				}
			}
			*return_string_ptr = message_ok;
			ret_value = true;
			break;
		case SET_PWM_USE_TABLE:
			if(param<POWER_OUTPUT_STEP_TOTAL_NO)
			{
				PowerOutputSetting(param);
				*return_string_ptr = message_ok;
				ret_value = true;
			}
			else
				*return_string_ptr = error_parameter;
			break;
		case GET_FW_VERSION:
			{
				const char voltage_output_welcome_message_line2[] =
				{   'V', 'e', 'r', ':', FW_MAJOR, FW_MIDDLE, FW_MINOR, '_', // "Ver:x.x_" - total 8 chars
				   BUILD_MONTH_CH0, BUILD_MONTH_CH1, BUILD_DAY_CH0, BUILD_DAY_CH1, BUILD_HOUR_CH0, BUILD_HOUR_CH1,  BUILD_MIN_CH0, BUILD_MIN_CH1, // 8 chars
					'\0'};

				strcpy(command_return_string, voltage_output_welcome_message_line2);
				*return_string_ptr = command_return_string;
				ret_value = true;
			}
			break;
		case GET_ECHO:
			*return_string_ptr = (CheckEchoEnableStatus())?message_On:message_Off;
			ret_value = true;
			break;
		case SET_ECHO:
			EchoEnable((param!=0)?true:false);
			*return_string_ptr = message_ok;
			ret_value = true;
			break;
		case GET_PWM_DUTY_VALUE:
		case SET_PWM_DUTY_VALUE:
		case GET_PWM_DUTY_RANGE:
		case GET_PWM_FREQ:
		case SET_PWM_FREQ:
		case GET_PWM_FREQ_RANGE:
		case GET_PWM_DUTY_PERCENTAGE:
			*return_string_ptr = error_developing;	// To be implemented -- return current duty value
			break;
		default:
			// command error
			*return_string_ptr = error_command;
			break;
	}

	return ret_value;
}
