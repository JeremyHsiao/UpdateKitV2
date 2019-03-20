/*
 * event.h
 *
 *  Created on: 2018年12月25日
 *      Author: jeremy.hsiao
 */

#ifndef EVENT_H_
#define EVENT_H_

//extern bool		EVENT_raw_current_goes_above_threshold;
//extern bool		EVENT_raw_current_goes_below_threshold;
//extern bool		EVENT_filtered_current_goes_above_threshold;
//extern bool		EVENT_filtered_current_goes_below_threshold;
//extern bool		EVENT_filtered_current_above_threshold;
//extern bool		EVENT_filtered_current_below_threshold;
//extern bool		EVENT_filtered_current_TV_standby;
//extern bool		EVENT_filtered_current_no_output;
extern bool		EVENT_filtered_current_unplugged_debounced;
extern bool		EVENT_filtered_current_TV_standby_debounced;
extern bool		EVENT_filtered_current_above_fw_upgrade_threshold;
extern bool		EVENT_OK_string_confirmed;
extern bool		EVENT_Version_string_confirmed;
extern bool		EVENT_Button_pressed_debounced;
extern bool		EVENT_2nd_key_pressed_debounced;	// For voltage output branch
extern bool		EVENT_Enter_User_Ctrl_Mode;			// For voltage output branch

#ifdef POWERON_IS_DETECTING
extern bool		EVENT_POWERON_string_confirmed;
#endif // #ifdef POWERON_IS_DETECTING

#define EVENTS_IN_USE_AND_FLAG()	(EVENT_filtered_current_above_fw_upgrade_threshold && EVENT_filtered_current_unplugged_debounced && EVENT_filtered_current_TV_standby_debounced && 				\
							 	 	 EVENT_OK_string_confirmed && EVENT_Version_string_confirmed && EVENT_Button_pressed_debounced && EVENT_Enter_User_Ctrl_Mode)

#define EVENTS_IN_USE_OR_FLAG()		(EVENT_filtered_current_above_fw_upgrade_threshold || EVENT_filtered_current_unplugged_debounced || EVENT_filtered_current_TV_standby_debounced ||				\
							 	 	 EVENT_OK_string_confirmed || EVENT_Version_string_confirmed || EVENT_Button_pressed_debounced || EVENT_Enter_User_Ctrl_Mode)

#endif /* EVENT_H_ */
