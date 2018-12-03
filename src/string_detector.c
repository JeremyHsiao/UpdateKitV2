/*
 * string_detector.c
 *
 *  Created on: 2018年11月27日
 *      Author: Jeremy.Hsiao
 */
#include "chip.h"
#include "string.h"

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/
uint8_t 	OK_state;
uint32_t	OK_cnt;

uint8_t POWERON_state;
uint8_t poweron_string[] = "@POWERON";
bool    POWERON_string_detected;

uint8_t VER_state;

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
	POWERON_string_detected = 0;
	VER_state = 0;
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
	if(POWERON_state<sizeof(poweron_string))
	{
		if(input_ch==poweron_string[POWERON_state])
		{
			POWERON_state++;
			if (POWERON_state==(sizeof(poweron_string)-1))	// Reaching '\0'
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

void locate_Line_data(char input_ch)
{

}


