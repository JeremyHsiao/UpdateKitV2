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

#ifdef _REAL_UPDATEKIT_V2_BOARD_

#define BUTTON_SRC_PORT		BLUE_KEY_GPIO_PORT
#define BUTTON_SRC_PIN		BLUE_KEY_GPIO_PIN
#define BUTTON_DEC_PORT		YELLOW_KEY_GPIO_PORT
#define BUTTON_DEC_PIN		YELLOW_KEY_GPIO_PIN
#define BUTTON_INC_PORT		GREEN_KEY_GPIO_PORT
#define BUTTON_INC_PIN		GREEN_KEY_GPIO_PIN
#define BUTTON_SEL_PORT		RED_KEY_GPIO_PORT
#define BUTTON_SEL_PIN		RED_KEY_GPIO_PIN
#define BUTTON_ISP_PORT		SWITCH_KEY_GPIO_PORT
#define BUTTON_ISP_PIN		SWITCH_KEY_GPIO_PIN

#else

#define BUTTON_SRC_PORT		BLUE_KEY_GPIO_PORT
#define BUTTON_SRC_PIN		BLUE_KEY_GPIO_PIN
#define BUTTON_DEC_PORT		YELLOW_KEY_GPIO_PORT
#define BUTTON_DEC_PIN		YELLOW_KEY_GPIO_PIN
#define BUTTON_INC_PORT		GREEN_KEY_GPIO_PORT
#define BUTTON_INC_PIN		GREEN_KEY_GPIO_PIN
#define BUTTON_SEL_PORT		RED_KEY_GPIO_PORT
#define BUTTON_SEL_PIN		RED_KEY_GPIO_PIN
#define BUTTON_ISP_PORT		SWITCH_KEY_GPIO_PORT
#define BUTTON_ISP_PIN		SWITCH_KEY_GPIO_PIN

#endif // #ifdef _REAL_UPDATEKIT_V2_BOARD_

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/
uint8_t				current_output_stage;

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
	  "RL1/22       32\xf4",		// power(2,5)			/// RL and R to be updated later after this line
	  "RL43  R16/76/136",
	  "RL2/23       64\xf4",		// power(2,6)
	  "RL44  R17/77/137",
	  "RL3/24      128\xf4",		// power(2,7)
	  "RL45  R18/78/138",
	  "RL4/25      256\xf4",		// power(2,8)
	  "RL46  R19/79/139",
	  "RL5/26      512\xf4",		// power(2,9)
	  "RL47  R20/80/140",
	  "RL1/22     1024\xf4",		// power(2,10)
	  "RL43  R16/76/136",
	  "RL2/23     2048\xf4",		// power(2,11)
	  "RL44  R17/77/137",
	  "RL3/24     4096\xf4",		// power(2,12)
	  "RL45  R18/78/138",
	  "RL4/25     8192\xf4",		// power(2,13)
	  "RL46  R19/79/139",
	  "RL5/26    16384\xf4",		// power(2,14)
	  "RL47  R20/80/140",
	  "RL1/22    32768\xf4",		// power(2,15)
	  "RL43  R16/76/136",
	  "RL2/23    65536\xf4",		// power(2,16)
	  "RL44  R17/77/137",
	  "RL3/24   131072\xf4",		// power(2,17)
	  "RL45  R18/78/138",
	  "RL4/25   262144\xf4",		// power(2,18)
	  "RL46  R19/79/139",
	  "RL5/26   524288\xf4",		// power(2,19)
	  "RL47  R20/80/140",
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

void Init_Value_From_EEPROM(void)
{
	Load_User_Selection(&current_output_stage);
	Load_System_Timeout_v2(current_output_stage,&max_upgrade_time_in_S);
}

void Init_UpdateKitV2_variables(void)
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

