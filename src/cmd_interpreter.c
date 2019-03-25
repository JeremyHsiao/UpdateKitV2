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
#include "voltage_output.h"
#include "cmd_interpreter.h"

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/
char 	serial_gets_return_string[MAX_SERIAL_GETS_LEN+1];	// Extra one is for '\0'
char 	*ptr_str;
bool 	EchoEnabled;
static	bool In_User_Ctrl_Mode;

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
	CMD_CODE_GET = 0,
	CMD_CODE_SET,
	CMD_CODE_RESERVED_2,
	CMD_CODE_RESERVED_3
};

enum
{
	CMD_OBJECT_USERMODE = 0,
	CMD_OBJECT_PWM_DUTY_VALUE,
	CMD_OBJECT_PWM_DUTY_PERCENTAGE,
	CMD_OBJECT_PWM_DUTY_RANGE,
	CMD_OBJECT_PWM_FREQ_VALUE,
	CMD_OBJECT_PWM_FREQ_RANGE,
	CMD_OBJECT_PWM_OUTPUT,
	CMD_OBJECT_PWM_USE_TABLE,
	CMD_OBJECT_FW_VER,
	CMD_OBJECT_MAX,
};

#define CMD_DEFINE_GET						(((CmdExecutionPacket)CMD_CODE_GET) << CmdCodeBitPos )
#define CMD_DEFINE_SET						(((CmdExecutionPacket)CMD_CODE_SET) << CmdCodeBitPos )
#define CMD_DEFINE_RESERVED3     			(((CmdExecutionPacket)CMD_CODE_RESERVED_3) << CmdCodeBitPos )
#define CMD_DEFINE_OBJ(OBJECT)				(((CmdExecutionPacket)(OBJECT)) << CmdObjBitPos)

#define CMD_GET_OBJECT_VALUE(OBJECT)		( CMD_DEFINE_GET | CMD_DEFINE_OBJ(OBJECT) )
#define CMD_SET_OBJECT_VALUE(OBJECT)	    ( CMD_DEFINE_SET | CMD_DEFINE_OBJ(OBJECT) )
#define CMD_CODE_WITH_OBJECT(PACKET)		(PACKET&(~CmdValueBitMask))
#define CMD_GET_PARAMETER(PACKET)			((PACKET&CmdValueBitMask)>>CmdValueBitPos)
#define CMD_DEFINE_PARAMETER(PARAM)			((((CmdExecutionPacket)PARAM)<<(CmdValueBitPos))&CmdValueBitMask)

#define CMD_NONE_DEFAULT					((((CmdExecutionPacket)CMD_CODE_RESERVED_3) << CmdCodeBitPos ) | CMD_DEFINE_OBJ(CMD_OBJECT_MAX) )

typedef enum {
	//  No GET
	SET_USER_MODE 			= CMD_SET_OBJECT_VALUE(CMD_OBJECT_USERMODE),				// Enter/Leave user control mode
	GET_PWM_DUTY_VALUE		= CMD_GET_OBJECT_VALUE(CMD_OBJECT_PWM_DUTY_VALUE),		// get duty cycle value
	SET_PWM_DUTY_VALUE		= CMD_SET_OBJECT_VALUE(CMD_OBJECT_PWM_DUTY_VALUE),		// set duty cycle value
	GET_PWM_DUTY_PERCENTAGE	= CMD_GET_OBJECT_VALUE(CMD_OBJECT_PWM_DUTY_PERCENTAGE),		// get duty cycle value
	SET_PWM_DUTY_PERCENTAGE	= CMD_SET_OBJECT_VALUE(CMD_OBJECT_PWM_DUTY_PERCENTAGE),		// set duty cycle value
	GET_PWM_DUTY_RANGE		= CMD_GET_OBJECT_VALUE(CMD_OBJECT_PWM_DUTY_RANGE),		// get duty cycle range
	//  No SET
	GET_PWM_FREQ 			= CMD_GET_OBJECT_VALUE(CMD_OBJECT_PWM_FREQ_VALUE),		// get duty cycle value
	SET_PWM_FREQ 			= CMD_SET_OBJECT_VALUE(CMD_OBJECT_PWM_FREQ_VALUE),		// set duty cycle value
	GET_PWM_FREQ_RANGE		= CMD_GET_OBJECT_VALUE(CMD_OBJECT_PWM_FREQ_RANGE),		// get frequency range
	//  No SET
	GET_PWM_OUTPUT 			= CMD_GET_OBJECT_VALUE(CMD_OBJECT_PWM_OUTPUT),			// get if pwm output is enabled
	SET_PWM_OUTPUT 			= CMD_SET_OBJECT_VALUE(CMD_OBJECT_PWM_OUTPUT),			// set pwm output enable
	//  No GET
	SET_PWM_USE_TABLE		= CMD_SET_OBJECT_VALUE(CMD_OBJECT_PWM_USE_TABLE),			// use table value in code as output value
	GET_FW_VERSION			= CMD_GET_OBJECT_VALUE(CMD_OBJECT_FW_VER),				// get FW version
	//  No SET
	DEFAULT_NON_CMD			= CMD_NONE_DEFAULT
} CMD_LIST;

