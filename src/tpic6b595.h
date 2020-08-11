/*
 * tpic6b595.h
 *
 *  Created on: May 15, 2020
 *      Author: Jeremy
 */

#ifndef TPIC6B595_H_
#define TPIC6B595_H_

extern void Init_Shift_Register_GPIO(void);
extern void Enable_Shift_Register_Output(bool enable_true);
extern void Latch_Register_Byte_to_Output(void);
extern void Shift_and_Set_Register_Bit(bool set_bit_data);
extern void Clear_Register_Byte(void);
extern void Clear_Shiftout_log(void);
extern uint32_t Read_Shiftout_log(void);
extern uint32_t Setup_Shift_Register_32it(uint32_t value);
extern uint32_t Test_Shift_Register(uint8_t test_value);
extern bool Setup_Shift_Register_and_Test_64bit(uint64_t value);
extern void Setup_Shift_Register_64bit(uint64_t value);
extern bool SelfTest_Shift_Register(void);
extern void Calc_Relay_Value(uint32_t *Resistor, uint64_t *Relay);

extern bool get_disable_relay_control(void);
extern void set_disable_relay_control(bool set_disable);
extern bool get_tpic6b595_selftest_On(void);
extern void set_tpic6b595_selftest_On(bool OnOff);
extern uint32_t get_tpic6b595_selftest_Total_Count(void);
extern uint32_t get_tpic6b595_selftest_OK_Count(void);

#endif /* TPIC6B595_H_ */
