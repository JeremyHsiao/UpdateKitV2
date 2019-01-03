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
extern bool		EVENT_filtered_current_above_threshold;
extern bool		EVENT_filtered_current_below_threshold;
extern bool		EVENT_filtered_current_TV_standby;
extern bool		EVENT_filtered_current_no_output;
extern bool		EVENT_OK_string_confirmed;
extern bool		EVENT_Version_string_confirmed;
extern bool		EVENT_POWERON_string_confirmed;
extern bool		EVENT_Button_pressed_debounced;

#define EVENTS_IN_USE_AND_FLAG()	(EVENT_filtered_current_above_threshold && EVENT_filtered_current_below_threshold && EVENT_filtered_current_no_output && EVENT_filtered_current_TV_standby &&				\
							 	 	 EVENT_OK_string_confirmed && EVENT_Version_string_confirmed && EVENT_Button_pressed_debounced)

#define EVENTS_IN_USE_OR_FLAG()		(EVENT_filtered_current_above_threshold || EVENT_filtered_current_below_threshold || EVENT_filtered_current_no_output || EVENT_filtered_current_TV_standby ||				\
							 	 	 EVENT_OK_string_confirmed || EVENT_Version_string_confirmed || EVENT_Button_pressed_debounced)

#endif /* EVENT_H_ */