char *error_parameter  = "Parameter error.";
char *error_command    = "Command error.";
char *error_developing = "Under development";
char *message_ok       = "OK";
char *pwm_output_On    = "PWM_ON";
char *pwm_output_Off   = "PWM_OFF";

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/
CmdExecutionPacket		received_cmd_packet;

/*****************************************************************************
 * Private functions
 ****************************************************************************/

/*****************************************************************************
 * Public functions
 ****************************************************************************/

void init_cmd_interpreter(void)
{
	*serial_gets_return_string = '\0';
	ptr_str = serial_gets_return_string;
	EchoEnabled = true; //
	In_User_Ctrl_Mode = false;
}

/*
typedef enum {
	ISS_1ST_CHAR = 0,				// Check if 1st char meets some command
	ISS_IDENTIFYING,				// Checking among several command
	ISS_CMD_ONE_LEFT,				//
	ISS_GETTING_PARAMETER,
	ISS_CRLF_FOUND,
	ISS_NONE_MATCH,
	ISS_MAX_STATE_NO
} IDENTIFY_STRING_STATE;


typedef enum {
    CMD_NONE 			= 0,		//  None
    CMD_OFF				= (1L<<ID_OFF),	//	cmd.off
    CMD_ON				= (1L<<ID_ON),	//	cmd.on
    CMD_GETPWMDUTY		= (1L<<ID_GETPWMDUTY),	//	get.pwmduty
    CMD_GETPWMDUTYRANGE	= (1L<<ID_GETPWMDUTYRANGE),	//	get.pwmdutyrange
    CMD_GETPWMFREQ		= (1L<<ID_GETPWMFREQ),	//	get.pwmfreq
    CMD_GETPWMFREQRANGE	= (1L<<ID_GETPWMFREQRANGE),	//	get.pwmfreqrange
    CMD_GETVER			= (1L<<ID_GETVER),	//	get.ver
    CMD_PWMOFF			= (1L<<ID_PWMOFF),	//	pwm.off
    CMD_PWMON			= (1L<<ID_PWMON),	//	pwm.on
    CMD_PWMUPDATEKIT	= (1L<<ID_PWMUPDATEKIT),	//	pwm.updatekit
    CMD_PWMUSER			= (1L<<ID_PWMUSER),	//	pwm.user
    CMD_SETPWMDUTY		= (1L<<ID_SETPWMDUTY),	//	set.pwmduty. (xxxx)
    CMD_SETPWMFREQ		= (1L<<ID_SETPWMFREQ),	//	set.pwmfreq. (xxxxxx)
    CMD_ENTER_CMD_MODE 	= (1L<<ID_ENTER_CMD_MODE),	//	x&Vht&GD
    CMD_MAX				= ~0
} FoundCMD;

// For quick response time of state ISS_1ST_CHAR:

const char first_cmd_char_list[] = "cgpsx";
const uint32_t first_cmd_bit_list[] = {
	( CMD_OFF | CMD_ON ),
    ( CMD_GETPWMDUTY | CMD_GETPWMDUTYRANGE| CMD_GETPWMFREQ | CMD_GETPWMFREQRANGE | CMD_GETVER ),
	( CMD_PWMOFF | CMD_PWMON | CMD_PWMUPDATEKIT | CMD_PWMUSER ),
	( CMD_SETPWMDUTY | CMD_SETPWMFREQ ),
	( CMD_ENTER_CMD_MODE ),
	CMD_NONE
};

const uint8_t first_cmd_range_list[][2] = {
	{ID_OFF, ID_ON},
	{ID_GETPWMDUTY, ID_GETVER},
	{ID_PWMOFF, ID_PWMUSER},
	{ID_SETPWMDUTY, CMD_SETPWMFREQ},
	{ID_ENTER_CMD_MODE}
};

// for all state

static uint8_t 	cmd_id_low, cmd_id_high;
static IDENTIFY_STRING_STATE iss_state = ISS_1ST_CHAR;
void IdentifyCommand(char input_ch)
{
	uint8_t	temp;
	char	temp_ch;

	switch (iss_state)
	{
		// Use LUT to identify 1st char
		case ISS_1ST_CHAR:
			temp = sizeof(first_cmd_char_list)-1;
			cmd_bit = CMD_NONE;
			iss_state = ISS_NONE_MATCH;
			do
			{
				if(input_ch==first_cmd_char_list[temp])
				{
					cmd_id_low = first_cmd_range_list[temp][0];
					cmd_id_high = first_cmd_range_list[temp][1];
					cmd_bit = first_cmd_bit_list[temp];
					input_char_index = 1;
					iss_state = ISS_IDENTIFYING;
					break;
				}
			}
			while (temp-->0);
			break;

		case ISS_IDENTIFYING:
			temp = cmd_id_low;
			do
			{
				if(input_ch!=*(command_list[temp]+input_char_index))
				{
					cmd_bit &= ~(1L<<temp);
					cmd_id_low = temp+1;
				}
			}
			while(++temp>cmd_id_high);
			if(cmd_bit==0)
			{
				iss_state = ISS_NONE_MATCH;
			}
			else
			{
				input_char_index++;
				cmd_id_high = 31 - __builtin_clz (cmd_bit);
				cmd_id_low  = __builtin_ctz (cmd_bit);
				if(cmd_id_high == cmd_id_low)
				{
					iss_state = ISS_CMD_ONE_LEFT;
				}
			}
			break;

		case ISS_CMD_ONE_LEFT:
			temp_ch = *(command_list[cmd_id_low]+input_char_index);
			// still not end-of-command
			if(temp_ch!='\0')
			{
				if(input_ch!=*(command_list[cmd_id_low]+input_char_index))
				{
					iss_state = ISS_NONE_MATCH;
				}
				input_char_index++;
			}
			else
			{
				iss_state = ISS_CMD_ONE_LEFT; // Jump to
			}
			break;

		case ISS_GETTING_PARAMETER:

			break;

		case ISS_CRLF_FOUND:
			// Start to process command
			break;

		case ISS_NONE_MATCH:
			// Stay at this state until CR or LF is encountered.
			if ((input_ch=='\r')||(input_ch=='\n'))
			{
				iss_state = ISS_1ST_CHAR;
			}
			break;

		default:
			iss_state = ISS_NONE_MATCH;
			break;
	}
}
*/

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