//	//                      			 1234567890123456
//	lcm_text_buffer_cpy(LCM_PC_MODE,0,0,"PC Mode: Press  ", LCM_DISPLAY_COL);
//	lcm_text_buffer_cpy(LCM_PC_MODE,1,0,"button to change", LCM_DISPLAY_COL);

	lcm_text_buffer_cpy(LCM_SINGLE_VR_DISPLAY,0,0,"R0:    1        ",LCM_DISPLAY_COL);
	lcm_text_buffer_cpy(LCM_SINGLE_VR_DISPLAY,1,0,"button to change",LCM_DISPLAY_COL);

	//	//                      			 	      1234567890123456
	lcm_text_buffer_cpy(LCM_ALL_VR_DISPLAY,0,0,      "Value-R  A:    1", LCM_DISPLAY_COL);
	lcm_text_buffer_cpy(LCM_ALL_VR_DISPLAY,1,0,      "B:    1  C:    1", LCM_DISPLAY_COL);

	lcm_text_buffer_cpy(LCM_ALL_SET_BLINKING,0,0,    "Set RA   A:    1", LCM_DISPLAY_COL);
	lcm_text_buffer_cpy(LCM_ALL_SET_BLINKING,1,0,    "B:    1  C:    1", LCM_DISPLAY_COL);

	lcm_text_buffer_cpy(LCM_ALL_SET_R0_BLINKING,0,0, "         A:     ", LCM_DISPLAY_COL);
	lcm_text_buffer_cpy(LCM_ALL_SET_R0_BLINKING,1,0, "B:    1  C:    1", LCM_DISPLAY_COL);

	lcm_text_buffer_cpy(LCM_ALL_SET_R1_BLINKING,0,0, "         A:    1", LCM_DISPLAY_COL);
	lcm_text_buffer_cpy(LCM_ALL_SET_R1_BLINKING,1,0, "B:       C:    1", LCM_DISPLAY_COL);

	lcm_text_buffer_cpy(LCM_ALL_SET_R2_BLINKING,0,0, "         A:    1", LCM_DISPLAY_COL);
	lcm_text_buffer_cpy(LCM_ALL_SET_R2_BLINKING,1,0, "B:    1  C:     ", LCM_DISPLAY_COL);

	lcm_text_buffer_cpy(LCM_ALL_SET_2N_VALUE,   0,0, FineTuneResistorMsg[0], LCM_DISPLAY_COL);
	lcm_text_buffer_cpy(LCM_ALL_SET_2N_VALUE,   1,0, FineTuneResistorMsg[1], LCM_DISPLAY_COL);

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
	else if (temp_value<1)
	{
		temp_value = 1;
	}

	return (uint32_t) temp_value;
}

