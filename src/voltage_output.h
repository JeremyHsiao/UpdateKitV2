/*
 * voltage_output.h
 *
 *  Created on: 2019年3月14日
 *      Author: Jeremy.Hsiao
 */

#ifndef VOLTAGE_OUTPUT_H_
#define VOLTAGE_OUTPUT_H_

extern void Init_OutputVoltageCurrent_variables(void);
extern void OutputVoltageCurrentViaUART_Task(void);
extern void lcm_content_init_for_voltage_output(void);
extern bool Event_Proc_State_Independent_for_voltage_output(void);
extern UPDATE_STATE Event_Proc_by_System_State_for_voltage_output(UPDATE_STATE current_state);
extern UPDATE_STATE System_State_Begin_Proc_for_voltage_output(UPDATE_STATE current_state);
extern UPDATE_STATE System_State_Running_Proc_for_voltage_output(UPDATE_STATE current_state);
extern UPDATE_STATE System_State_End_Proc_for_voltage_output(UPDATE_STATE current_state);


#endif /* VOLTAGE_OUTPUT_H_ */
