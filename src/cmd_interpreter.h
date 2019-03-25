/*
 * cmd_interpreter.h
 *
 *  Created on: 2018年11月27日
 *      Author: Jeremy.Hsiao
 */

#ifndef _CMD_INTERPRETER_H_
#define _CMD_INTERPRETER_H_

#define MAX_SERIAL_GETS_LEN			(32)
#define MAX_RETURN_STR_LEN			(16)
typedef		uint32_t			CmdExecutionPacket;
#define     CmdPacketLen        (32)
//typedef		uint64_t			CmdExecutionPacket;
//#define     CmdPacketLen        (64)

extern void init_cmd_interpreter(void);
extern char *serial_gets(char input_ch);
extern bool CheckIfUserCtrlModeCommand(char *input_str);
extern char *trimwhitespace(char *str);
extern bool CommandInterpreter(char *input_str, CmdExecutionPacket* cmd_packet);
extern bool CommandExecution(CmdExecutionPacket cmd_packet, char **return_string);
extern void EchoEnable(bool enabled);
extern bool CheckEchoEnableStatus(void);
extern void SetUserCtrlModeFlag(bool flag);

extern CmdExecutionPacket		received_cmd_packet;

#endif /* _CMD_INTERPRETER_H_ */