void UI_Version_01(void)
{
	char 				temp_text[10];
	int 				temp_len;
	static uint32_t		res_value[3] = { 1, 1, 1 }, res_step[3] = { 1, 1, 1 };
	static uint8_t		res_index = 0;

	if(	State_Proc_Button(BUTTON_INC_ID) )
	{
		uint32_t	*res_ptr = res_value + res_index,
					*step_ptr = res_step + res_index;

		lcd_module_display_enable_only_one_page(LCM_SINGLE_VR_DISPLAY);

		*res_ptr = Update_Resistor_Value_after_button(*res_ptr,*step_ptr, true);

		temp_len = Show_Resistor_3_Digits(*res_ptr,temp_text);
		lcm_text_buffer_cpy(LCM_SINGLE_VR_DISPLAY,0,3,temp_text,temp_len);
	}

	if(	State_Proc_Button(BUTTON_DEC_ID) )
	{
		uint32_t	*res_ptr = res_value + res_index,
					*step_ptr = res_step + res_index;

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
		if(++res_index>=3)
		{
			res_index = 0;
		}
		temp_text[0] = '0'+ res_index;
		lcm_text_buffer_cpy(LCM_SINGLE_VR_DISPLAY,0,1,temp_text,1);
		res_ptr = res_value + res_index;
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
	{//
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
	}
}

// LCM_ALL_SET_2N_VALUE

#define RES_INDEX_NO	(4)

void UI_Version_02(void)
{
	char 				temp_text[10];
	int 				temp_len;
	static uint32_t		res_value[3] = { 1, 1, 1 }, res_step[3] = { 1, 1, 1 };
	static uint8_t		res_index = 3;		// default at Value-R menu
	static uint16_t		res_2_power_N = 0;

	if(	State_Proc_Button(BUTTON_SRC_ID) )
	{
		if(++res_index>RES_INDEX_NO)
			res_index = 0;

		switch(res_index)			// next state after button pressed
		{
			case 0:			// changing RA under all VR menu
				lcm_text_buffer_cpy(LCM_ALL_SET_BLINKING,0,5,"A",1);
				lcd_module_display_enable_only_one_page(LCM_ALL_SET_BLINKING);
				lcd_module_display_enable_page(LCM_ALL_SET_R0_BLINKING);
				break;
			case 1:			// changing RB under all VR menu
				lcm_text_buffer_cpy(LCM_ALL_SET_BLINKING,0,5,"B",1);
				lcd_module_display_enable_only_one_page(LCM_ALL_SET_BLINKING);
				lcd_module_display_enable_page(LCM_ALL_SET_R1_BLINKING);
				break;
			case 2:			// changing RC under all VR menu
				lcm_text_buffer_cpy(LCM_ALL_SET_BLINKING,0,5,"C",1);
				lcd_module_display_enable_only_one_page(LCM_ALL_SET_BLINKING);
				lcd_module_display_enable_page(LCM_ALL_SET_R2_BLINKING);
				break;
			case 3:			// Value-R menu
				lcd_module_display_enable_only_one_page(LCM_ALL_VR_DISPLAY);
				break;
			case 4:			// Value-R menu
				lcd_module_display_enable_only_one_page(LCM_ALL_SET_2N_VALUE);
				break;
		}
	}

	if(	State_Proc_Button(BUTTON_SEL_ID) )
	{
		// to be implemented
	}

	if(	State_Proc_Button(BUTTON_ISP_ID) )
	{
		// Reserved for debug purpose
	}

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
			lcm_force_to_display_page(LCM_ALL_SET_BLINKING);
		}
		if(	State_Proc_Button(BUTTON_DEC_ID) )
		{
			uint32_t	*res_ptr = res_value + res_index,
						*step_ptr = res_step + res_index;

			*res_ptr = Update_Resistor_Value_after_button(*res_ptr,*step_ptr,false);
			temp_len = Show_Resistor_3_Digits(*res_ptr,temp_text);
			UI_V2_Update_after_change(res_index,*res_ptr,temp_text,temp_len);
			lcm_force_to_display_page(LCM_ALL_SET_BLINKING);
		}
	}
	else if (res_index == 4)
	{
		char *update_str;

		if(	State_Proc_Button(BUTTON_INC_ID) )
		{
			++res_2_power_N;
			if(res_2_power_N>=20)
			{
				res_2_power_N = 0;
			}
			update_str = FineTuneResistorMsg[res_2_power_N*2];
			lcm_text_buffer_cpy(LCM_ALL_SET_2N_VALUE,   0,0, update_str, LCM_DISPLAY_COL);
			update_str = FineTuneResistorMsg[res_2_power_N*2+1];
			lcm_text_buffer_cpy(LCM_ALL_SET_2N_VALUE,   1,0, update_str, LCM_DISPLAY_COL);
		}
		if(	State_Proc_Button(BUTTON_DEC_ID) )
		{
			if(res_2_power_N==0)
			{
				res_2_power_N = (20-1);
			}
			else
			{
				res_2_power_N--;
			}
			update_str = FineTuneResistorMsg[res_2_power_N*2];
			lcm_text_buffer_cpy(LCM_ALL_SET_2N_VALUE,   0,0, update_str, LCM_DISPLAY_COL);
			update_str = FineTuneResistorMsg[res_2_power_N*2+1];
			lcm_text_buffer_cpy(LCM_ALL_SET_2N_VALUE,   1,0, update_str, LCM_DISPLAY_COL);
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


///
///
///


