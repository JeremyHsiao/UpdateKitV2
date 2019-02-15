/*
 * eeprom.h
 *
 *  Created on: 2019年1月2日
 *      Author: jeremy.hsiao
 */

#ifndef USER_OPT_H_
#define USER_OPT_H_

extern bool Load_User_Selection(uint8_t *pUserSelect);
extern bool Save_User_Selection(uint8_t UserSelect);
extern bool Check_if_different_from_last_ReadWrite(uint8_t UserSelect);
extern bool Load_System_Timeout_v2(uint8_t user_selection, uint16_t *pSystemTimeout);
extern bool Check_if_different_from_last_System_Timeout_v2(uint8_t user_selection, uint16_t timeout);
extern bool Save_System_Timeout_v2(uint8_t user_selection, uint16_t SystemTimeout);

#endif /* USER_OPT_H_ */
