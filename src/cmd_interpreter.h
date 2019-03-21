/*
 * cmd_interpreter.h
 *
 *  Created on: 2018年11月27日
 *      Author: Jeremy.Hsiao
 */

#ifndef _CMD_INTERPRETER_H_
#define _CMD_INTERPRETER_H_

#define MAX_SERIAL_GETS_LEN			(32)

extern void init_cmd_interpreter(void);
extern char *serial_gets(char input_ch);
extern bool CheckIfUserCtrlModeCommand(char *input_str);
extern char *trimwhitespace(char *str);
extern bool CommandProcessor(char *input_str);
extern int EchoInputString(char *input_str);
extern 	void EchoEnable(bool enabled);

#endif /* _CMD_INTERPRETER_H_ */
