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
	OK_wait_1st_K,
	OK_wait_next_O,
	OK_wait_next_K,
} OK_PATTERN_STATE;

OK_PATTERN_STATE 	OK_state;
uint32_t	OK_cnt;

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
			if(OK_state==OK_wait_next_O)
			{
				// don't need to reset OK_cnt if already detecting OK pattern
				OK_state = OK_wait_next_K;
			}
			else
			{
				OK_cnt = 0;
				OK_state = OK_wait_next_K;
			}
			break;
		case 'K':
		case 'k':
			if(++OK_cnt==DEFAULT_OK_THRESHOLD)
			{
				bRet = true;
			}
			OK_state = OK_wait_next_O;
			break;
		default:
			// always reset if other char
			OK_cnt = 0;
			OK_state = OK_wait_1st_O;
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
			VER_state++;
			if((VER_state-(sizeof(ver_string)-1))==(MAX_VER_NO_LEN-1))		// only 1 more space left
			{
				VER_string_end_of_line = true;
				VER_NO_str[MAX_VER_NO_LEN-1] = '\0';		// End of version string is enforced.
				VER_string_detected = false;
				VER_state = 0;
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

