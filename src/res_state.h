/*
 * res_state.h
 *
 *  Created on: May 22, 2020
 *      Author: Jeremy Hsiao
 */

#ifndef RES_STATE_H_
#define RES_STATE_H_

typedef enum
{
	US_SYSTEM_BOOTUP_STATE = 0,
	US_SYSTEM_WELCOME_STATE,
	US_USER_OPERATION_STATE,
	US_COMPUTER_CONTROL_STATE,
	US_MAX_STATE_NO
} UPDATE_STATE;


typedef enum
{
	BUTTON_UP_STATE = 0,
	BUTTON_DOWN_DEBOUNCE_STATE,
	BUTTON_DOWN_STEPWISE_STATE,
	BUTTON_ACCELERATING_STATE,
	BUTTON_TURBO_STATE,
	BUTTON_UP_DEBOUNCE_STATE,
	BUTTON_MAX_STATE_NO
} BUTTON_STATE;

#endif /* RES_STATE_H_ */