typedef enum {
    ID_OFF = 0			,
    ID_ON				,
    ID_GETPWMDUTY		,
    ID_GETPWMDUTYRANGE	,
    ID_GETPWMFREQ		,
    ID_GETPWMFREQRANGE	,
    ID_GETVER			,
    ID_PWMOFF			,
    ID_PWMON			,
    ID_PWMUPDATEKIT		,
    ID_PWMUSER			,
    ID_SETPWMDUTY		,
    ID_SETPWMFREQ		,
    ID_ENTER_CMD_MODE 	,
    ID_MAX
} CMDIndex;

const char *command_list[] =
{
	"cmd.off",
	"cmd.on",
	"get.pwmduty",
	"get.pwmdutyrange",
	"get.pwmfreq",
	"get.pwmfreqrange",
	"get.ver",
	"pwm.off",
	"pwm.on",
	"pwm.updatekit",
	"pwm.user",
	"set.pwmduty.$$$$",
	"set.pwmfreq.$$$",
	"x&Vht&GD",
};

bool CheckIfUserCtrlModeCommand(char *input_str)
{
	char* token = strtok(input_str, " ");
	if (strcmp(token,command_list[ID_ENTER_CMD_MODE])==0)
		return true;
	else
		return false;
}

bool ProcessUserCommand(char *input_str, CmdExecutionPacket *ptr_packet)
{
	char 				*token = strtok(input_str, " ");
	bool				ret = true;

	// 1st command
	if(token!=NULL)
	{
		OutputString_with_newline(token);
		if (strcmp(token,"get")==0)
		{
			*ptr_packet = CMD_DEFINE_GET;
		}
		else if (strcmp(token,"set")==0)
		{
			*ptr_packet = CMD_DEFINE_SET;
		}
		else
		{
			*ptr_packet = CMD_CODE_RESERVED_3;
		}
	}
	else
	{
		// return false if none token at all
		ret = false;
	}

	// 2nd object
	token = strtok(NULL, " ");
	if(token!=NULL)
	{
		if (strcmp(token,"pwm_percent")==0)
		{
			*ptr_packet |= CMD_DEFINE_OBJ(CMD_OBJECT_PWM_DUTY_PERCENTAGE);
			OutputString_with_newline(token);
		}
		else if (strcmp(token,"user_mode")==0)
		{
			*ptr_packet |= CMD_DEFINE_OBJ(CMD_OBJECT_USERMODE);
		}
		else if (strcmp(token,"pwm_output")==0)
		{
			*ptr_packet |= CMD_DEFINE_OBJ(CMD_OBJECT_PWM_OUTPUT);
		}
		else if (strcmp(token,"pwm_use_table")==0)
		{
			*ptr_packet |= CMD_DEFINE_OBJ(CMD_OBJECT_PWM_USE_TABLE);
			OutputString_with_newline(token);
		}
		else
		{
			*ptr_packet |= CMD_DEFINE_OBJ(CMD_OBJECT_MAX);
		}
	}
	else
	{
		*ptr_packet |= CMD_DEFINE_OBJ(CMD_OBJECT_MAX);
	}

	// 3rd parameter
	token = strtok(NULL, " ");
	if(token!=NULL)
	{
		int val;
		OutputString_with_newline(token);
		val = atoi(token);
		*ptr_packet |= CMD_DEFINE_PARAMETER(val);
	}
	OutputHexValue_with_newline(*ptr_packet);

	return ret;
}

