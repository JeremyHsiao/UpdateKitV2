/*
 * user_if.c
 *
 *  Created on: Jun 4, 2020
 *      Author: Jerem
 */

#include "chip.h"
#include "fw_version.h"
#include "lcd_module.h"
#include "string.h"
#include "res_state.h"
#include "sw_timer.h"
#include "user_if.h"
#include "gpio.h"

#ifdef _REAL_UPDATEKIT_V2_BOARD_

#define BUTTON_SRC_PORT		SWITCH_KEY_GPIO_PORT
#define BUTTON_SRC_PIN		SWITCH_KEY_GPIO_PIN
#define BUTTON_DEC_PORT		SECOND_KEY_GPIO_PORT
#define BUTTON_DEC_PIN		SECOND_KEY_GPIO_PIN
#define BUTTON_INC_PORT		SWITCH_KEY_GPIO_PORT
#define BUTTON_INC_PIN		SWITCH_KEY_GPIO_PIN
#define BUTTON_SEL_PORT		SECOND_KEY_GPIO_PORT
#define BUTTON_SEL_PIN		SECOND_KEY_GPIO_PIN

#else

#define BUTTON_SRC_PORT		SWITCH_KEY_GPIO_PORT
#define BUTTON_SRC_PIN		SWITCH_KEY_GPIO_PIN
#define BUTTON_DEC_PORT		SECOND_KEY_GPIO_PORT
#define BUTTON_DEC_PIN		SECOND_KEY_GPIO_PIN
#define BUTTON_INC_PORT		SWITCH_KEY_GPIO_PORT
#define BUTTON_INC_PIN		SWITCH_KEY_GPIO_PIN
#define BUTTON_SEL_PORT		SECOND_KEY_GPIO_PORT
#define BUTTON_SEL_PIN		SECOND_KEY_GPIO_PIN

#endif // #ifdef _REAL_UPDATEKIT_V2_BOARD_

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

//	// PC Mode page		     			 1234567890123456
	lcm_text_buffer_cpy(LCM_PC_MODE,0,0,"PC Mode: Press  ", LCM_DISPLAY_COL);
	lcm_text_buffer_cpy(LCM_PC_MODE,1,0,"button to change", LCM_DISPLAY_COL);

	// enable/disable some page/
	memset((void *)lcd_module_display_enable, 0x00, LCM_MAX_PAGE_NO);	// Initial only - later sw determine which page is to be displayed

}

///
///
///

Button_Data const button_data[4] =
{
		{ BUTTON_SRC_PORT, BUTTON_SRC_PIN, BUTTON_SRC_STATE_IN_10MS, TIMER_10MS, BUTTON_SRC_REPEAT_IN_10MS, TIMER_10MS},
		{ BUTTON_DEC_PORT, BUTTON_DEC_PIN, BUTTON_DEC_STATE_IN_10MS, TIMER_10MS, BUTTON_DEC_REPEAT_IN_10MS, TIMER_10MS},
		{ BUTTON_INC_PORT, BUTTON_INC_PIN, BUTTON_INC_STATE_IN_10MS, TIMER_10MS, BUTTON_INC_REPEAT_IN_10MS, TIMER_10MS},
		{ BUTTON_SEL_PORT, BUTTON_SEL_PIN, BUTTON_SEL_STATE_IN_10MS, TIMER_10MS, BUTTON_SEL_REPEAT_IN_10MS, TIMER_10MS}
};

BUTTON_STATE button_state[4] = { BUTTON_UP_STATE, BUTTON_UP_STATE, BUTTON_UP_STATE, BUTTON_UP_STATE };

uint32_t next_state_10ms_lut[] =
{
		(4-1),	// BUTTON_UP_STATE = 0,					time before end of debounce == debounce-time
		(120-1), // BUTTON_DOWN_DEBOUNCE_STATE,			time before end of stepwise == no repeat during this period
		(360-1), // BUTTON_DOWN_STEPWISE_STATE,			time before end of accelerating == +1 fast
		(~0U), // BUTTON_ACCELERATING_STATE,			not used
		(~0U), // BUTTON_TURBO_STATE,					not used
		(4-1), // BUTTON_UP_DEBOUNCE_STATE,				time before end of up-debounce == start to detect next key pressed.
};

#define ACCLERATING_TICK	(12-1)		// unit: 10ms		+1 each 120ms
#define TURBO_TICK			(3-1)		// unit: 10ms		+1 each 30ms

bool State_Proc_Button(ButtonID button_index)
{
	Button_Data 	*button_ptr = button_data + button_index;
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
///
///
///


