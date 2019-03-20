/*
 * event.c
 *
 *  Created on: 2018年12月25日
 *      Author: jeremy.hsiao
 */

#include "board.h"
#include "event.h"

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/
//bool		EVENT_raw_current_goes_above_threshold = false;
//bool		EVENT_raw_current_goes_below_threshold = false;
//bool		EVENT_filtered_current_goes_above_threshold = false;
//bool		EVENT_filtered_current_goes_below_threshold = false;
//bool		EVENT_filtered_current_above_threshold = false;
//bool		EVENT_filtered_current_below_threshold = false;
//bool		EVENT_filtered_current_TV_standby = false;
//bool		EVENT_filtered_current_no_output = false;
bool		EVENT_filtered_current_unplugged_debounced = false;
bool		EVENT_filtered_current_TV_standby_debounced = false;
bool		EVENT_filtered_current_above_fw_upgrade_threshold = false;
bool		EVENT_OK_string_confirmed = false;
bool		EVENT_Version_string_confirmed = false;
bool		EVENT_Button_pressed_debounced = false;
bool		EVENT_2nd_key_pressed_debounced = false;		// For voltage output branch
bool		EVENT_Enter_User_Ctrl_Mode = false;				// For voltage output branch

#ifdef POWERON_IS_DETECTING
bool		EVENT_POWERON_string_confirmed = false;
#endif // #ifdef POWERON_IS_DETECTING

/*****************************************************************************
 * Private functions
 ****************************************************************************/

/*****************************************************************************
 * Public functions
 ****************************************************************************/
