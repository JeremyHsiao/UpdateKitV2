/*
 * string_detector.c
 *
 *  Created on: 2018年11月27日
 *      Author: Jeremy.Hsiao
 */
#include "chip.h"
#include "string.h"
#include "string_detector.h"

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/
uint8_t 	OK_state;
uint32_t	OK_cnt;

uint8_t POWERON_state;
uint8_t poweron_string[] = "@POWERON";
bool    POWERON_string_detected;

uint8_t VER_state;
uint8_t ver_string[] = "@VER";
bool    VER_string_detected;
bool    VER_string_end_of_line;
uint8_t ver_string_index;
uint8_t VER_NO_str[MAX_VER_NO_LEN];

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

uint32_t locate_OK_pattern_process(char input_ch)
{
	// Skip '\r', '\n', ' '
	if ((input_ch=='\r')||(input_ch=='\n')||(input_ch==' '))
	{
		return OK_cnt;
	}
	else
	{
		switch(OK_state)
		{
			case 0:
				if(input_ch=='O') // Check "O"
				{
					OK_state = 1;
				}
				else
				{
					OK_state = 0;
					OK_cnt = 0;
				}
				break;
			case 1:
				if(input_ch=='K') // Check "K"
				{
					OK_state = 2;
					OK_cnt++;
				}
				else
				{
					if(input_ch=='O') // Check "O"
					{
						OK_state = 1;
					}
					else
					{
						OK_state = 0;
						OK_cnt=0;
					}
				}
				break;
			case 2:
				if(input_ch=='O') // Check "O" -- after 1st OK
				{
					OK_state = 3;
				}
				else
				{
					OK_state = 0;
					OK_cnt=0;
				}
				break;
			case 3:
				if(input_ch=='K') // Check "K" -- after 1st OK
				{
					OK_state = 2;
					OK_cnt++;
				}
				else
				{
					OK_state = 0;
					OK_cnt=0;
				}
				break;
			default:
				OK_state = 0;
				OK_cnt=0;
				break;
		}
		return OK_cnt;
	}
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
