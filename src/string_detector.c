/*
 * string_detector.c
 *
 *  Created on: 2018年11月27日
 *      Author: Jeremy.Hsiao
 */
#include "chip.h"
#include "string.h"
#include "string_detector.h"
#include "UpdateKitV2.h"

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/
typedef enum {
	OK_wait_1st_O = 0,
//	OK_wait_1st_K,
	OK_wait_next_O,
	OK_wait_next_K,
} OK_PATTERN_STATE;

OK_PATTERN_STATE 	OK_state;
uint32_t			OK_cnt;

uint8_t POWERON_state;
const uint8_t poweron_string[] = "@POWERON";
bool    POWERON_string_detected;

uint8_t VER_state;
const uint8_t ver_string[] = "@VER";
bool    VER_string_detected;
bool    VER_string_end_of_line;
uint8_t ver_string_index;
uint8_t VER_NO_str[MAX_VER_NO_LEN];
uint8_t Previous_VER_NO_str[MAX_VER_NO_LEN];

char serial_gets_return_string[MAX_SERIAL_GETS_LEN+1];	// Extra one is for '\0'
char *ptr_str;

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/

/*****************************************************************************
 * Private functions
 ****************************************************************************/

/*****************************************************************************
 * Public functions
 ****************************************************************************/
void reset_string_detector(void)
{
	OK_state = 0;
	OK_cnt = 0;
	POWERON_state = 0;
	POWERON_string_detected = false;
	VER_state = 0;
	VER_string_detected = false;
	VER_string_end_of_line = false;
	VER_NO_str[0]='\0';
	ver_string_index = 0;

	ptr_str = serial_gets_return_string;
}

void Clear_OK_pattern_state(void)
{
	OK_state = OK_wait_1st_O;
	OK_cnt = 0;
}

uint32_t Read_OK_Count(void)
{
	return OK_cnt;
}

bool locate_OK_pattern_process(char input_ch)
{
	bool	bRet = false;

	switch(input_ch)
	{
		case '\r':
		case '\n':
		case '\t':
		case ' ':
			// simply skip this
			break;
		case 'O':
		case 'o':
			if((OK_state==OK_wait_1st_O)||(OK_state==OK_wait_next_O))
			{
				// don't need to reset OK_cnt if already detecting OK pattern
				OK_state = OK_wait_next_K;
			}
			else
			{
				OK_state = OK_wait_next_K;
				OK_cnt = 0;
			}
			break;
		case 'K':
		case 'k':
			if(OK_state==OK_wait_next_K)
			{
				if(++OK_cnt==DEFAULT_OK_THRESHOLD)
				{
					bRet = true;
				}
				OK_state = OK_wait_next_O;
			}
			else
			{
				Clear_OK_pattern_state();
			}
			break;
		default:
			// always reset if other char
			Clear_OK_pattern_state();
			break;
	}

	return bRet;
}

bool Get_POWERON_pattern(void)
{
	return POWERON_string_detected;
}

void Clear_POWERON_pattern(void)
{
	POWERON_string_detected = false;
}

void locate_POWERON_pattern_process(char input_ch)
{
	// safe-guard distorted POWERON_state
	if(POWERON_state<(sizeof(poweron_string)-1))
	{
		if(input_ch==poweron_string[POWERON_state])
		{
			POWERON_state++;
			if (POWERON_state==(sizeof(poweron_string)-1))	// Reaching '\0' of poweron_string
			{
				POWERON_string_detected = true;
				POWERON_state = 0;
			}
		}
		else
		{
			if(input_ch==poweron_string[0])
			{
				POWERON_state = 1;
			}
			else
			{
				POWERON_state = 0;
			}
		}
	}
	else
	{
		POWERON_state = 0;
	}
}

bool Found_VER_string(void)
{
	return VER_string_end_of_line;
}

void Clear_VER_string(void)
{
	VER_string_end_of_line=false;
	VER_NO_str[0] = '\0';
}

uint8_t *Get_VER_string(void)
{
	if(VER_string_end_of_line==false)
	{
		return (uint8_t *)(0);
	}
	else
	{
		return VER_NO_str;
	}
}

void locate_VER_pattern_process(char input_ch)
{
	// safe-guard distorted POWERON_state
	if(VER_state<sizeof((ver_string)-1))
	{
		if(input_ch==ver_string[VER_state])
		{
			VER_state++;
			if (VER_state==(sizeof(ver_string)-1))	// Reaching '\0' of ver_string
			{
				VER_string_detected = true;
			}
		}
		else
		{
			if(input_ch==ver_string[0])
			{
				VER_state = 1;
			}
			else
			{
				VER_state = 0;
			}
			VER_string_detected = false;
		}
	}
	else if (VER_string_detected==true)
	{
		if ((input_ch=='\r')||(input_ch=='\n'))
		{
			VER_string_end_of_line = true;
			VER_NO_str[VER_state-(sizeof(ver_string)-1)] = '\0';
			VER_string_detected = false;
			VER_state = 0;
		}
		else
		{
			VER_NO_str[VER_state-(sizeof(ver_string)-1)] = input_ch;
			if((VER_state-(sizeof(ver_string)-1))<(MAX_VER_NO_LEN-1))		// still >1 space left --> add one more char; otherwise simply discard this char
			{
				VER_state++;
			}
		}
	}
	else // default for abnormal situation
	{
		VER_string_detected = false;
		VER_string_end_of_line = false;
		VER_state = 0;
		VER_NO_str[0] = '\0';
	}
}

int Remove_NonLCMASCII_ESCCode(char *str)
{
	char 		*ptr = str;
	bool		esc_now = false;
	int			str_len=0;

	while(ptr!=0)
	{
		if(esc_now)
		{

		}
		else
		{
			if(*ptr)
			{

			}
		}
	}
	return str_len;
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
static uint32_t cmd_bit;
static IDENTIFY_STRING_STATE iss_state = ISS_1ST_CHAR;
static uint8_t	input_char_index;
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
		case 26:		//ctrl-z
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

