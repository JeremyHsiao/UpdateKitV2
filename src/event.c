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
bool		EVENT_raw_current_goes_above_threshold = false;
bool		EVENT_raw_current_goes_below_threshold = false;
bool		EVENT_filtered_current_goes_above_threshold = false;
bool		EVENT_filtered_current_goes_below_threshold = false;
bool		EVENT_filtered_current_above_threshold = false;
bool		EVENT_filtered_current_below_threshold = false;
bool		EVENT_OK_string_confirmed = false;
bool		EVENT_Version_string_confirmed = false;
bool		EVENT_POWERON_string_confirmed = false;
bool		EVENT_Button_pressed_debounced = false;

/*****************************************************************************
 * Private functions
 ****************************************************************************/

/*****************************************************************************
 * Public functions
 ****************************************************************************/
