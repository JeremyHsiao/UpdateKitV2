/*
 * user_if.c
 *
 *  Created on: Jun 4, 2020
 *      Author: Jerem
 */

#include "chip.h"
#include "fw_version.h"
#include "string.h"
#include "res_state.h"
#include "sw_timer.h"
#include "gpio.h"
#include "uart_0_rb.h"
#include "user_opt.h"
#include "user_if.h"
#include "lcd_module.h"

#define BUTTON_SRC_PORT		BLUE_KEY_GPIO_PORT
#define BUTTON_SRC_PIN		BLUE_KEY_GPIO_PIN
#define BUTTON_SEL_PORT		YELLOW_KEY_GPIO_PORT
#define BUTTON_SEL_PIN		YELLOW_KEY_GPIO_PIN
#define BUTTON_INC_PORT		GREEN_KEY_GPIO_PORT
#define BUTTON_INC_PIN		GREEN_KEY_GPIO_PIN
#define BUTTON_DEC_PORT		RED_KEY_GPIO_PORT
#define BUTTON_DEC_PIN		RED_KEY_GPIO_PIN
#define BUTTON_ISP_PORT		SWITCH_KEY_GPIO_PORT
#define BUTTON_ISP_PIN		SWITCH_KEY_GPIO_PIN

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/
uint8_t				current_output_stage;
#ifdef _BOARD_DEBUG_SW_
	static uint8_t		res_index = 4;		// default at Value-R menu
#else
	static uint8_t		res_index = 3;		// default at Value-R menu
#endif // #ifdef _BOARD_DEBUG_SW_


char  *FineTuneResistorMsg[] = {
	//"0123456789012345"
	  "RL1/22        1\xf4",		// power(2,0)
	  "RL43  R16/76/136",
	  "RL2/23        2\xf4",		// power(2,1)
	  "RL44  R17/77/137",
	  "RL3/24        4\xf4",		// power(2,2)
	  "RL45  R18/78/138",
	  "RL4/25        8\xf4",		// power(2,3)
	  "RL46  R19/79/139",
	  "RL5/26       16\xf4",		// power(2,4)
	  "RL47  R20/80/140",
	  "RL6/27       32\xf4",		// power(2,5)			/// RL and R to be updated later after this line
	  "RL48  R21/81/141",
	  "RL7/28       64\xf4",		// power(2,6)
	  "RL49  R22/82/142",
	  "RL8/29      128\xf4",		// power(2,7)
	  "RL50  R23/83/143",
	  "RL9/30      256\xf4",		// power(2,8)
	  "RL51 R40/100/160",
	  "RL10/31     512\xf4",		// power(2,9)
	  "RL52 R41/101/161",
	  "RL11/32    1024\xf4",		// power(2,10)
	  "RL53 R42/102/162",
	  "RL12/33    2048\xf4",		// power(2,11)
	  "RL54 R43/103/163",
	  "RL13/34    4096\xf4",		// power(2,12)
	  "RL55 R44/104/164",
	  "RL14/35    8192\xf4",		// power(2,13)
	  "RL56 R45/105/165",
	  "RL15/36   16384\xf4",		// power(2,14)
	  "RL57 R46/106/166",
	  "RL16/37   32768\xf4",		// power(2,15)
	  "RL58 R47/107/167",
	  "RL17/38   65536\xf4",		// power(2,16)
	  "RL59 R64/124/184",
	  "RL18/39  131072\xf4",		// power(2,17)
	  "RL60 R65/125/185",
	  "RL19/40  262144\xf4",		// power(2,18)
	  "RL61 R66/126/186",
	  "RL20/41  524288\xf4",		// power(2,19)
	  "RL62 R67/127/187",
	  "RL21/42/63-0  0\xf4",		// 0ohm - only relay 1-4 & RL21
	  "Shortcut On     ",
	  "RL21/42/63-1  0\xf4",		// 0ohm - all relay 1-21
	  "Shortcut Off    ",
};

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/

UPDATE_STATE	current_system_proc_state;
uint16_t		max_upgrade_time_in_S;
uint8_t			lcm_page_change_duration_in_sec;

/*****************************************************************************
 * Private functions
 ****************************************************************************/

/*****************************************************************************
 * Public functions
 ****************************************************************************/
bool Check_if_Resistor_in_Range(uint32_t res)
{
	if((res<(1UL<<20))&&(res>0))
	{
		return true;
	}
	else
	{
		return false;
	}
}

void Init_Value_From_EEPROM(void)
{
	Load_Resistor_Value();
}

void Init_HotSpringBoard_variables(void)
{
	current_system_proc_state = US_SYSTEM_BOOTUP_STATE;
	lcm_page_change_duration_in_sec = DEFAULT_LCM_PAGE_CHANGE_S_WELCOME;
}

bool lcm_text_buffer_cpy(LCM_PAGE_ID page_id, uint8_t row, uint8_t col, const void * restrict __s2, size_t len)
{
	// If row/col is out-of-range, skip
	if((row>=LCM_DISPLAY_ROW)||(col>=LCM_DISPLAY_COL))
	{
		return false;
	}
	// If size is larger, simply skip out-of-boundary text
	if((col+len-1)>=LCM_DISPLAY_COL)
	{
		len=LCM_DISPLAY_COL-col;
	}
	// execute actual memcpy
	memcpy((void *)&lcd_module_display_content[page_id][row][col],__s2, len);
	return true;
}

void lcm_content_init(void)
{
	char const		*welcome_message_line1 =  "Hot Spring Board";
	const uint8_t	welcome_message_line2[] =
	{   'F', 'W', ':', 'V', FW_MAJOR, FW_MIDDLE, FW_MINOR, '_', // "FW:Vx.x-" - total 8 chars
	   BUILD_MONTH_CH0, BUILD_MONTH_CH1, BUILD_DAY_CH0, BUILD_DAY_CH1, BUILD_HOUR_CH0, BUILD_HOUR_CH1,  BUILD_MIN_CH0, BUILD_MIN_CH1, // 8 chars
		'\0'};

	// Prepare firmware version for welcome page

	// Welcome page														 1234567890123456
	lcm_text_buffer_cpy(LCM_WELCOME_PAGE,0,0,welcome_message_line1,LCM_DISPLAY_COL);
	lcm_text_buffer_cpy(LCM_WELCOME_PAGE,1,0,welcome_message_line2,LCM_DISPLAY_COL);

	lcm_text_buffer_cpy(LCM_5V_PROTECTION_DISPLAY,0,0,    "     VOLT:      ",LCM_DISPLAY_COL);
	lcm_text_buffer_cpy(LCM_5V_PROTECTION_DISPLAY,1,0,    "NEED 5V POWER   ",LCM_DISPLAY_COL);

	lcm_text_buffer_cpy(LCM_INPUT_HIGH_BLINKING,0,0,    "HIGH VOLT:      ",LCM_DISPLAY_COL);
	lcm_text_buffer_cpy(LCM_INPUT_HIGH_BLINKING,1,0,    "NEED 5V POWER   ",LCM_DISPLAY_COL);

	lcm_text_buffer_cpy(LCM_INPUT_LOW_BLINKING,0,0,    "LOW  VOLT:      ",LCM_DISPLAY_COL);
	lcm_text_buffer_cpy(LCM_INPUT_LOW_BLINKING,1,0,    "NEED 5V POWER   ",LCM_DISPLAY_COL);

	lcm_text_buffer_cpy(LCM_SHIFT_REGISTER_DISPLAY,0,0,"TPIC6B595 ERROR ",LCM_DISPLAY_COL);
	lcm_text_buffer_cpy(LCM_SHIFT_REGISTER_DISPLAY,1,0,"PLEASE FIX BOARD",LCM_DISPLAY_COL);

//	//                      			 1234567890123456
//	lcm_text_buffer_cpy(LCM_PC_MODE,0,0,"PC Mode: Press  ", LCM_DISPLAY_COL);
//	lcm_text_buffer_cpy(LCM_PC_MODE,1,0,"button to change", LCM_DISPLAY_COL);

	lcm_text_buffer_cpy(LCM_SINGLE_VR_DISPLAY,0,0,"R0:    1        ",LCM_DISPLAY_COL);
	lcm_text_buffer_cpy(LCM_SINGLE_VR_DISPLAY,1,0,"button to change",LCM_DISPLAY_COL);

	//	//                      			 	      1234567890123456
	lcm_text_buffer_cpy(LCM_ALL_VR_DISPLAY,0,0,      "Value-R  A:    2", LCM_DISPLAY_COL);
	lcm_text_buffer_cpy(LCM_ALL_VR_DISPLAY,1,0,      "B:    2  C:    2", LCM_DISPLAY_COL);

	lcm_text_buffer_cpy(LCM_ALL_SET_BLINKING,0,0,    "Set RA   A:    2", LCM_DISPLAY_COL);
	lcm_text_buffer_cpy(LCM_ALL_SET_BLINKING,1,0,    "B:    2  C:    2", LCM_DISPLAY_COL);

	lcm_text_buffer_cpy(LCM_ALL_SET_R0_BLINKING,0,0, "         A:     ", LCM_DISPLAY_COL);
	lcm_text_buffer_cpy(LCM_ALL_SET_R0_BLINKING,1,0, "B:    2  C:    2", LCM_DISPLAY_COL);

	lcm_text_buffer_cpy(LCM_ALL_SET_R1_BLINKING,0,0, "         A:    2", LCM_DISPLAY_COL);
	lcm_text_buffer_cpy(LCM_ALL_SET_R1_BLINKING,1,0, "B:       C:    2", LCM_DISPLAY_COL);

	lcm_text_buffer_cpy(LCM_ALL_SET_R2_BLINKING,0,0, "         A:    2", LCM_DISPLAY_COL);
	lcm_text_buffer_cpy(LCM_ALL_SET_R2_BLINKING,1,0, "B:    2  C:     ", LCM_DISPLAY_COL);

	lcm_text_buffer_cpy(LCM_ALL_SET_2N_VALUE,   0,0, FineTuneResistorMsg[0], LCM_DISPLAY_COL);
	lcm_text_buffer_cpy(LCM_ALL_SET_2N_VALUE,   1,0, FineTuneResistorMsg[1], LCM_DISPLAY_COL);

	//lcm_text_buffer_cpy(LCM_ALL_SEL_UNIT_DISPLAY,0,0,    "Fine-tuning RA  ", LCM_DISPLAY_COL);
	//lcm_text_buffer_cpy(LCM_ALL_SEL_UNIT_DISPLAY,1,0,    "Value:       1 \xf4", LCM_DISPLAY_COL);

	//lcm_text_buffer_cpy(LCM_SEL_UNIT_R0_DISPLAY,0,0,    "Fine-tuning RA  ", LCM_DISPLAY_COL);
	//lcm_text_buffer_cpy(LCM_SEL_UNIT_R0_DISPLAY,1,0,    "Value:      1  \xf4", LCM_DISPLAY_COL);

	//lcm_text_buffer_cpy(LCM_SEL_SET_R1_DISPLAY,0,0,    "Fine-tuning RB  ", LCM_DISPLAY_COL);
	//lcm_text_buffer_cpy(LCM_SEL_SET_R1_DISPLAY,1,0,    "Value:       1 \xf4", LCM_DISPLAY_COL);

	//lcm_text_buffer_cpy(LCM_SEL_SET_R2_DISPLAY,0,0,    "Fine-tuning RC  ", LCM_DISPLAY_COL);
	//lcm_text_buffer_cpy(LCM_SEL_SET_R2_DISPLAY,1,0,    "Value:       1 \xf4", LCM_DISPLAY_COL);

	lcm_text_buffer_cpy(LCM_ALL_SEL_SET_DISPLAY,0,0,    "UNIT:\x2B\x2D      1  ", LCM_DISPLAY_COL);
	lcm_text_buffer_cpy(LCM_ALL_SEL_SET_DISPLAY,1,0,    "RA:          2 \xf4", LCM_DISPLAY_COL);

	//lcm_text_buffer_cpy(LCM_SEL_R1_DISPLAY,0,0,    "                ", LCM_DISPLAY_COL);
	//lcm_text_buffer_cpy(LCM_SEL_R1_DISPLAY,1,0,    "RB:          1 \xf4", LCM_DISPLAY_COL);

	//lcm_text_buffer_cpy(LCM_SEL_R2_DISPLAY,0,0,    "                ", LCM_DISPLAY_COL);
	//lcm_text_buffer_cpy(LCM_SEL_R2_DISPLAY,1,0,    "RC:          1 \xf4", LCM_DISPLAY_COL);

	lcm_text_buffer_cpy(LCM_SEL_UNIT_0_DISPLAY,0,0,    "UNIT:\x2B\x2D      1  ", LCM_DISPLAY_COL);
	lcm_text_buffer_cpy(LCM_SEL_UNIT_0_DISPLAY,1,0,    "RA:          2 \xf4", LCM_DISPLAY_COL);

	lcm_text_buffer_cpy(LCM_SEL_UNIT_1_DISPLAY,0,0,    "UNIT:\x2B\x2D     10  ", LCM_DISPLAY_COL);
	lcm_text_buffer_cpy(LCM_SEL_UNIT_1_DISPLAY,1,0,    "RA:            \xf4", LCM_DISPLAY_COL);

	lcm_text_buffer_cpy(LCM_SEL_UNIT_2_DISPLAY,0,0,    "UNIT:\x2B\x2D    100  ", LCM_DISPLAY_COL);
	lcm_text_buffer_cpy(LCM_SEL_UNIT_2_DISPLAY,1,0,    "RA:            \xf4", LCM_DISPLAY_COL);

	lcm_text_buffer_cpy(LCM_SEL_UNIT_3_DISPLAY,0,0,    "UNIT:\x2B\x2D   1000  ", LCM_DISPLAY_COL);
	lcm_text_buffer_cpy(LCM_SEL_UNIT_3_DISPLAY,1,0,    "RA:            \xf4", LCM_DISPLAY_COL);

	lcm_text_buffer_cpy(LCM_SEL_UNIT_4_DISPLAY,0,0,    "UNIT:\x2B\x2D  10000  ", LCM_DISPLAY_COL);
	lcm_text_buffer_cpy(LCM_SEL_UNIT_4_DISPLAY,1,0,    "RA:            \xf4", LCM_DISPLAY_COL);

	lcm_text_buffer_cpy(LCM_SEL_UNIT_5_DISPLAY,0,0,    "UNIT:\x2B\x2D 100000  ", LCM_DISPLAY_COL);
	lcm_text_buffer_cpy(LCM_SEL_UNIT_5_DISPLAY,1,0,    "RA:            \xf4", LCM_DISPLAY_COL);

	// enable/disable some page/
	memset((void *)lcd_module_display_enable, 0x00, LCM_MAX_PAGE_NO);	// Initial only - later sw determine which page is to be displayed

}
///
///
///