void SetUserCtrlModeFlag(bool flag)
{
	In_User_Ctrl_Mode = flag;
}

bool CommandInterpreter(char *input_str, CmdExecutionPacket* ptr_packet)
{
	bool ret = false;

	if(In_User_Ctrl_Mode)
	{
		ret = ProcessUserCommand(input_str, ptr_packet);
	}
	else
	{
		if(CheckIfUserCtrlModeCommand(input_str))
		{
			//*ptr_packet = SET_USER_MODE | CMD_DEFINE_PARAMETER(1);
			EVENT_Enter_User_Ctrl_Mode = true;
			SetUserCtrlModeFlag(true);
			ret = true;
		}
	}
	return ret;
}

bool CommandExecution(CmdExecutionPacket cmd_packet, char **return_string_ptr)
{
	uint32_t	param = CMD_GET_PARAMETER(cmd_packet);
	bool		ret_value = false;

	switch(CMD_CODE_WITH_OBJECT(cmd_packet))
	{
		case SET_USER_MODE:
			if(param!=0)
			{
				EVENT_Enter_User_Ctrl_Mode = true;
				SetUserCtrlModeFlag(true);
				*return_string_ptr = message_ok;
			}
			else
			{
				SetUserCtrlModeFlag(false);
				*return_string_ptr = error_developing;	// To be implemented -- an event to leave User Ctrl Mode
			}
			break;
		case GET_PWM_DUTY_VALUE:
			*return_string_ptr = error_developing;	// To be implemented -- return current duty value
			break;
		case SET_PWM_DUTY_VALUE:
			*return_string_ptr = error_developing;
			break;
		case GET_PWM_DUTY_PERCENTAGE:
			*return_string_ptr = error_developing;	// To be implemented -- return current duty value
			break;
		case SET_PWM_DUTY_PERCENTAGE:
			if(param<=MAX_DUTY_SELECTION_VALUE)
			{
				PWMOutputSetting(param+DUTY_SELECTION_OFFSET_VALUE);
				*return_string_ptr = message_ok;
				ret_value = true;
			}
			else
				*return_string_ptr = error_parameter;
			break;
		case GET_PWM_DUTY_RANGE:
			*return_string_ptr = error_developing;	// To be implemented -- return current duty range
			break;
		case GET_PWM_FREQ:
			*return_string_ptr = error_developing;	// To be implemented -- return current frequency
			break;
		case SET_PWM_FREQ:
			*return_string_ptr = error_developing;
			break;
		case GET_PWM_FREQ_RANGE:
			*return_string_ptr = error_developing;	// To be implemented -- return current frequency range
			break;
		case GET_PWM_OUTPUT:
			if (Chip_GPIO_GetPinState(LPC_GPIO, VOUT_ENABLE_GPIO_PORT, VOUT_ENABLE_GPIO_PIN))
			{
				*return_string_ptr = pwm_output_On;
			}
			else
			{
				*return_string_ptr = pwm_output_Off;
			}
			ret_value = true;
			break;
		case SET_PWM_OUTPUT:
			if(param==0)
			{
				Chip_GPIO_SetPinOutLow(LPC_GPIO, VOUT_ENABLE_GPIO_PORT, VOUT_ENABLE_GPIO_PIN);
			}
			else
			{
				Chip_GPIO_SetPinOutHigh(LPC_GPIO, VOUT_ENABLE_GPIO_PORT, VOUT_ENABLE_GPIO_PIN);
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
			*return_string_ptr = error_developing;	// To be implemented -- return fw version
			break;
		default:
			// command error
			*return_string_ptr = error_command;
			break;
	}

	return ret_value;
}

void EchoEnable(bool enabled)
{
	EchoEnabled = enabled;
}

bool CheckEchoEnableStatus(void)
{
	return EchoEnabled;
}

//	// Returns 2nd token and check
//	{
//		char* token = strtok(NULL, " ");
//		OutputString_with_newline(token);
//	}
//
//	// Returns 3rd token and check
//	{
//		char* token = strtok(NULL, " ");
//		OutputString_with_newline(token);
//	}

