/*
 * eeprom.h
 *
 *  Created on: 2019年1月2日
 *      Author: jeremy.hsiao
 */

#ifndef USER_OPT_H_
#define USER_OPT_H_

extern int Load_Resistor_Value(void);
extern bool Check_if_Resistor_different_from_last_ReadWrite(void);
extern bool Save_Resistor_Value(void);
extern void Update_VR_Page_value_at_beginig();

#endif /* USER_OPT_H_ */