Button_Data const button_data[5] =
{
		{ BUTTON_SRC_PORT, BUTTON_SRC_PIN, BUTTON_SRC_STATE_IN_10MS, TIMER_10MS, BUTTON_SRC_REPEAT_IN_10MS, TIMER_10MS},
		{ BUTTON_DEC_PORT, BUTTON_DEC_PIN, BUTTON_DEC_STATE_IN_10MS, TIMER_10MS, BUTTON_DEC_REPEAT_IN_10MS, TIMER_10MS},
		{ BUTTON_INC_PORT, BUTTON_INC_PIN, BUTTON_INC_STATE_IN_10MS, TIMER_10MS, BUTTON_INC_REPEAT_IN_10MS, TIMER_10MS},
		{ BUTTON_SEL_PORT, BUTTON_SEL_PIN, BUTTON_SEL_STATE_IN_10MS, TIMER_10MS, BUTTON_SEL_REPEAT_IN_10MS, TIMER_10MS},
		{ BUTTON_ISP_PORT, BUTTON_ISP_PIN, BUTTON_ISP_STATE_IN_10MS, TIMER_10MS, BUTTON_ISP_REPEAT_IN_10MS, TIMER_10MS}
};

BUTTON_STATE button_state[4] = { BUTTON_UP_STATE, BUTTON_UP_STATE, BUTTON_UP_STATE, BUTTON_UP_STATE };

uint8_t Get_Button_IO_Value(void)
{
	uint8_t	ret_value=0;

	if(!Chip_GPIO_GetPinState(LPC_GPIO, BUTTON_SRC_PORT, BUTTON_SRC_PIN))	// SRC -- BLUE
		ret_value =(1<<3);
	if(!Chip_GPIO_GetPinState(LPC_GPIO, BUTTON_SEL_PORT, BUTTON_SEL_PIN))	// SEL -- GREEN
		ret_value|=(1<<2);
	if(!Chip_GPIO_GetPinState(LPC_GPIO, BUTTON_INC_PORT, BUTTON_INC_PIN))	// INC -- YELLOW
		ret_value|=(1<<1);
	if(!Chip_GPIO_GetPinState(LPC_GPIO, BUTTON_DEC_PORT, BUTTON_DEC_PIN))	// DEC -- RED
		ret_value|=(1<<0);

	return ret_value;

}

uint32_t next_state_10ms_lut[] =
{
		(4),	// BUTTON_UP_STATE = 0,					time before end of debounce == debounce-time
		(120), // BUTTON_DOWN_DEBOUNCE_STATE,			time before end of stepwise == no repeat during this period
		(360), // BUTTON_DOWN_STEPWISE_STATE,			time before end of accelerating == +1 fast
		(~0U), // BUTTON_ACCELERATING_STATE,			not used
		(~0U), // BUTTON_TURBO_STATE,					not used
		(4-1), // BUTTON_UP_DEBOUNCE_STATE,				time before end of up-debounce == start to detect next key pressed.
};

#define ACCLERATING_TICK	(12)		// unit: 10ms		+1 each 120ms
#define TURBO_TICK			(1)			// unit: 10ms		+1 each 10ms

bool State_Proc_Button(ButtonID button_index)
{
	Button_Data 	*button_ptr = (Button_Data *)button_data + button_index;
	BUTTON_STATE	state = button_state[button_index];
	bool			pin_down = !Chip_GPIO_GetPinState(LPC_GPIO, button_ptr->port, button_ptr->pin);
	TIMER_ID		state_timer = button_ptr->state_change_timer, repeat_timer = button_ptr->repeat_timer;
	TIMER_UNIT_ID	state_timer_unit = button_ptr->state_change_timer_unit, repeat_timer_unit = button_ptr->repeat_timer_unit;
	bool			value_change = false;

	switch(state)
	{
		case BUTTON_UP_STATE:
			// Key pressed to next state
			if(pin_down)
			{
				Clear_SW_TIMER_Reload_Flag(repeat_timer);
				Countdown_Once(state_timer, next_state_10ms_lut[state], state_timer_unit);
			    state = BUTTON_DOWN_DEBOUNCE_STATE;    	// falling edge debounce
			}
			break;
		case BUTTON_DOWN_DEBOUNCE_STATE:
			// Debounce timeout + key pressed to +1
			if(Read_and_Clear_SW_TIMER_Reload_Flag(state_timer))
			{
			    if(pin_down)
			    {
					Clear_SW_TIMER_Reload_Flag(repeat_timer);
					Countdown_Once(state_timer, next_state_10ms_lut[state], state_timer_unit);
			    	value_change = true;
		    		state = BUTTON_DOWN_STEPWISE_STATE;
			    }
			    else
			    {
			    	// treat as noise
		    		state = BUTTON_UP_STATE;
			    }
			}
			break;
		case BUTTON_DOWN_STEPWISE_STATE:
			// If key released back to up-debounce
		    if(!pin_down)
		    {
				Clear_SW_TIMER_Reload_Flag(repeat_timer);
				Countdown_Once(state_timer, next_state_10ms_lut[BUTTON_UP_DEBOUNCE_STATE], state_timer_unit);
	    		state = BUTTON_UP_DEBOUNCE_STATE;
		    }
		    else
		    {
		    	// timeout to accelerating state and +1
				if(Read_and_Clear_SW_TIMER_Reload_Flag(state_timer))
				{
					Countdown_Once(repeat_timer,ACCLERATING_TICK,repeat_timer_unit);
					Countdown_Once(state_timer, next_state_10ms_lut[state], state_timer_unit);
			    	value_change = true;
			    	state = BUTTON_ACCELERATING_STATE;
				}
		    }
			break;
		case BUTTON_ACCELERATING_STATE:
			// If key released back to up-debounce
		    if(!pin_down)
		    {
				Clear_SW_TIMER_Reload_Flag(repeat_timer);
				Countdown_Once(state_timer, next_state_10ms_lut[BUTTON_UP_DEBOUNCE_STATE], state_timer_unit);
	    		state = BUTTON_UP_DEBOUNCE_STATE;
		    }
		    else
		    {
		    	// repeat timeout to +1
				if(Read_and_Clear_SW_TIMER_Reload_Flag(repeat_timer))
				{
					Countdown_Once(repeat_timer,ACCLERATING_TICK,repeat_timer_unit);
			    	value_change = true;
				}
		    	// timeout to turbo state and +1
				if(Read_and_Clear_SW_TIMER_Reload_Flag(state_timer))
				{
					Countdown_Once(repeat_timer,TURBO_TICK,repeat_timer_unit);
			    	value_change = true;
			    	state = BUTTON_TURBO_STATE;
				}
		    }
			break;
		case BUTTON_TURBO_STATE:
			// If key released back to up-debounce
		    if(!pin_down)
		    {
				Clear_SW_TIMER_Reload_Flag(repeat_timer);
				Countdown_Once(state_timer, next_state_10ms_lut[BUTTON_UP_DEBOUNCE_STATE], state_timer_unit);
	    		state = BUTTON_UP_DEBOUNCE_STATE;
		    }
		    else
		    {
		    	// repeat timeout to +1
				if(Read_and_Clear_SW_TIMER_Reload_Flag(repeat_timer))
				{
					Countdown_Once(repeat_timer,TURBO_TICK,repeat_timer_unit);
			    	value_change = true;
				}
		    }
		    break;
		case BUTTON_UP_DEBOUNCE_STATE:
			if(Read_and_Clear_SW_TIMER_Reload_Flag(state_timer))
			{
				Clear_SW_TIMER_Reload_Flag(state_timer);
				state = BUTTON_UP_STATE;
			}
			break;
		default:
			// Shouldn't be here
			Clear_SW_TIMER_Reload_Flag(repeat_timer);
			Clear_SW_TIMER_Reload_Flag(state_timer);
			state = BUTTON_UP_STATE;
			break;
	}

	button_state[button_index] = state;
	return value_change;
}

int Show_Resistor_3_Digits(uint32_t value, char* result)
{
    int return_value;

    if (value<(1000))			// direct output value
    {
    	itoa_10_fixed_position(value, result, 5);   // 1-999 ==> xxx
    	result[5] = '\0';
    	return_value = 5;
    }
    else if (value<(10000))
    {
    	// rounding and remove latest digit (so only 3 digit left)
    	value = (value + (10/2)) / (10);
    	itoa_10_fixed_position(value, result, 3);
    	result[5] = '\0';
    	result[4] = 'K';
    	result[3] = result[2];
    	result[2] = result[1];
    	result[1] = '.';
    	return_value = 5;
    }
    else if (value<(100000))
    {
    	// rounding and remove latest-2 digit (so only 3 digit left)
    	value = (value + (100/2)) / (100);
    	itoa_10_fixed_position(value, result, 3);
    	result[5] = '\0';
    	result[4] = 'K';
    	result[3] = result[2];
    	result[2] = '.';
    	return_value = 5;
    }
    else if (value<(1000000))
    {
    	// rounding and remove latest-3 digit (so only 3 digit left)
    	value = (value + (1000/2)) / (1000);
    	itoa_10_fixed_position(value, result, 4);
       	result[5] = '\0';
        result[4] = 'K';
    	return_value = 5;
    }
    else if (value<(1UL<<20))
    {
    	// rounding and remove latest-4 digit (so only 3 digit left)
    	value = (value + (10000/2)) / (10000);
    	itoa_10_fixed_position(value, result, 3);
    	result[5] = '\0';
    	result[4] = 'M';
    	result[3] = result[2];
    	result[2] = result[1];
    	result[1] = '.';
    	return_value = 5;
    }
    else
    {
    	result[0] = '\0';
		return_value = 0;
    }

    return return_value;
}

uint32_t Update_Resistor_Value_after_button(uint32_t previous_value, uint32_t step, bool inc)
{
	int change_value, temp_value;

	if (previous_value<(1000))
    {
		change_value = 1;
    }
    else if (previous_value<(10000))
    {
		change_value = 10;
    }
    else if (previous_value<(100000))
    {
		change_value = 100;
    }
    else if (previous_value<(1000000))
    {
		change_value = 1000;
    }
    else if (previous_value<(1UL<<20))
    {
		change_value = 10000;
    }
    else
    {
    	change_value = 0;
    }

	change_value *= step;
	if(inc==false)
	change_value = -change_value;

	temp_value = previous_value;
	temp_value += change_value;
	if(temp_value>=(1UL<<20))
	{
		temp_value = (1UL<<20)-1;
	}
	else if (temp_value<2)
	{
		temp_value = 2;
	}

	return (uint32_t) temp_value;
}

int Show_ADC_Voltage_3_Digits(uint32_t value, char* result)
{
    int return_value;

    if (value<(1000))
    {
    	// if input voltage <1 V
    	result[0] = '0';
    	result[1] = '.';
    	itoa_10_fixed_position(value, result+2, 3); // 3 digit are placed at position 2/3/4 and only pos 2/3 will be shown
    	if(result[3]==' ') {result[3]=result[2]='0';}
    	else if(result[2]==' ') {result[2]='0';}
    	result[4] = 'V';
    	result[5] = '\0';
    	return_value = 5;
    }
    else if (value<(10000))
    {
    	// rounding and remove latest digit (so only 3 digit left)
    	value = (value + (10/2)) / (10);
    	itoa_10_fixed_position(value, result, 3);
    	result[5] = '\0';
    	result[4] = 'V';
    	result[3] = result[2];
    	result[2] = result[1];
    	result[1] = '.';
    	return_value = 5;
    }
    else if (value<(100000))
    {
    	// rounding and remove latest-2 digit (so only 3 digit left)
    	value = (value + (100/2)) / (100);
    	itoa_10_fixed_position(value, result, 3);
    	result[5] = '\0';
    	result[4] = 'V';
    	result[3] = result[2];
    	result[2] = '.';
    	return_value = 5;
    }
    else if (value<(10000000))
    {
    	// 100 - 9999
    	value = (value + (1000/2)) / (1000);
    	itoa_10_fixed_position(value, result, 4);
    	result[5] = '\0';
    	result[4] = 'V';
    	return_value = 5;
    }
    else
    {
    	result[0] = '9';
    	result[1] = '9';
    	result[2] = '9';
    	result[3] = '9';
    	result[4] = 'V';
    	result[5] = '\0';
    	return_value = 5;
    }

    return return_value;
}

