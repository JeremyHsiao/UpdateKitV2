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
uint8_t 	OK_state = 0;
uint32_t	OK_cnt = 0;


uint8_t POWERON_state = 0;
uint8_t VER_state = 0;

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/

/*****************************************************************************
 * Private functions
 ****************************************************************************/

/*****************************************************************************
 * Public functions
 ****************************************************************************/

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
					OK_state = 0;
					OK_cnt=0;
				}
				break;
			case 2:
				if(input_ch=='O') // Check "K"
				{
					OK_state = 1;
				}
				else
				{
					OK_state = 0;
					OK_cnt=0;
				}
				break;
		}
		return OK_cnt;
	}
}

void locate_POWERIN_pattern_process(char input_ch)
{

}

void locate_Line_data(char input_ch)
{

}


