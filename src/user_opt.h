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

#endif /* USER_OPT_H_ */