void UI_Version_01(void)
{
	char 				temp_text[10];
	int 				temp_len;
	static uint32_t		res_value[3] = { 1, 1, 1 }, res_step[3] = { 1, 1, 1 };
	static uint8_t		res_index_01 = 0;

	if(	State_Proc_Button(BUTTON_INC_ID) )
	{
		uint32_t	*res_ptr = res_value + res_index_01,
					*step_ptr = res_step + res_index_01;

		lcd_module_display_enable_only_one_page(LCM_SINGLE_VR_DISPLAY);

		*res_ptr = Update_Resistor_Value_after_button(*res_ptr,*step_ptr, true);

		temp_len = Show_Resistor_3_Digits(*res_ptr,temp_text);
		lcm_text_buffer_cpy(LCM_SINGLE_VR_DISPLAY,0,3,temp_text,temp_len);
	}

	if(	State_Proc_Button(BUTTON_DEC_ID) )
	{
		uint32_t	*res_ptr = res_value + res_index_01,
					*step_ptr = res_step + res_index_01;

		lcd_module_display_enable_only_one_page(LCM_SINGLE_VR_DISPLAY);

		*res_ptr = Update_Resistor_Value_after_button(*res_ptr,*step_ptr,false);

		temp_len = Show_Resistor_3_Digits(*res_ptr,temp_text);
		lcm_text_buffer_cpy(LCM_SINGLE_VR_DISPLAY,0,3,temp_text,temp_len);
	}

	if(	State_Proc_Button(BUTTON_SEL_ID) )
	{
		// to be implemented
	}

	if(	State_Proc_Button(BUTTON_SRC_ID) )
	{
		uint32_t	*res_ptr;
		if(++res_index_01>=3)
		{
			res_index_01 = 0;
		}
		temp_text[0] = '0'+ res_index;
		lcm_text_buffer_cpy(LCM_SINGLE_VR_DISPLAY,0,1,temp_text,1);
		res_ptr = res_value + res_index_01;
		temp_len = Show_Resistor_3_Digits(*res_ptr,temp_text);
		lcm_text_buffer_cpy(LCM_SINGLE_VR_DISPLAY,0,3,temp_text,temp_len);
	}

	if(	State_Proc_Button(BUTTON_ISP_ID) )
	{
		// Reserved for debug purpose
	}
}

//
//	lcm_page_change_duration_in_sec = DEFAULT_LCM_PAGE_CHANGE_S_WELCOME;
// lcd_module_display_enable_page(LCM_WELCOME_PAGE);

void UI_V2_Update_after_change(uint8_t res_id, uint32_t new_value, char* temp_text, int temp_len )
{
	switch (res_id)
	{
		case 0:
			lcm_text_buffer_cpy(LCM_ALL_VR_DISPLAY  ,   0,9+2,temp_text,temp_len);
			lcm_text_buffer_cpy(LCM_ALL_SET_BLINKING,   0,9+2,temp_text,temp_len);
			lcm_text_buffer_cpy(LCM_ALL_SET_R1_BLINKING,0,9+2,temp_text,temp_len);
			lcm_text_buffer_cpy(LCM_ALL_SET_R2_BLINKING,0,9+2,temp_text,temp_len);
			break;
		case 1:
			lcm_text_buffer_cpy(LCM_ALL_VR_DISPLAY  ,   1,2,temp_text,temp_len);
			lcm_text_buffer_cpy(LCM_ALL_SET_R0_BLINKING,1,2,temp_text,temp_len);
			lcm_text_buffer_cpy(LCM_ALL_SET_BLINKING,   1,2,temp_text,temp_len);
			lcm_text_buffer_cpy(LCM_ALL_SET_R2_BLINKING,1,2,temp_text,temp_len);
			break;
		case 2:
			lcm_text_buffer_cpy(LCM_ALL_VR_DISPLAY  ,   1,9+2,temp_text,temp_len);
			lcm_text_buffer_cpy(LCM_ALL_SET_R0_BLINKING,1,9+2,temp_text,temp_len);
			lcm_text_buffer_cpy(LCM_ALL_SET_R1_BLINKING,1,9+2,temp_text,temp_len);
			lcm_text_buffer_cpy(LCM_ALL_SET_BLINKING,   1,9+2,temp_text,temp_len);
			break;
/*		case 5:
			lcm_text_buffer_cpy(LCM_ALL_SEL_SET_DISPLAY,1,6,temp_text,temp_len);
			lcm_text_buffer_cpy(LCM_SEL_UNIT_1_DISPLAY,1,6,temp_text,temp_len);
			lcm_text_buffer_cpy(LCM_SEL_UNIT_2_DISPLAY,1,6,temp_text,temp_len);
			break;
		case 6:
			lcm_text_buffer_cpy(LCM_SEL_SET_R0_BLINKING,1,6,temp_text,temp_len);
			lcm_text_buffer_cpy(LCM_ALL_SEL_SET_BLINKING,1,6,temp_text,temp_len);
			lcm_text_buffer_cpy(LCM_SEL_SET_R2_BLINKING,1,6,temp_text,temp_len);
			//lcm_text_buffer_cpy(LCM_ALL_SET_BLINKING,   1,2,temp_text,temp_len);
			break;
		case 7:
			lcm_text_buffer_cpy(LCM_SEL_SET_R0_BLINKING,1,6,temp_text,temp_len);
			lcm_text_buffer_cpy(LCM_SEL_SET_R1_BLINKING,1,6,temp_text,temp_len);
			lcm_text_buffer_cpy(LCM_ALL_SEL_SET_BLINKING,1,6,temp_text,temp_len);
			//lcm_text_buffer_cpy(LCM_ALL_SET_BLINKING,   1,9+2,temp_text,temp_len);
			break;*/
	}
}

// LCM_ALL_SET_2N_VALUE

#define RES_INDEX_NO	(4)
#define RES_SEL_INDEX_NO	(6)

static uint32_t		res_value[3] = { 1, 1, 1 };
static uint32_t		res_2_power_N = 0;
bool	Value_has_been_changed;

uint32_t *GetResistorValue(void)
{
	return res_value;
}

uint32_t *Get_2PowerN_Value(void)
{
	return &res_2_power_N;
}

