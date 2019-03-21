/*
 * cmd_interpreter.c
 *
 *  Created on: 2018年11月27日
 *      Author: Jeremy.Hsiao
 */
#include "chip.h"
#include "string.h"
#include "ctype.h"
#include "UpdateKitV2.h"
#include "uart_0_rb.h"
#include "cmd_interpreter.h"

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/
char serial_gets_return_string[MAX_SERIAL_GETS_LEN+1];	// Extra one is for '\0'
char *ptr_str;
bool EchoEnabled;

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/

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
	EchoEnabled = false;
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

bool CommandInterpreter(char *input_str)
{
	return false;
}

int EchoInputString(char *input_str)
{
	if (EchoEnabled)
	{
		return OutputString_with_newline(input_str);
	}
	else
	{
		return 0;
	}
}

void EchoEnable(bool enabled)
{
	EchoEnabled = enabled;
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

