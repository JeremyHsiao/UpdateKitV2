/*
 * voltage_output.h
 *
 *  Created on: 2019年3月14日
 *      Author: Jeremy.Hsiao
 */

#ifndef VOLTAGE_OUTPUT_H_
#define VOLTAGE_OUTPUT_H_

extern void Init_Value_From_EEPROM_for_voltage_output(void);
extern void OutputVoltageCurrentViaUART_Task(void);
extern void lcm_content_init_for_voltage_output(void);
extern bool Event_Proc_State_Independent_for_voltage_output(void);
extern UPDATE_STATE Event_Proc_by_System_State_for_voltage_output(UPDATE_STATE current_state);
extern UPDATE_STATE System_State_Begin_Proc_for_voltage_output(UPDATE_STATE current_state);
extern UPDATE_STATE System_State_Running_Proc_for_voltage_output(UPDATE_STATE current_state);
extern UPDATE_STATE System_State_End_Proc_for_voltage_output(UPDATE_STATE current_state);

extern void PWMOutputSetting(uint8_t current_pwm_sel);

#define				PWM_WELCOME_MESSAGE_IN_MS			(1000)
#define				USER_PWM_EEPROM_STORE_DELAY			(3)				// save value to EEPROM after 5 seconds without latest changes.

#define				MAX_DUTY_SELECTION_VALUE		(100)
#define				DUTY_SELECTION_OFFSET_VALUE		(1)
#define				PWM_OFF_DUTY_SELECTION_VALUE	(0)

extern uint8_t		voltage_output_welcome_message_line2[];

#endif /* VOLTAGE_OUTPUT_H_ */