void UI_Version_02(void)
{
	char 				temp_text[10];
	int 				temp_len;
	int                 Unit1=10;
	int                 Unit2=100;
	int                 Unit3=1000;
	int                 Unit4=10000;
	int                 Unit5=100000;
	static uint32_t		res_step[3] = { 1, 1, 1 };

	if(	State_Proc_Button(BUTTON_SRC_ID) )
	{
		if(res_index<=3)
		{
			if(++res_index>3)
			{
				res_index = 0;
			}
			switch(res_index)			// next state after button pressed
			{
				case 0:			// changing RA under all VR menu
					temp_len = Show_Resistor_3_Digits(res_value[0],temp_text);
					UI_V2_Update_after_change(0,res_value[0],temp_text,temp_len); //above 2 line for update value from step to press source
					lcm_text_buffer_cpy(LCM_ALL_SET_BLINKING,0,5,"A",1);
					lcd_module_display_enable_only_one_page(LCM_ALL_SET_BLINKING);
					lcd_module_display_enable_page(LCM_ALL_SET_R0_BLINKING);
					break;
				case 1:			// changing RB under all VR menu
					temp_len = Show_Resistor_3_Digits(res_value[1],temp_text);
					UI_V2_Update_after_change(1,res_value[1],temp_text,temp_len);
					lcm_text_buffer_cpy(LCM_ALL_SET_BLINKING,0,5,"B",1);
					lcd_module_display_enable_only_one_page(LCM_ALL_SET_BLINKING);
					lcd_module_display_enable_page(LCM_ALL_SET_R1_BLINKING);
					break;
				case 2:			// changing RC under all VR menu
					temp_len = Show_Resistor_3_Digits(res_value[2],temp_text);
					UI_V2_Update_after_change(2,res_value[2],temp_text,temp_len);
					lcm_text_buffer_cpy(LCM_ALL_SET_BLINKING,0,5,"C",1);
					lcd_module_display_enable_only_one_page(LCM_ALL_SET_BLINKING);
					lcd_module_display_enable_page(LCM_ALL_SET_R2_BLINKING);
					break;
				case 3:			// Value-R menu
					temp_len = Show_Resistor_3_Digits(res_value[0],temp_text);
					UI_V2_Update_after_change(0,res_value[0],temp_text,temp_len);
					temp_len = Show_Resistor_3_Digits(res_value[1],temp_text);
					UI_V2_Update_after_change(1,res_value[1],temp_text,temp_len);
					temp_len = Show_Resistor_3_Digits(res_value[2],temp_text);
					UI_V2_Update_after_change(2,res_value[2],temp_text,temp_len);
					lcd_module_display_enable_only_one_page(LCM_ALL_VR_DISPLAY);
					break;
				case 4:			// Value-R menu
					lcd_module_display_enable_only_one_page(LCM_ALL_SET_2N_VALUE);
					break;
			}
		}
		else if (res_index<=22)
		{
			switch(res_index)			// next state after button pressed
			{
				case 5:			// changing RA under UNIT menu
					res_index=0;
					temp_len = Show_Resistor_3_Digits(res_value[0],temp_text);
					UI_V2_Update_after_change(0,res_value[0],temp_text,temp_len);//above 2 line for update value from step to press source
					lcm_text_buffer_cpy(LCM_ALL_SET_BLINKING,0,5,"A",1);
					lcd_module_display_enable_only_one_page(LCM_ALL_SET_BLINKING);
					lcd_module_display_enable_page(LCM_ALL_SET_R0_BLINKING);
					break;
				case 6:			// changing RB under UNIT menu
					res_index=1;
					temp_len = Show_Resistor_3_Digits(res_value[1],temp_text);
					UI_V2_Update_after_change(1,res_value[1],temp_text,temp_len);
					lcm_text_buffer_cpy(LCM_ALL_SET_BLINKING,0,5,"B",1);
					lcd_module_display_enable_only_one_page(LCM_ALL_SET_BLINKING);
					lcd_module_display_enable_page(LCM_ALL_SET_R1_BLINKING);
					break;
				case 7:			// changing RC under UNIT menu
					res_index=2;
					temp_len = Show_Resistor_3_Digits(res_value[2],temp_text);
					UI_V2_Update_after_change(2,res_value[2],temp_text,temp_len);
					lcm_text_buffer_cpy(LCM_ALL_SET_BLINKING,0,5,"C",1);
					lcd_module_display_enable_only_one_page(LCM_ALL_SET_BLINKING);
					lcd_module_display_enable_page(LCM_ALL_SET_R2_BLINKING);
					break;
				case 8:			// changing RA under UNIT menu
					res_index=0;
					temp_len = Show_Resistor_3_Digits(res_value[0],temp_text);
					UI_V2_Update_after_change(0,res_value[0],temp_text,temp_len);
					lcm_text_buffer_cpy(LCM_ALL_SET_BLINKING,0,5,"A",1);
					lcd_module_display_enable_only_one_page(LCM_ALL_SET_BLINKING);
					lcd_module_display_enable_page(LCM_ALL_SET_R0_BLINKING);
					break;
				case 9:			// changing RA under UNIT menu
					res_index=0;
					temp_len = Show_Resistor_3_Digits(res_value[0],temp_text);
					UI_V2_Update_after_change(0,res_value[0],temp_text,temp_len);
					lcm_text_buffer_cpy(LCM_ALL_SET_BLINKING,0,5,"A",1);
					lcd_module_display_enable_only_one_page(LCM_ALL_SET_BLINKING);
					lcd_module_display_enable_page(LCM_ALL_SET_R0_BLINKING);
					break;
				case 10:			// changing RA under UNIT menu
					res_index=0;
					temp_len = Show_Resistor_3_Digits(res_value[0],temp_text);
					UI_V2_Update_after_change(0,res_value[0],temp_text,temp_len);
					lcm_text_buffer_cpy(LCM_ALL_SET_BLINKING,0,5,"A",1);
					lcd_module_display_enable_only_one_page(LCM_ALL_SET_BLINKING);
					lcd_module_display_enable_page(LCM_ALL_SET_R0_BLINKING);
					break;
				case 11:			// changing RA under UNIT menu
					res_index=0;
					temp_len = Show_Resistor_3_Digits(res_value[0],temp_text);
					UI_V2_Update_after_change(0,res_value[0],temp_text,temp_len);
					lcm_text_buffer_cpy(LCM_ALL_SET_BLINKING,0,5,"A",1);
					lcd_module_display_enable_only_one_page(LCM_ALL_SET_BLINKING);
					lcd_module_display_enable_page(LCM_ALL_SET_R0_BLINKING);
					break;
				case 12:			// changing RA under UNIT menu
					res_index=0;
					temp_len = Show_Resistor_3_Digits(res_value[0],temp_text);
					UI_V2_Update_after_change(0,res_value[0],temp_text,temp_len);
					lcm_text_buffer_cpy(LCM_ALL_SET_BLINKING,0,5,"A",1);
					lcd_module_display_enable_only_one_page(LCM_ALL_SET_BLINKING);
					lcd_module_display_enable_page(LCM_ALL_SET_R0_BLINKING);
					break;
				case 13:			// changing RB under UNIT menu
					res_index=1;
					temp_len = Show_Resistor_3_Digits(res_value[1],temp_text);
					UI_V2_Update_after_change(1,res_value[1],temp_text,temp_len);
					lcm_text_buffer_cpy(LCM_ALL_SET_BLINKING,0,5,"B",1);
					lcd_module_display_enable_only_one_page(LCM_ALL_SET_BLINKING);
					lcd_module_display_enable_page(LCM_ALL_SET_R1_BLINKING);
					break;
				case 14:			// changing RB under UNIT menu
					res_index=1;
					temp_len = Show_Resistor_3_Digits(res_value[1],temp_text);
					UI_V2_Update_after_change(1,res_value[1],temp_text,temp_len);
					lcm_text_buffer_cpy(LCM_ALL_SET_BLINKING,0,5,"B",1);
					lcd_module_display_enable_only_one_page(LCM_ALL_SET_BLINKING);
					lcd_module_display_enable_page(LCM_ALL_SET_R1_BLINKING);
					break;
				case 15:			// changing RB under UNIT menu
					res_index=1;
					temp_len = Show_Resistor_3_Digits(res_value[1],temp_text);
					UI_V2_Update_after_change(1,res_value[1],temp_text,temp_len);
					lcm_text_buffer_cpy(LCM_ALL_SET_BLINKING,0,5,"B",1);
					lcd_module_display_enable_only_one_page(LCM_ALL_SET_BLINKING);
					lcd_module_display_enable_page(LCM_ALL_SET_R1_BLINKING);
					break;
				case 16:			// changing RB under UNIT menu
					res_index=1;
					temp_len = Show_Resistor_3_Digits(res_value[1],temp_text);
					UI_V2_Update_after_change(1,res_value[1],temp_text,temp_len);
					lcm_text_buffer_cpy(LCM_ALL_SET_BLINKING,0,5,"B",1);
					lcd_module_display_enable_only_one_page(LCM_ALL_SET_BLINKING);
					lcd_module_display_enable_page(LCM_ALL_SET_R1_BLINKING);
					break;
				case 17:			// changing RB under UNIT menu
					res_index=1;
					temp_len = Show_Resistor_3_Digits(res_value[1],temp_text);
					UI_V2_Update_after_change(1,res_value[1],temp_text,temp_len);
					lcm_text_buffer_cpy(LCM_ALL_SET_BLINKING,0,5,"B",1);
					lcd_module_display_enable_only_one_page(LCM_ALL_SET_BLINKING);
					lcd_module_display_enable_page(LCM_ALL_SET_R1_BLINKING);
					break;
				case 18:			// changing RC under UNIT menu
					res_index=2;
					temp_len = Show_Resistor_3_Digits(res_value[2],temp_text);
					UI_V2_Update_after_change(2,res_value[2],temp_text,temp_len);
					lcm_text_buffer_cpy(LCM_ALL_SET_BLINKING,0,5,"C",1);
					lcd_module_display_enable_only_one_page(LCM_ALL_SET_BLINKING);
					lcd_module_display_enable_page(LCM_ALL_SET_R2_BLINKING);
					break;
				case 19:			// changing RC under UNIT menu
					res_index=2;
					temp_len = Show_Resistor_3_Digits(res_value[2],temp_text);
					UI_V2_Update_after_change(2,res_value[2],temp_text,temp_len);
					lcm_text_buffer_cpy(LCM_ALL_SET_BLINKING,0,5,"C",1);
					lcd_module_display_enable_only_one_page(LCM_ALL_SET_BLINKING);
					lcd_module_display_enable_page(LCM_ALL_SET_R2_BLINKING);
					break;
				case 20:			// changing RC under UNIT menu
					res_index=2;
					temp_len = Show_Resistor_3_Digits(res_value[2],temp_text);
					UI_V2_Update_after_change(2,res_value[2],temp_text,temp_len);
					lcm_text_buffer_cpy(LCM_ALL_SET_BLINKING,0,5,"C",1);
					lcd_module_display_enable_only_one_page(LCM_ALL_SET_BLINKING);
					lcd_module_display_enable_page(LCM_ALL_SET_R2_BLINKING);
					break;
				case 21:			// changing RC under UNIT menu
					res_index=2;
					temp_len = Show_Resistor_3_Digits(res_value[2],temp_text);
					UI_V2_Update_after_change(2,res_value[2],temp_text,temp_len);
					lcm_text_buffer_cpy(LCM_ALL_SET_BLINKING,0,5,"C",1);
					lcd_module_display_enable_only_one_page(LCM_ALL_SET_BLINKING);
					lcd_module_display_enable_page(LCM_ALL_SET_R2_BLINKING);
					break;
				case 22:			// changing RC under UNIT menu
					res_index=2;
					temp_len = Show_Resistor_3_Digits(res_value[2],temp_text);
					UI_V2_Update_after_change(2,res_value[2],temp_text,temp_len);
					lcm_text_buffer_cpy(LCM_ALL_SET_BLINKING,0,5,"C",1);
					lcd_module_display_enable_only_one_page(LCM_ALL_SET_BLINKING);
					lcd_module_display_enable_page(LCM_ALL_SET_R2_BLINKING);
					break;
			}

		}
	}

	if(	State_Proc_Button(BUTTON_SEL_ID) )
	{
		switch(res_index)			// next state after button pressed
		{
			case 0:			// changing Fine-tuning RA
				lcm_text_buffer_cpy(LCM_ALL_SEL_SET_DISPLAY,1,1,"A",1);
				temp_len = itoa_10_fixed_position(res_value[0],temp_text,8);
				UI_V2_Update_after_change(res_index,res_value[0],temp_text,5); // above 2 line for update value without pressing +-
				lcd_module_display_enable_only_one_page(LCM_ALL_SEL_SET_DISPLAY);
				lcm_text_buffer_cpy(LCM_ALL_SEL_SET_DISPLAY,1,6,temp_text,temp_len);
				res_index=5;
				break;
			case 1:			// changing Fine-tuning RB
				lcm_text_buffer_cpy(LCM_ALL_SEL_SET_DISPLAY,1,1,"B",1);
				temp_len = itoa_10_fixed_position(res_value[1],temp_text,8);
				UI_V2_Update_after_change(res_index,res_value[1],temp_text,5);// above 2 line for update value without pressing +-
				lcd_module_display_enable_only_one_page(LCM_ALL_SEL_SET_DISPLAY);
				lcm_text_buffer_cpy(LCM_ALL_SEL_SET_DISPLAY,1,6,temp_text,temp_len);
				res_index=6;
				break;
			case 2:			// changing Fine-tuning RC
				lcm_text_buffer_cpy(LCM_ALL_SEL_SET_DISPLAY,1,1,"C",1);
				temp_len = itoa_10_fixed_position(res_value[2],temp_text,8);
				UI_V2_Update_after_change(res_index,res_value[2],temp_text,5);// above 2 line for update value without pressing +-
				lcd_module_display_enable_only_one_page(LCM_ALL_SEL_SET_DISPLAY);
				lcm_text_buffer_cpy(LCM_ALL_SEL_SET_DISPLAY,1,6,temp_text,temp_len);
				res_index=7;
				break;
			case 3: // Mode1_change to RA on VR menu
				temp_len = Show_Resistor_3_Digits(res_value[0],temp_text);
				UI_V2_Update_after_change(0,res_value[0],temp_text,temp_len);
				lcm_text_buffer_cpy(LCM_ALL_SET_BLINKING,0,5,"A",1);
				lcd_module_display_enable_only_one_page(LCM_ALL_SET_BLINKING);
				lcd_module_display_enable_page(LCM_ALL_SET_R0_BLINKING);
				res_index=0;
				break;
			case 5:
				lcm_text_buffer_cpy(LCM_SEL_UNIT_1_DISPLAY,1,1,"A",1);
				temp_len = itoa_10_fixed_position(res_value[res_index-5],temp_text,8);
				UI_V2_Update_after_change(res_index,res_value[res_index-5],temp_text,temp_len);
				lcm_text_buffer_cpy(LCM_SEL_UNIT_1_DISPLAY,1,6,temp_text,temp_len);
				lcd_module_display_enable_only_one_page(LCM_SEL_UNIT_1_DISPLAY);
				res_index=8;
				break;
			case 6:
				lcm_text_buffer_cpy(LCM_SEL_UNIT_1_DISPLAY,1,1,"B",1);
				temp_len = itoa_10_fixed_position(res_value[res_index-5],temp_text,8);
				UI_V2_Update_after_change(res_index,res_value[res_index-5],temp_text,temp_len);
				lcm_text_buffer_cpy(LCM_SEL_UNIT_1_DISPLAY,1,6,temp_text,temp_len);
				lcd_module_display_enable_only_one_page(LCM_SEL_UNIT_1_DISPLAY);
				res_index=13;
				break;
			case 7:
				lcm_text_buffer_cpy(LCM_SEL_UNIT_1_DISPLAY,1,1,"C",1);
				temp_len = itoa_10_fixed_position(res_value[res_index-5],temp_text,8);
				UI_V2_Update_after_change(res_index,res_value[res_index-5],temp_text,temp_len);
				lcm_text_buffer_cpy(LCM_SEL_UNIT_1_DISPLAY,1,6,temp_text,temp_len);
				lcd_module_display_enable_only_one_page(LCM_SEL_UNIT_1_DISPLAY);
				res_index=18;
				break;

			case 8: //case 8 to case 12 change RA UNIT
				lcm_text_buffer_cpy(LCM_SEL_UNIT_2_DISPLAY,1,1,"A",1);
				temp_len = itoa_10_fixed_position(res_value[res_index-8],temp_text,8);
				UI_V2_Update_after_change(res_index,res_value[res_index-8],temp_text,temp_len);
				lcm_text_buffer_cpy(LCM_SEL_UNIT_2_DISPLAY,1,6,temp_text,temp_len);
				lcd_module_display_enable_only_one_page(LCM_SEL_UNIT_2_DISPLAY);
				res_index=9;
				break;
			case 9:
				lcm_text_buffer_cpy(LCM_SEL_UNIT_3_DISPLAY,1,1,"A",1);
				temp_len = itoa_10_fixed_position(res_value[res_index-9],temp_text,8);
				UI_V2_Update_after_change(res_index,res_value[res_index-9],temp_text,temp_len);
				lcm_text_buffer_cpy(LCM_SEL_UNIT_3_DISPLAY,1,6,temp_text,temp_len);
				lcd_module_display_enable_only_one_page(LCM_SEL_UNIT_3_DISPLAY);
				res_index=10;
				break;
			case 10:
				lcm_text_buffer_cpy(LCM_SEL_UNIT_4_DISPLAY,1,1,"A",1);
				temp_len = itoa_10_fixed_position(res_value[res_index-10],temp_text,8);
				UI_V2_Update_after_change(res_index,res_value[res_index-10],temp_text,temp_len);
				lcm_text_buffer_cpy(LCM_SEL_UNIT_4_DISPLAY,1,6,temp_text,temp_len);
				lcd_module_display_enable_only_one_page(LCM_SEL_UNIT_4_DISPLAY);
				res_index=11;
				break;
			case 11:
				lcm_text_buffer_cpy(LCM_SEL_UNIT_5_DISPLAY,1,1,"A",1);
				temp_len = itoa_10_fixed_position(res_value[res_index-11],temp_text,8);
				UI_V2_Update_after_change(res_index,res_value[res_index-11],temp_text,temp_len);
				lcm_text_buffer_cpy(LCM_SEL_UNIT_5_DISPLAY,1,6,temp_text,temp_len);
				lcd_module_display_enable_only_one_page(LCM_SEL_UNIT_5_DISPLAY);
				res_index=12;
				break;
			case 12:
				lcm_text_buffer_cpy(LCM_ALL_SEL_SET_DISPLAY,1,1,"A",1);
				temp_len = itoa_10_fixed_position(res_value[res_index-12],temp_text,8);
				UI_V2_Update_after_change(res_index,res_value[res_index-12],temp_text,temp_len);
				lcm_text_buffer_cpy(LCM_ALL_SEL_SET_DISPLAY,1,6,temp_text,temp_len);
				lcd_module_display_enable_only_one_page(LCM_ALL_SEL_SET_DISPLAY);
				res_index=5;
				break;

			case 13://case 13 to case 17 change RB UNIT
				lcm_text_buffer_cpy(LCM_SEL_UNIT_2_DISPLAY,1,1,"B",1);
				temp_len = itoa_10_fixed_position(res_value[res_index-12],temp_text,8);
				UI_V2_Update_after_change(res_index,res_value[res_index-12],temp_text,temp_len);
				lcm_text_buffer_cpy(LCM_SEL_UNIT_2_DISPLAY,1,6,temp_text,temp_len);
				lcd_module_display_enable_only_one_page(LCM_SEL_UNIT_2_DISPLAY);
				res_index=14;
				break;
			case 14:
				lcm_text_buffer_cpy(LCM_SEL_UNIT_3_DISPLAY,1,1,"B",1);
				temp_len = itoa_10_fixed_position(res_value[res_index-13],temp_text,8);
				UI_V2_Update_after_change(res_index,res_value[res_index-13],temp_text,temp_len);
				lcm_text_buffer_cpy(LCM_SEL_UNIT_3_DISPLAY,1,6,temp_text,temp_len);
				lcd_module_display_enable_only_one_page(LCM_SEL_UNIT_3_DISPLAY);
				res_index=15;
				break;
			case 15:
				lcm_text_buffer_cpy(LCM_SEL_UNIT_4_DISPLAY,1,1,"B",1);
				temp_len = itoa_10_fixed_position(res_value[res_index-14],temp_text,8);
				UI_V2_Update_after_change(res_index,res_value[res_index-14],temp_text,temp_len);
				lcm_text_buffer_cpy(LCM_SEL_UNIT_4_DISPLAY,1,6,temp_text,temp_len);
				lcd_module_display_enable_only_one_page(LCM_SEL_UNIT_4_DISPLAY);
				res_index=16;
				break;
			case 16:
				lcm_text_buffer_cpy(LCM_SEL_UNIT_5_DISPLAY,1,1,"B",1);
				temp_len = itoa_10_fixed_position(res_value[res_index-15],temp_text,8);
				UI_V2_Update_after_change(res_index,res_value[res_index-15],temp_text,temp_len);
				lcm_text_buffer_cpy(LCM_SEL_UNIT_5_DISPLAY,1,6,temp_text,temp_len);
				lcd_module_display_enable_only_one_page(LCM_SEL_UNIT_5_DISPLAY);
				res_index=17;
				break;
			case 17:
				lcm_text_buffer_cpy(LCM_ALL_SEL_SET_DISPLAY,1,1,"B",1);
				temp_len = itoa_10_fixed_position(res_value[res_index-16],temp_text,8);
				UI_V2_Update_after_change(res_index,res_value[res_index-16],temp_text,temp_len);
				lcm_text_buffer_cpy(LCM_ALL_SEL_SET_DISPLAY,1,6,temp_text,temp_len);
				lcd_module_display_enable_only_one_page(LCM_ALL_SEL_SET_DISPLAY);
				res_index=6;
				break;

			case 18: //case 18 to case 22 change RC UNIT
				lcm_text_buffer_cpy(LCM_SEL_UNIT_2_DISPLAY,1,1,"C",1);
				temp_len = itoa_10_fixed_position(res_value[res_index-16],temp_text,8);
				UI_V2_Update_after_change(res_index,res_value[res_index-16],temp_text,temp_len);
				lcm_text_buffer_cpy(LCM_SEL_UNIT_2_DISPLAY,1,6,temp_text,temp_len);
				lcd_module_display_enable_only_one_page(LCM_SEL_UNIT_2_DISPLAY);
				res_index=19;
				break;
			case 19:
				lcm_text_buffer_cpy(LCM_SEL_UNIT_3_DISPLAY,1,1,"C",1);
				temp_len = itoa_10_fixed_position(res_value[res_index-17],temp_text,8);
				UI_V2_Update_after_change(res_index,res_value[res_index-17],temp_text,temp_len);
				lcm_text_buffer_cpy(LCM_SEL_UNIT_3_DISPLAY,1,6,temp_text,temp_len);
				lcd_module_display_enable_only_one_page(LCM_SEL_UNIT_3_DISPLAY);
				res_index=20;
				break;
			case 20:
				lcm_text_buffer_cpy(LCM_SEL_UNIT_4_DISPLAY,1,1,"C",1);
				temp_len = itoa_10_fixed_position(res_value[res_index-18],temp_text,8);
				UI_V2_Update_after_change(res_index,res_value[res_index-18],temp_text,temp_len);
				lcm_text_buffer_cpy(LCM_SEL_UNIT_4_DISPLAY,1,6,temp_text,temp_len);
				lcd_module_display_enable_only_one_page(LCM_SEL_UNIT_4_DISPLAY);
				res_index=21;
				break;
			case 21:
				lcm_text_buffer_cpy(LCM_SEL_UNIT_5_DISPLAY,1,1,"C",1);
				temp_len = itoa_10_fixed_position(res_value[res_index-19],temp_text,8);
				UI_V2_Update_after_change(res_index,res_value[res_index-19],temp_text,temp_len);
				lcm_text_buffer_cpy(LCM_SEL_UNIT_5_DISPLAY,1,6,temp_text,temp_len);
				lcd_module_display_enable_only_one_page(LCM_SEL_UNIT_5_DISPLAY);
				res_index=22;
				break;
			case 22:
				lcm_text_buffer_cpy(LCM_ALL_SEL_SET_DISPLAY,1,1,"C",1);
				temp_len = itoa_10_fixed_position(res_value[res_index-20],temp_text,8);
				UI_V2_Update_after_change(res_index,res_value[res_index-20],temp_text,temp_len);
				lcm_text_buffer_cpy(LCM_ALL_SEL_SET_DISPLAY,1,6,temp_text,temp_len);
				lcd_module_display_enable_only_one_page(LCM_ALL_SEL_SET_DISPLAY);
				res_index=7;
				break;
		}
	}
	// INC/DEC
	if(res_index<3)
	{
		// when res_index == 0 or 1 or 2
		if(	State_Proc_Button(BUTTON_INC_ID) )
		{
			uint32_t	*res_ptr = res_value + res_index,
						*step_ptr = res_step + res_index;

			*res_ptr = Update_Resistor_Value_after_button(*res_ptr,*step_ptr, true);
			temp_len = Show_Resistor_3_Digits(*res_ptr,temp_text);
			UI_V2_Update_after_change(res_index,*res_ptr,temp_text,temp_len);
			lcm_text_buffer_cpy(LCM_ALL_VR_DISPLAY,0,9+2,temp_text,temp_len);
			lcm_text_buffer_cpy(LCM_ALL_VR_DISPLAY,1,2,temp_text,temp_len);
			lcm_text_buffer_cpy(LCM_ALL_VR_DISPLAY,1,9+2,temp_text,temp_len);
			lcm_force_to_display_page(LCM_ALL_SET_BLINKING);
			// update fine-tune value
			temp_len = itoa_10_fixed_position(*res_ptr,temp_text,8);
			lcm_text_buffer_cpy(LCM_ALL_SEL_SET_DISPLAY,1,6,temp_text,temp_len);
			Value_has_been_changed=true;
		}
		if(	State_Proc_Button(BUTTON_DEC_ID) )
		{
			uint32_t	*res_ptr = res_value + res_index,
						*step_ptr = res_step + res_index;

			*res_ptr = Update_Resistor_Value_after_button(*res_ptr,*step_ptr,false);
			temp_len = Show_Resistor_3_Digits(*res_ptr,temp_text);
			UI_V2_Update_after_change(res_index,*res_ptr,temp_text,temp_len);
			lcm_text_buffer_cpy(LCM_ALL_VR_DISPLAY,0,9+2,temp_text,temp_len);
			lcm_text_buffer_cpy(LCM_ALL_VR_DISPLAY,1,2,temp_text,temp_len);
			lcm_text_buffer_cpy(LCM_ALL_VR_DISPLAY,1,9+2,temp_text,temp_len);
			lcm_force_to_display_page(LCM_ALL_SET_BLINKING);
			// update fine-tune value
			temp_len = itoa_10_fixed_position(*res_ptr,temp_text,8);
			lcm_text_buffer_cpy(LCM_ALL_SEL_SET_DISPLAY,1,6,temp_text,temp_len);
			Value_has_been_changed=true;
		}
	}

	if (res_index == 3) //Mode1_change to RA menu
	{
		if(	State_Proc_Button(BUTTON_INC_ID) )
		{
			temp_len = Show_Resistor_3_Digits(res_value[0],temp_text);
			UI_V2_Update_after_change(0,res_value[0],temp_text,temp_len);
			lcm_text_buffer_cpy(LCM_ALL_SET_BLINKING,0,5,"A",1);
			lcd_module_display_enable_only_one_page(LCM_ALL_SET_BLINKING);
			lcd_module_display_enable_page(LCM_ALL_SET_R0_BLINKING);
			res_index=0;
		}
		if(	State_Proc_Button(BUTTON_DEC_ID) )
		{
			temp_len = Show_Resistor_3_Digits(res_value[0],temp_text);
			UI_V2_Update_after_change(0,res_value[0],temp_text,temp_len);
			lcm_text_buffer_cpy(LCM_ALL_SET_BLINKING,0,5,"A",1);
			lcd_module_display_enable_only_one_page(LCM_ALL_SET_BLINKING);
			lcd_module_display_enable_page(LCM_ALL_SET_R0_BLINKING);
			res_index=0;
		}
	}

	if (res_index == 4)
	{
		char *update_str;

		if(	State_Proc_Button(BUTTON_INC_ID) )
		{
			++res_2_power_N;
			if(res_2_power_N>=20+2)
			{
				res_2_power_N = 0;
			}
			update_str = FineTuneResistorMsg[res_2_power_N*2];
			lcm_text_buffer_cpy(LCM_ALL_SET_2N_VALUE,   0,0, update_str, LCM_DISPLAY_COL);
			update_str = FineTuneResistorMsg[res_2_power_N*2+1];
			lcm_text_buffer_cpy(LCM_ALL_SET_2N_VALUE,   1,0, update_str, LCM_DISPLAY_COL);
			if(res_2_power_N<=19)
			{
				res_value[0] = res_value[1] = res_value[2] = 1UL<<(res_2_power_N);
			}
			else if(res_2_power_N==20)
			{
				res_value[0] = res_value[1] = res_value[2] = 0;
			}
			else
			{
				res_value[0] = res_value[1] = res_value[2] = ~0UL;
			}
			Value_has_been_changed=true;
		}
		if(	State_Proc_Button(BUTTON_DEC_ID) )
		{
			if(res_2_power_N==0)
			{
				res_2_power_N = (20-1)+2;
			}
			else
			{
				res_2_power_N--;
			}
			update_str = FineTuneResistorMsg[res_2_power_N*2];
			lcm_text_buffer_cpy(LCM_ALL_SET_2N_VALUE,   0,0, update_str, LCM_DISPLAY_COL);
			update_str = FineTuneResistorMsg[res_2_power_N*2+1];
			lcm_text_buffer_cpy(LCM_ALL_SET_2N_VALUE,   1,0, update_str, LCM_DISPLAY_COL);
			if(res_2_power_N<=19)
			{
				res_value[0] = res_value[1] = res_value[2] = 1UL<<(res_2_power_N);
			}
			else if(res_2_power_N==20)
			{
				res_value[0] = res_value[1] = res_value[2] = 0;
			}
			else
			{
				res_value[0] = res_value[1] = res_value[2] = ~0UL;
			}
			Value_has_been_changed=true;
		}
	}

	if(res_index==5 ||
	   res_index==6 ||
	   res_index==7)
	{
		if(	State_Proc_Button(BUTTON_INC_ID) )
		{
			if(res_value[res_index-5]<(1UL<<20))
			{
				res_value[res_index-5]++;
			}
			else
			{
				//do nothing
			}
			//*res_ptr = Update_Resistor_Value_after_button(*res_ptr,*step_ptr, true);
			temp_len = itoa_10_fixed_position(res_value[res_index-5],temp_text,8);
			UI_V2_Update_after_change(res_index,res_value[res_index-5],temp_text,temp_len);
			lcm_text_buffer_cpy(LCM_ALL_SEL_SET_DISPLAY,1,6,temp_text,temp_len);
			// update RA/RB/RC
			temp_len = Show_Resistor_3_Digits(res_value[res_index-5],temp_text);
			UI_V2_Update_after_change(res_index-5,res_value[res_index-5],temp_text,temp_len);
			Value_has_been_changed=true;
		}

		if(	State_Proc_Button(BUTTON_DEC_ID) )
		{
			//uint32_t	*res_ptr = res_value[res_index-5] + res_index-5,   // // res_value[res_index-5]
						//*step_ptr = res_step + res_index-5;

			if (res_value[res_index-5]>2)			// // res_value[res_index-5]>1  res_value[res_index-5]--
			{
				res_value[res_index-5]--;
			}
			else
			{
				res_value[res_index-5]=2;
			}

			//*res_ptr = Update_Resistor_Value_after_button(*res_ptr,*step_ptr, false);
			temp_len = itoa_10_fixed_position(res_value[res_index-5],temp_text,8);
			UI_V2_Update_after_change(res_index,res_value[res_index-5],temp_text,temp_len);
			lcm_text_buffer_cpy(LCM_ALL_SEL_SET_DISPLAY,1,6,temp_text,temp_len);
			// update RA/RB/RC
			temp_len = Show_Resistor_3_Digits(res_value[res_index-5],temp_text);
			UI_V2_Update_after_change(res_index-5,res_value[res_index-5],temp_text,temp_len);
			Value_has_been_changed=true;
		}
	}

	if(res_index==8)
	{
		if(	State_Proc_Button(BUTTON_INC_ID) )
		{
			if(res_value[res_index-8]<=1048564)
			{
				res_value[res_index-8]=res_value[res_index-8]+Unit1;
			}
			else
			{
				res_value[res_index-8]=1048575;
			}
			//*res_ptr = Update_Resistor_Value_after_button(*res_ptr,*step_ptr, true);
			temp_len = itoa_10_fixed_position(res_value[res_index-8],temp_text,8);
			UI_V2_Update_after_change(res_index,res_value[res_index-8],temp_text,temp_len);
			lcm_text_buffer_cpy(LCM_SEL_UNIT_1_DISPLAY,1,6,temp_text,temp_len);
			// update RA/RB/RC
			temp_len = Show_Resistor_3_Digits(res_value[res_index-8],temp_text);
			UI_V2_Update_after_change(res_index-8,res_value[res_index-8],temp_text,temp_len);
			Value_has_been_changed=true;
		}

		if(	State_Proc_Button(BUTTON_DEC_ID) )
		{
			//uint32_t	*res_ptr = res_value[res_index-5] + res_index-5,   // // res_value[res_index-5]
						//*step_ptr = res_step + res_index-5;

			if (res_value[res_index-8]>10)			// // res_value[res_index-5]>1  res_value[res_index-5]--
			{
				res_value[res_index-8]=res_value[res_index-8]-Unit1;
			}
			else
			{
				res_value[res_index-8]=2;
			}

			//*res_ptr = Update_Resistor_Value_after_button(*res_ptr,*step_ptr, false);
			temp_len = itoa_10_fixed_position(res_value[res_index-8],temp_text,8);
			UI_V2_Update_after_change(res_index,res_value[res_index-8],temp_text,temp_len);
			lcm_text_buffer_cpy(LCM_SEL_UNIT_1_DISPLAY,1,6,temp_text,temp_len);
			// update RA/RB/RC
			temp_len = Show_Resistor_3_Digits(res_value[res_index-8],temp_text);
			UI_V2_Update_after_change(res_index-8,res_value[res_index-8],temp_text,temp_len);
			Value_has_been_changed=true;
		}
	}
	if(res_index==9)
	{
		if(	State_Proc_Button(BUTTON_INC_ID) )
		{
			if(res_value[res_index-9]<=1048475)
			{
				res_value[res_index-9]=res_value[res_index-9]+Unit2;
			}
			else
			{
				res_value[res_index-9]=1048575;
			}
			//*res_ptr = Update_Resistor_Value_after_button(*res_ptr,*step_ptr, true);
			temp_len = itoa_10_fixed_position(res_value[res_index-9],temp_text,8);
			UI_V2_Update_after_change(res_index,res_value[res_index-9],temp_text,temp_len);
			lcm_text_buffer_cpy(LCM_SEL_UNIT_2_DISPLAY,1,6,temp_text,temp_len);
			// update RA/RB/RC
			temp_len = Show_Resistor_3_Digits(res_value[res_index-9],temp_text);
			UI_V2_Update_after_change(res_index-9,res_value[res_index-9],temp_text,temp_len);
			Value_has_been_changed=true;
		}

		if(	State_Proc_Button(BUTTON_DEC_ID) )
		{
			//uint32_t	*res_ptr = res_value[res_index-5] + res_index-5,   // // res_value[res_index-5]
						//*step_ptr = res_step + res_index-5;

			if (res_value[res_index-9]>100)			// // res_value[res_index-5]>1  res_value[res_index-5]--
			{
				res_value[res_index-9]=res_value[res_index-9]-Unit2;
			}
			else
			{
				res_value[res_index-9]=2;
			}

			//*res_ptr = Update_Resistor_Value_after_button(*res_ptr,*step_ptr, false);
			temp_len = itoa_10_fixed_position(res_value[res_index-9],temp_text,8);
			UI_V2_Update_after_change(res_index,res_value[res_index-9],temp_text,temp_len);
			lcm_text_buffer_cpy(LCM_SEL_UNIT_2_DISPLAY,1,6,temp_text,temp_len);
			// update RA/RB/RC
			temp_len = Show_Resistor_3_Digits(res_value[res_index-9],temp_text);
			UI_V2_Update_after_change(res_index-9,res_value[res_index-9],temp_text,temp_len);
			Value_has_been_changed=true;
		}
	}
	if(res_index==10)
	{
		if(	State_Proc_Button(BUTTON_INC_ID) )
		{
			if(res_value[res_index-10]<=1047575)
			{
				res_value[res_index-10]=res_value[res_index-10]+Unit3;
			}
			else
			{
				res_value[res_index-10]=1048575;
			}
			//*res_ptr = Update_Resistor_Value_after_button(*res_ptr,*step_ptr, true);
			temp_len = itoa_10_fixed_position(res_value[res_index-10],temp_text,8);
			UI_V2_Update_after_change(res_index,res_value[res_index-10],temp_text,temp_len);
			lcm_text_buffer_cpy(LCM_SEL_UNIT_3_DISPLAY,1,6,temp_text,temp_len);
			// update RA/RB/RC
			temp_len = Show_Resistor_3_Digits(res_value[res_index-10],temp_text);
			UI_V2_Update_after_change(res_index-10,res_value[res_index-10],temp_text,temp_len);
			Value_has_been_changed=true;
		}

		if(	State_Proc_Button(BUTTON_DEC_ID) )
		{
			//uint32_t	*res_ptr = res_value[res_index-5] + res_index-5,   // // res_value[res_index-5]
						//*step_ptr = res_step + res_index-5;

			if (res_value[res_index-10]>1000)			// // res_value[res_index-5]>1  res_value[res_index-5]--
			{
				res_value[res_index-10]=res_value[res_index-10]-Unit3;
			}
			else
			{
				res_value[res_index-10]=2;
			}

			//*res_ptr = Update_Resistor_Value_after_button(*res_ptr,*step_ptr, false);
			temp_len = itoa_10_fixed_position(res_value[res_index-10],temp_text,8);
			UI_V2_Update_after_change(res_index,res_value[res_index-10],temp_text,temp_len);
			lcm_text_buffer_cpy(LCM_SEL_UNIT_3_DISPLAY,1,6,temp_text,temp_len);
			// update RA/RB/RC
			temp_len = Show_Resistor_3_Digits(res_value[res_index-10],temp_text);
			UI_V2_Update_after_change(res_index-10,res_value[res_index-10],temp_text,temp_len);
			Value_has_been_changed=true;
		}
	}
	if(res_index==11)
	{
		if(	State_Proc_Button(BUTTON_INC_ID) )
		{
			if(res_value[res_index-11]<=1038575)
			{
				res_value[res_index-11]=res_value[res_index-11]+Unit4;
			}
			else
			{
				res_value[res_index-11]=1048575;
			}
			//*res_ptr = Update_Resistor_Value_after_button(*res_ptr,*step_ptr, true);
			temp_len = itoa_10_fixed_position(res_value[res_index-11],temp_text,8);
			UI_V2_Update_after_change(res_index,res_value[res_index-11],temp_text,temp_len);
			lcm_text_buffer_cpy(LCM_SEL_UNIT_4_DISPLAY,1,6,temp_text,temp_len);
			// update RA/RB/RC
			temp_len = Show_Resistor_3_Digits(res_value[res_index-11],temp_text);
			UI_V2_Update_after_change(res_index-11,res_value[res_index-11],temp_text,temp_len);
			Value_has_been_changed=true;
		}

		if(	State_Proc_Button(BUTTON_DEC_ID) )
		{
			//uint32_t	*res_ptr = res_value[res_index-5] + res_index-5,   // // res_value[res_index-5]
						//*step_ptr = res_step + res_index-5;

			if (res_value[res_index-11]>10000)			// // res_value[res_index-5]>1  res_value[res_index-5]--
			{
				res_value[res_index-11]=res_value[res_index-11]-Unit4;
			}
			else
			{
				res_value[res_index-11]=2;
			}

			//*res_ptr = Update_Resistor_Value_after_button(*res_ptr,*step_ptr, false);
			temp_len = itoa_10_fixed_position(res_value[res_index-11],temp_text,8);
			UI_V2_Update_after_change(res_index,res_value[res_index-11],temp_text,temp_len);
			lcm_text_buffer_cpy(LCM_SEL_UNIT_4_DISPLAY,1,6,temp_text,temp_len);
			// update RA/RB/RC
			temp_len = Show_Resistor_3_Digits(res_value[res_index-11],temp_text);
			UI_V2_Update_after_change(res_index-11,res_value[res_index-11],temp_text,temp_len);
			Value_has_been_changed=true;
		}
	}
	if(res_index==12)
	{
		if(	State_Proc_Button(BUTTON_INC_ID) )
		{
			if(res_value[res_index-12]<=948575)
			{
				res_value[res_index-12]=res_value[res_index-12]+Unit5;
			}
			else
			{
				res_value[res_index-12]=1048575;
			}
			//*res_ptr = Update_Resistor_Value_after_button(*res_ptr,*step_ptr, true);
			temp_len = itoa_10_fixed_position(res_value[res_index-12],temp_text,8);
			UI_V2_Update_after_change(res_index,res_value[res_index-12],temp_text,temp_len);
			lcm_text_buffer_cpy(LCM_SEL_UNIT_5_DISPLAY,1,6,temp_text,temp_len);
			// update RA/RB/RC
			temp_len = Show_Resistor_3_Digits(res_value[res_index-12],temp_text);
			UI_V2_Update_after_change(res_index-12,res_value[res_index-12],temp_text,temp_len);
			Value_has_been_changed=true;
		}

		if(	State_Proc_Button(BUTTON_DEC_ID) )
		{
			//uint32_t	*res_ptr = res_value[res_index-5] + res_index-5,   // // res_value[res_index-5]
						//*step_ptr = res_step + res_index-5;

			if (res_value[res_index-12]>100000)			// // res_value[res_index-5]>1  res_value[res_index-5]--
			{
				res_value[res_index-12]=res_value[res_index-12]-Unit5;
			}
			else
			{
				res_value[res_index-12]=2;
			}

			//*res_ptr = Update_Resistor_Value_after_button(*res_ptr,*step_ptr, false);
			temp_len = itoa_10_fixed_position(res_value[res_index-12],temp_text,8);
			UI_V2_Update_after_change(res_index,res_value[res_index-12],temp_text,temp_len);
			lcm_text_buffer_cpy(LCM_SEL_UNIT_5_DISPLAY,1,6,temp_text,temp_len);
			// update RA/RB/RC
			temp_len = Show_Resistor_3_Digits(res_value[res_index-12],temp_text);
			UI_V2_Update_after_change(res_index-12,res_value[res_index-12],temp_text,temp_len);
			Value_has_been_changed=true;
		}
	}
	if(res_index==13)
	{
		if(	State_Proc_Button(BUTTON_INC_ID) )
		{
			if(res_value[res_index-12]<=1048564)
			{
				res_value[res_index-12]=res_value[res_index-12]+Unit1;
			}
			else
			{
				res_value[res_index-12]=1048575;
			}
			//*res_ptr = Update_Resistor_Value_after_button(*res_ptr,*step_ptr, true);
			temp_len = itoa_10_fixed_position(res_value[res_index-12],temp_text,8);
			UI_V2_Update_after_change(res_index,res_value[res_index-12],temp_text,temp_len);
			lcm_text_buffer_cpy(LCM_SEL_UNIT_1_DISPLAY,1,6,temp_text,temp_len);
			// update RA/RB/RC
			temp_len = Show_Resistor_3_Digits(res_value[res_index-12],temp_text);
			UI_V2_Update_after_change(res_index-12,res_value[res_index-12],temp_text,temp_len);
			Value_has_been_changed=true;
		}

		if(	State_Proc_Button(BUTTON_DEC_ID) )
		{
			//uint32_t	*res_ptr = res_value[res_index-5] + res_index-5,   // // res_value[res_index-5]
						//*step_ptr = res_step + res_index-5;

			if (res_value[res_index-12]>10)			// // res_value[res_index-5]>1  res_value[res_index-5]--
			{
				res_value[res_index-12]=res_value[res_index-12]-Unit1;
			}
			else
			{
				res_value[res_index-12]=2;
			}

			//*res_ptr = Update_Resistor_Value_after_button(*res_ptr,*step_ptr, false);
			temp_len = itoa_10_fixed_position(res_value[res_index-12],temp_text,8);
			UI_V2_Update_after_change(res_index,res_value[res_index-12],temp_text,temp_len);
			lcm_text_buffer_cpy(LCM_SEL_UNIT_1_DISPLAY,1,6,temp_text,temp_len);
			// update RA/RB/RC
			temp_len = Show_Resistor_3_Digits(res_value[res_index-12],temp_text);
			UI_V2_Update_after_change(res_index-12,res_value[res_index-12],temp_text,temp_len);
			Value_has_been_changed=true;
		}
	}
	if(res_index==14)
	{
		if(	State_Proc_Button(BUTTON_INC_ID) )
		{
			if(res_value[res_index-13]<=1048475)
			{
				res_value[res_index-13]=res_value[res_index-13]+Unit2;
			}
			else
			{
				res_value[res_index-13]=1048575;
			}
			//*res_ptr = Update_Resistor_Value_after_button(*res_ptr,*step_ptr, true);
			temp_len = itoa_10_fixed_position(res_value[res_index-13],temp_text,8);
			UI_V2_Update_after_change(res_index,res_value[res_index-13],temp_text,temp_len);
			lcm_text_buffer_cpy(LCM_SEL_UNIT_2_DISPLAY,1,6,temp_text,temp_len);
			// update RA/RB/RC
			temp_len = Show_Resistor_3_Digits(res_value[res_index-13],temp_text);
			UI_V2_Update_after_change(res_index-13,res_value[res_index-13],temp_text,temp_len);
			Value_has_been_changed=true;
		}

		if(	State_Proc_Button(BUTTON_DEC_ID) )
		{
			//uint32_t	*res_ptr = res_value[res_index-5] + res_index-5,   // // res_value[res_index-5]
						//*step_ptr = res_step + res_index-5;

			if (res_value[res_index-13]>100)			// // res_value[res_index-5]>1  res_value[res_index-5]--
			{
				res_value[res_index-13]=res_value[res_index-13]-Unit2;
			}
			else
			{
				res_value[res_index-13]=2;
			}

			//*res_ptr = Update_Resistor_Value_after_button(*res_ptr,*step_ptr, false);
			temp_len = itoa_10_fixed_position(res_value[res_index-13],temp_text,8);
			UI_V2_Update_after_change(res_index,res_value[res_index-13],temp_text,temp_len);
			lcm_text_buffer_cpy(LCM_SEL_UNIT_2_DISPLAY,1,6,temp_text,temp_len);
			// update RA/RB/RC
			temp_len = Show_Resistor_3_Digits(res_value[res_index-13],temp_text);
			UI_V2_Update_after_change(res_index-13,res_value[res_index-13],temp_text,temp_len);
			Value_has_been_changed=true;
		}
	}
	if(res_index==15)
	{
		if(	State_Proc_Button(BUTTON_INC_ID) )
		{
			if(res_value[res_index-14]<=1047575)
			{
				res_value[res_index-14]=res_value[res_index-14]+Unit3;
			}
			else
			{
				res_value[res_index-14]=1048575;
			}
			//*res_ptr = Update_Resistor_Value_after_button(*res_ptr,*step_ptr, true);
			temp_len = itoa_10_fixed_position(res_value[res_index-14],temp_text,8);
			UI_V2_Update_after_change(res_index,res_value[res_index-14],temp_text,temp_len);
			lcm_text_buffer_cpy(LCM_SEL_UNIT_3_DISPLAY,1,6,temp_text,temp_len);
			// update RA/RB/RC
			temp_len = Show_Resistor_3_Digits(res_value[res_index-14],temp_text);
			UI_V2_Update_after_change(res_index-14,res_value[res_index-14],temp_text,temp_len);
			Value_has_been_changed=true;
		}

		if(	State_Proc_Button(BUTTON_DEC_ID) )
		{
			//uint32_t	*res_ptr = res_value[res_index-5] + res_index-5,   // // res_value[res_index-5]
						//*step_ptr = res_step + res_index-5;

			if (res_value[res_index-14]>1000)			// // res_value[res_index-5]>1  res_value[res_index-5]--
			{
				res_value[res_index-14]=res_value[res_index-14]-Unit3;
			}
			else
			{
				res_value[res_index-14]=2;
			}

			//*res_ptr = Update_Resistor_Value_after_button(*res_ptr,*step_ptr, false);
			temp_len = itoa_10_fixed_position(res_value[res_index-14],temp_text,8);
			UI_V2_Update_after_change(res_index,res_value[res_index-14],temp_text,temp_len);
			lcm_text_buffer_cpy(LCM_SEL_UNIT_3_DISPLAY,1,6,temp_text,temp_len);
			// update RA/RB/RC
			temp_len = Show_Resistor_3_Digits(res_value[res_index-14],temp_text);
			UI_V2_Update_after_change(res_index-14,res_value[res_index-14],temp_text,temp_len);
			Value_has_been_changed=true;
		}
	}
	if(res_index==16)
	{
		if(	State_Proc_Button(BUTTON_INC_ID) )
		{
			if(res_value[res_index-15]<=1038575)
			{
				res_value[res_index-15]=res_value[res_index-15]+Unit4;
			}
			else
			{
				res_value[res_index-15]=1048575;
			}
			//*res_ptr = Update_Resistor_Value_after_button(*res_ptr,*step_ptr, true);
			temp_len = itoa_10_fixed_position(res_value[res_index-15],temp_text,8);
			UI_V2_Update_after_change(res_index,res_value[res_index-15],temp_text,temp_len);
			lcm_text_buffer_cpy(LCM_SEL_UNIT_4_DISPLAY,1,6,temp_text,temp_len);
			// update RA/RB/RC
			temp_len = Show_Resistor_3_Digits(res_value[res_index-15],temp_text);
			UI_V2_Update_after_change(res_index-15,res_value[res_index-15],temp_text,temp_len);
			Value_has_been_changed=true;
		}

		if(	State_Proc_Button(BUTTON_DEC_ID) )
		{
			//uint32_t	*res_ptr = res_value[res_index-5] + res_index-5,   // // res_value[res_index-5]
						//*step_ptr = res_step + res_index-5;

			if (res_value[res_index-15]>10000)			// // res_value[res_index-5]>1  res_value[res_index-5]--
			{
				res_value[res_index-15]=res_value[res_index-15]-Unit4;
			}
			else
			{
				res_value[res_index-15]=2;
			}

			//*res_ptr = Update_Resistor_Value_after_button(*res_ptr,*step_ptr, false);
			temp_len = itoa_10_fixed_position(res_value[res_index-15],temp_text,8);
			UI_V2_Update_after_change(res_index,res_value[res_index-15],temp_text,temp_len);
			lcm_text_buffer_cpy(LCM_SEL_UNIT_4_DISPLAY,1,6,temp_text,temp_len);
			// update RA/RB/RC
			temp_len = Show_Resistor_3_Digits(res_value[res_index-15],temp_text);
			UI_V2_Update_after_change(res_index-15,res_value[res_index-15],temp_text,temp_len);
			Value_has_been_changed=true;
		}
	}
	if(res_index==17)
	{
		if(	State_Proc_Button(BUTTON_INC_ID) )
		{
			if(res_value[res_index-16]<=948575)
			{
				res_value[res_index-16]=res_value[res_index-16]+Unit5;
			}
			else
			{
				res_value[res_index-16]=1048575;
			}
			//*res_ptr = Update_Resistor_Value_after_button(*res_ptr,*step_ptr, true);
			temp_len = itoa_10_fixed_position(res_value[res_index-16],temp_text,8);
			UI_V2_Update_after_change(res_index,res_value[res_index-16],temp_text,temp_len);
			lcm_text_buffer_cpy(LCM_SEL_UNIT_5_DISPLAY,1,6,temp_text,temp_len);
			// update RA/RB/RC
			temp_len = Show_Resistor_3_Digits(res_value[res_index-16],temp_text);
			UI_V2_Update_after_change(res_index-16,res_value[res_index-16],temp_text,temp_len);
			Value_has_been_changed=true;
		}

		if(	State_Proc_Button(BUTTON_DEC_ID) )
		{
			//uint32_t	*res_ptr = res_value[res_index-5] + res_index-5,   // // res_value[res_index-5]
						//*step_ptr = res_step + res_index-5;

			if (res_value[res_index-16]>100000)			// // res_value[res_index-5]>1  res_value[res_index-5]--
			{
				res_value[res_index-16]=res_value[res_index-16]-Unit5;
			}
			else
			{
				res_value[res_index-16]=2;
			}

			//*res_ptr = Update_Resistor_Value_after_button(*res_ptr,*step_ptr, false);
			temp_len = itoa_10_fixed_position(res_value[res_index-16],temp_text,8);
			UI_V2_Update_after_change(res_index,res_value[res_index-16],temp_text,temp_len);
			lcm_text_buffer_cpy(LCM_SEL_UNIT_5_DISPLAY,1,6,temp_text,temp_len);
			// update RA/RB/RC
			temp_len = Show_Resistor_3_Digits(res_value[res_index-16],temp_text);
			UI_V2_Update_after_change(res_index-16,res_value[res_index-16],temp_text,temp_len);
			Value_has_been_changed=true;
		}
	}
	if(res_index==18)
	{
		if(	State_Proc_Button(BUTTON_INC_ID) )
		{
			if(res_value[res_index-16]<=1048564)
			{
				res_value[res_index-16]=res_value[res_index-16]+Unit1;
			}
			else
			{
				res_value[res_index-16]=1048575;
			}
			//*res_ptr = Update_Resistor_Value_after_button(*res_ptr,*step_ptr, true);
			temp_len = itoa_10_fixed_position(res_value[res_index-16],temp_text,8);
			UI_V2_Update_after_change(res_index,res_value[res_index-16],temp_text,temp_len);
			lcm_text_buffer_cpy(LCM_SEL_UNIT_1_DISPLAY,1,6,temp_text,temp_len);
			// update RA/RB/RC
			temp_len = Show_Resistor_3_Digits(res_value[res_index-16],temp_text);
			UI_V2_Update_after_change(res_index-16,res_value[res_index-16],temp_text,temp_len);
			Value_has_been_changed=true;
		}

		if(	State_Proc_Button(BUTTON_DEC_ID) )
		{
			//uint32_t	*res_ptr = res_value[res_index-5] + res_index-5,   // // res_value[res_index-5]
						//*step_ptr = res_step + res_index-5;

			if (res_value[res_index-16]>10)			// // res_value[res_index-5]>1  res_value[res_index-5]--
			{
				res_value[res_index-16]=res_value[res_index-16]-Unit1;
			}
			else
			{
				res_value[res_index-16]=2;
			}

			//*res_ptr = Update_Resistor_Value_after_button(*res_ptr,*step_ptr, false);
			temp_len = itoa_10_fixed_position(res_value[res_index-16],temp_text,8);
			UI_V2_Update_after_change(res_index,res_value[res_index-16],temp_text,temp_len);
			lcm_text_buffer_cpy(LCM_SEL_UNIT_1_DISPLAY,1,6,temp_text,temp_len);
			// update RA/RB/RC
			temp_len = Show_Resistor_3_Digits(res_value[res_index-16],temp_text);
			UI_V2_Update_after_change(res_index-16,res_value[res_index-16],temp_text,temp_len);
			Value_has_been_changed=true;
		}
	}
	if(res_index==19)
	{
		if(	State_Proc_Button(BUTTON_INC_ID) )
		{
			if(res_value[res_index-17]<=1048475)
			{
				res_value[res_index-17]=res_value[res_index-17]+Unit2;
			}
			else
			{
				res_value[res_index-17]=1048575;
			}
			//*res_ptr = Update_Resistor_Value_after_button(*res_ptr,*step_ptr, true);
			temp_len = itoa_10_fixed_position(res_value[res_index-17],temp_text,8);
			UI_V2_Update_after_change(res_index,res_value[res_index-17],temp_text,temp_len);
			lcm_text_buffer_cpy(LCM_SEL_UNIT_2_DISPLAY,1,6,temp_text,temp_len);
			// update RA/RB/RC
			temp_len = Show_Resistor_3_Digits(res_value[res_index-17],temp_text);
			UI_V2_Update_after_change(res_index-17,res_value[res_index-17],temp_text,temp_len);
			Value_has_been_changed=true;
		}

		if(	State_Proc_Button(BUTTON_DEC_ID) )
		{
			//uint32_t	*res_ptr = res_value[res_index-5] + res_index-5,   // // res_value[res_index-5]
						//*step_ptr = res_step + res_index-5;

			if (res_value[res_index-17]>100)			// // res_value[res_index-5]>1  res_value[res_index-5]--
			{
				res_value[res_index-17]=res_value[res_index-17]-Unit2;
			}
			else
			{
				res_value[res_index-17]=2;
			}

			//*res_ptr = Update_Resistor_Value_after_button(*res_ptr,*step_ptr, false);
			temp_len = itoa_10_fixed_position(res_value[res_index-17],temp_text,8);
			UI_V2_Update_after_change(res_index,res_value[res_index-17],temp_text,temp_len);
			lcm_text_buffer_cpy(LCM_SEL_UNIT_2_DISPLAY,1,6,temp_text,temp_len);
			// update RA/RB/RC
			temp_len = Show_Resistor_3_Digits(res_value[res_index-17],temp_text);
			UI_V2_Update_after_change(res_index-17,res_value[res_index-17],temp_text,temp_len);
			Value_has_been_changed=true;
		}
	}
	if(res_index==20)
	{
		if(	State_Proc_Button(BUTTON_INC_ID) )
		{
			if(res_value[res_index-18]<=1047575)
			{
				res_value[res_index-18]=res_value[res_index-18]+Unit3;
			}
			else
			{
				res_value[res_index-18]=1048575;
			}
			//*res_ptr = Update_Resistor_Value_after_button(*res_ptr,*step_ptr, true);
			temp_len = itoa_10_fixed_position(res_value[res_index-18],temp_text,8);
			UI_V2_Update_after_change(res_index,res_value[res_index-18],temp_text,temp_len);
			lcm_text_buffer_cpy(LCM_SEL_UNIT_3_DISPLAY,1,6,temp_text,temp_len);
			// update RA/RB/RC
			temp_len = Show_Resistor_3_Digits(res_value[res_index-18],temp_text);
			UI_V2_Update_after_change(res_index-18,res_value[res_index-18],temp_text,temp_len);
			Value_has_been_changed=true;
		}

		if(	State_Proc_Button(BUTTON_DEC_ID) )
		{
			//uint32_t	*res_ptr = res_value[res_index-5] + res_index-5,   // // res_value[res_index-5]
						//*step_ptr = res_step + res_index-5;

			if (res_value[res_index-18]>1000)			// // res_value[res_index-5]>1  res_value[res_index-5]--
			{
				res_value[res_index-18]=res_value[res_index-18]-Unit3;
			}
			else
			{
				res_value[res_index-18]=2;
			}

			//*res_ptr = Update_Resistor_Value_after_button(*res_ptr,*step_ptr, false);
			temp_len = itoa_10_fixed_position(res_value[res_index-18],temp_text,8);
			UI_V2_Update_after_change(res_index,res_value[res_index-18],temp_text,temp_len);
			lcm_text_buffer_cpy(LCM_SEL_UNIT_3_DISPLAY,1,6,temp_text,temp_len);
			// update RA/RB/RC
			temp_len = Show_Resistor_3_Digits(res_value[res_index-18],temp_text);
			UI_V2_Update_after_change(res_index-18,res_value[res_index-18],temp_text,temp_len);
			Value_has_been_changed=true;
		}
	}
	if(res_index==21)
	{
		if(	State_Proc_Button(BUTTON_INC_ID) )
		{
			if(res_value[res_index-19]<=1038575)
			{
				res_value[res_index-19]=res_value[res_index-19]+Unit4;
			}
			else
			{
				res_value[res_index-19]=1048575;
			}
			//*res_ptr = Update_Resistor_Value_after_button(*res_ptr,*step_ptr, true);
			temp_len = itoa_10_fixed_position(res_value[res_index-19],temp_text,8);
			UI_V2_Update_after_change(res_index,res_value[res_index-19],temp_text,temp_len);
			lcm_text_buffer_cpy(LCM_SEL_UNIT_4_DISPLAY,1,6,temp_text,temp_len);
			// update RA/RB/RC
			temp_len = Show_Resistor_3_Digits(res_value[res_index-19],temp_text);
			UI_V2_Update_after_change(res_index-19,res_value[res_index-19],temp_text,temp_len);
			Value_has_been_changed=true;
		}

		if(	State_Proc_Button(BUTTON_DEC_ID) )
		{
			//uint32_t	*res_ptr = res_value[res_index-5] + res_index-5,   // // res_value[res_index-5]
						//*step_ptr = res_step + res_index-5;

			if (res_value[res_index-19]>10000)			// // res_value[res_index-5]>1  res_value[res_index-5]--
			{
				res_value[res_index-19]=res_value[res_index-19]-Unit4;
			}
			else
			{
				res_value[res_index-19]=2;
			}

			//*res_ptr = Update_Resistor_Value_after_button(*res_ptr,*step_ptr, false);
			temp_len = itoa_10_fixed_position(res_value[res_index-19],temp_text,8);
			UI_V2_Update_after_change(res_index,res_value[res_index-19],temp_text,temp_len);
			lcm_text_buffer_cpy(LCM_SEL_UNIT_4_DISPLAY,1,6,temp_text,temp_len);
			// update RA/RB/RC
			temp_len = Show_Resistor_3_Digits(res_value[res_index-19],temp_text);
			UI_V2_Update_after_change(res_index-19,res_value[res_index-19],temp_text,temp_len);
			Value_has_been_changed=true;
		}
	}
	if(res_index==22)
	{
		if(	State_Proc_Button(BUTTON_INC_ID) )
		{
			if(res_value[res_index-20]<=948575)
			{
				res_value[res_index-20]=res_value[res_index-20]+Unit5;
			}
			else
			{
				res_value[res_index-20]=1048575;
			}
			//*res_ptr = Update_Resistor_Value_after_button(*res_ptr,*step_ptr, true);
			temp_len = itoa_10_fixed_position(res_value[res_index-20],temp_text,8);
			UI_V2_Update_after_change(res_index,res_value[res_index-20],temp_text,temp_len);
			lcm_text_buffer_cpy(LCM_SEL_UNIT_5_DISPLAY,1,6,temp_text,temp_len);
			// update RA/RB/RC
			temp_len = Show_Resistor_3_Digits(res_value[res_index-20],temp_text);
			UI_V2_Update_after_change(res_index-20,res_value[res_index-20],temp_text,temp_len);
			Value_has_been_changed=true;
		}

		if(	State_Proc_Button(BUTTON_DEC_ID) )
		{
			//uint32_t	*res_ptr = res_value[res_index-5] + res_index-5,   // // res_value[res_index-5]
						//*step_ptr = res_step + res_index-5;

			if (res_value[res_index-20]>100000)			// // res_value[res_index-5]>1  res_value[res_index-5]--
			{
				res_value[res_index-20]=res_value[res_index-20]-Unit5;
			}
			else
			{
				res_value[res_index-20]=2;
			}

			//*res_ptr = Update_Resistor_Value_after_button(*res_ptr,*step_ptr, false);
			temp_len = itoa_10_fixed_position(res_value[res_index-20],temp_text,8);
			UI_V2_Update_after_change(res_index,res_value[res_index-20],temp_text,temp_len);
			lcm_text_buffer_cpy(LCM_SEL_UNIT_5_DISPLAY,1,6,temp_text,temp_len);
			// update RA/RB/RC
			temp_len = Show_Resistor_3_Digits(res_value[res_index-20],temp_text);
			UI_V2_Update_after_change(res_index-20,res_value[res_index-20],temp_text,temp_len);
			Value_has_been_changed=true;
		}
	}
}


