/*
 * event.h
 *
 *  Created on: 2018年12月25日
 *      Author: jeremy.hsiao
 */

#ifndef EVENT_H_
#define EVENT_H_

extern bool		EVENT_raw_current_goes_above_threshold;
extern bool		EVENT_raw_current_goes_below_threshold;
extern bool		EVENT_filtered_current_goes_above_threshold;
extern bool		EVENT_filtered_current_goes_below_threshold;
extern bool		EVENT_OK_string_confirmed;
extern bool		EVENT_Version_string_confirmed;
extern bool		EVENT_POWERON_string_confirmed;
extern bool		EVENT_Button_pressed_debounced;

#endif /* EVENT_H_ */
