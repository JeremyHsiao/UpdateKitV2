/*
 * uart_0_rb.h
 *
 *  Created on: 2018年11月26日
 *      Author: jeremy.hsiao
 */

#ifndef UART_0_RB_H_
#define UART_0_RB_H_

extern void Init_UART0(void);
extern void DeInit_UART0(void);
extern int UART0_GetChar(void *return_ch);
extern uint32_t UART0_PutChar(char ch);
extern int OutputHexValue(uint32_t value);
extern int OutputHexValue_with_newline(uint32_t value);
extern int OutputString(char *str);
extern int OutputString_with_newline(char *str);

#endif /* UART_0_RB_H_ */