bool If_any_button_pressed(void)
{
	bool bRet = false;

	if (	State_Proc_Button(BUTTON_INC_ID) ||
			State_Proc_Button(BUTTON_DEC_ID) ||
			State_Proc_Button(BUTTON_SEL_ID) ||
			State_Proc_Button(BUTTON_SRC_ID) ||
			State_Proc_Button(BUTTON_ISP_ID) )
	{
		bRet = true;
	}
	return bRet;
}

bool If_value_has_been_changed(void)
{
	if(Value_has_been_changed)
	{
		Value_has_been_changed = false;
		return true;
	}
	else
		return false;

}

void update_resistor_value(void)
{
	char 				temp_text[10];
	int 				temp_len;

	switch(res_index)

	{
	    case 0:
	    case 1:
	    case 2:
	    case 3:

	    	temp_len = Show_Resistor_3_Digits(res_value[0],temp_text);
	    	UI_V2_Update_after_change(0,res_value[0],temp_text,temp_len);
	    	lcm_text_buffer_cpy(LCM_ALL_VR_DISPLAY,0,9+2,temp_text,temp_len);

			temp_len = Show_Resistor_3_Digits(res_value[1],temp_text);
			UI_V2_Update_after_change(1,res_value[1],temp_text,temp_len);
			lcm_text_buffer_cpy(LCM_ALL_VR_DISPLAY,1,2,temp_text,temp_len);

			temp_len = Show_Resistor_3_Digits(res_value[2],temp_text);
			UI_V2_Update_after_change(2,res_value[2],temp_text,temp_len);
  			lcm_text_buffer_cpy(LCM_ALL_VR_DISPLAY,1,9+2,temp_text,temp_len);
			break;
		case 5:
			temp_len = itoa_10_fixed_position(res_value[res_index-5],temp_text,8);
			UI_V2_Update_after_change(res_index,res_value[res_index-5],temp_text,temp_len);
			lcm_text_buffer_cpy(LCM_ALL_SEL_SET_DISPLAY,1,6,temp_text,temp_len);
			// update RA/RB/RC
			temp_len = Show_Resistor_3_Digits(res_value[res_index-5],temp_text);
			UI_V2_Update_after_change(res_index-5,res_value[res_index-5],temp_text,temp_len);
			break;
		case 6:
			temp_len = itoa_10_fixed_position(res_value[res_index-5],temp_text,8);
			UI_V2_Update_after_change(res_index,res_value[res_index-5],temp_text,temp_len);
			lcm_text_buffer_cpy(LCM_ALL_SEL_SET_DISPLAY,1,6,temp_text,temp_len);

			temp_len = Show_Resistor_3_Digits(res_value[res_index-5],temp_text);
			UI_V2_Update_after_change(res_index-5,res_value[res_index-5],temp_text,temp_len);
			break;
		case 7:
			temp_len = itoa_10_fixed_position(res_value[res_index-5],temp_text,8);
			UI_V2_Update_after_change(res_index,res_value[res_index-5],temp_text,temp_len);
			lcm_text_buffer_cpy(LCM_ALL_SEL_SET_DISPLAY,1,6,temp_text,temp_len);

			temp_len = Show_Resistor_3_Digits(res_value[res_index-5],temp_text);
			UI_V2_Update_after_change(res_index-5,res_value[res_index-5],temp_text,temp_len);
			break;
		case 8:
			temp_len = itoa_10_fixed_position(res_value[res_index-8],temp_text,8);
			UI_V2_Update_after_change(res_index,res_value[res_index-8],temp_text,temp_len);
			lcm_text_buffer_cpy(LCM_SEL_UNIT_1_DISPLAY,1,6,temp_text,temp_len);

			temp_len = Show_Resistor_3_Digits(res_value[res_index-8],temp_text);
			UI_V2_Update_after_change(res_index-8,res_value[res_index-8],temp_text,temp_len);
			break;
		case 9:
			temp_len = itoa_10_fixed_position(res_value[res_index-9],temp_text,8);
			UI_V2_Update_after_change(res_index,res_value[res_index-9],temp_text,temp_len);
			lcm_text_buffer_cpy(LCM_SEL_UNIT_2_DISPLAY,1,6,temp_text,temp_len);

			temp_len = Show_Resistor_3_Digits(res_value[res_index-9],temp_text);
			UI_V2_Update_after_change(res_index-9,res_value[res_index-9],temp_text,temp_len);
			break;
		case 10:
			temp_len = itoa_10_fixed_position(res_value[res_index-10],temp_text,8);
			UI_V2_Update_after_change(res_index,res_value[res_index-10],temp_text,temp_len);
			lcm_text_buffer_cpy(LCM_SEL_UNIT_3_DISPLAY,1,6,temp_text,temp_len);

			temp_len = Show_Resistor_3_Digits(res_value[res_index-10],temp_text);
			UI_V2_Update_after_change(res_index-10,res_value[res_index-10],temp_text,temp_len);
			break;
		case 11:
			temp_len = itoa_10_fixed_position(res_value[res_index-11],temp_text,8);
			UI_V2_Update_after_change(res_index,res_value[res_index-11],temp_text,temp_len);
			lcm_text_buffer_cpy(LCM_SEL_UNIT_4_DISPLAY,1,6,temp_text,temp_len);

			temp_len = Show_Resistor_3_Digits(res_value[res_index-11],temp_text);
			UI_V2_Update_after_change(res_index-11,res_value[res_index-11],temp_text,temp_len);
			break;
		case 12:
			temp_len = itoa_10_fixed_position(res_value[res_index-12],temp_text,8);
			UI_V2_Update_after_change(res_index,res_value[res_index-12],temp_text,temp_len);
			lcm_text_buffer_cpy(LCM_SEL_UNIT_5_DISPLAY,1,6,temp_text,temp_len);

			temp_len = Show_Resistor_3_Digits(res_value[res_index-12],temp_text);
			UI_V2_Update_after_change(res_index-12,res_value[res_index-12],temp_text,temp_len);
			break;
		case 13:
			temp_len = itoa_10_fixed_position(res_value[res_index-12],temp_text,8);
			UI_V2_Update_after_change(res_index,res_value[res_index-12],temp_text,temp_len);
			lcm_text_buffer_cpy(LCM_SEL_UNIT_1_DISPLAY,1,6,temp_text,temp_len);

			temp_len = Show_Resistor_3_Digits(res_value[res_index-12],temp_text);
			UI_V2_Update_after_change(res_index-12,res_value[res_index-12],temp_text,temp_len);
			break;
		case 14:
			temp_len = itoa_10_fixed_position(res_value[res_index-13],temp_text,8);
			UI_V2_Update_after_change(res_index,res_value[res_index-13],temp_text,temp_len);
			lcm_text_buffer_cpy(LCM_SEL_UNIT_2_DISPLAY,1,6,temp_text,temp_len);

			temp_len = Show_Resistor_3_Digits(res_value[res_index-13],temp_text);
			UI_V2_Update_after_change(res_index-13,res_value[res_index-13],temp_text,temp_len);
			break;
		case 15:
			temp_len = itoa_10_fixed_position(res_value[res_index-14],temp_text,8);
			UI_V2_Update_after_change(res_index,res_value[res_index-14],temp_text,temp_len);
			lcm_text_buffer_cpy(LCM_SEL_UNIT_3_DISPLAY,1,6,temp_text,temp_len);

			temp_len = Show_Resistor_3_Digits(res_value[res_index-14],temp_text);
			UI_V2_Update_after_change(res_index-14,res_value[res_index-14],temp_text,temp_len);
			break;
		case 16:
			temp_len = itoa_10_fixed_position(res_value[res_index-15],temp_text,8);
			UI_V2_Update_after_change(res_index,res_value[res_index-15],temp_text,temp_len);
			lcm_text_buffer_cpy(LCM_SEL_UNIT_4_DISPLAY,1,6,temp_text,temp_len);

			temp_len = Show_Resistor_3_Digits(res_value[res_index-15],temp_text);
			UI_V2_Update_after_change(res_index-15,res_value[res_index-15],temp_text,temp_len);
			break;
		case 17:
			temp_len = itoa_10_fixed_position(res_value[res_index-16],temp_text,8);
			UI_V2_Update_after_change(res_index,res_value[res_index-16],temp_text,temp_len);
			lcm_text_buffer_cpy(LCM_SEL_UNIT_5_DISPLAY,1,6,temp_text,temp_len);

			temp_len = Show_Resistor_3_Digits(res_value[res_index-16],temp_text);
			UI_V2_Update_after_change(res_index-16,res_value[res_index-16],temp_text,temp_len);
			break;
		case 18:
			temp_len = itoa_10_fixed_position(res_value[res_index-16],temp_text,8);
			UI_V2_Update_after_change(res_index,res_value[res_index-16],temp_text,temp_len);
			lcm_text_buffer_cpy(LCM_SEL_UNIT_1_DISPLAY,1,6,temp_text,temp_len);

			temp_len = Show_Resistor_3_Digits(res_value[res_index-16],temp_text);
			UI_V2_Update_after_change(res_index-16,res_value[res_index-16],temp_text,temp_len);
			break;
		case 19:
			temp_len = itoa_10_fixed_position(res_value[res_index-17],temp_text,8);
			UI_V2_Update_after_change(res_index,res_value[res_index-17],temp_text,temp_len);
			lcm_text_buffer_cpy(LCM_SEL_UNIT_2_DISPLAY,1,6,temp_text,temp_len);

			temp_len = Show_Resistor_3_Digits(res_value[res_index-17],temp_text);
			UI_V2_Update_after_change(res_index-17,res_value[res_index-17],temp_text,temp_len);
			break;
		case 20:
			temp_len = itoa_10_fixed_position(res_value[res_index-18],temp_text,8);
			UI_V2_Update_after_change(res_index,res_value[res_index-18],temp_text,temp_len);
			lcm_text_buffer_cpy(LCM_SEL_UNIT_3_DISPLAY,1,6,temp_text,temp_len);

			temp_len = Show_Resistor_3_Digits(res_value[res_index-18],temp_text);
			UI_V2_Update_after_change(res_index-18,res_value[res_index-18],temp_text,temp_len);
			break;
		case 21:
			temp_len = itoa_10_fixed_position(res_value[res_index-19],temp_text,8);
			UI_V2_Update_after_change(res_index,res_value[res_index-19],temp_text,temp_len);
			lcm_text_buffer_cpy(LCM_SEL_UNIT_4_DISPLAY,1,6,temp_text,temp_len);

			temp_len = Show_Resistor_3_Digits(res_value[res_index-19],temp_text);
			UI_V2_Update_after_change(res_index-19,res_value[res_index-19],temp_text,temp_len);
			break;
		case 22:
			temp_len = itoa_10_fixed_position(res_value[res_index-20],temp_text,8);
			UI_V2_Update_after_change(res_index,res_value[res_index-20],temp_text,temp_len);
			lcm_text_buffer_cpy(LCM_SEL_UNIT_5_DISPLAY,1,6,temp_text,temp_len);

			temp_len = Show_Resistor_3_Digits(res_value[res_index-20],temp_text);
			UI_V2_Update_after_change(res_index-20,res_value[res_index-20],temp_text,temp_len);
			break;
		default:
			break;
	}

}
///
///
///


