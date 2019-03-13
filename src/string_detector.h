/*
 * string_detector.h
 *
 *  Created on: 2018年11月27日
 *      Author: Jeremy.Hsiao
 */

#ifndef STRING_DETECTOR_H_
#define STRING_DETECTOR_H_

extern void reset_string_detector(void);
extern bool locate_OK_pattern_process(char input_ch);
extern bool OK_pattern_process(char input_ch);
extern uint32_t Read_OK_Count(void);
extern void Clear_OK_pattern_state(void);

extern void locate_POWERON_pattern_process(char input_ch);
extern bool Get_POWERON_pattern(void);
extern void Clear_POWERON_pattern(void);

void locate_VER_pattern_process(char input_ch);
extern bool Found_VER_string(void);
extern void Clear_VER_string(void);
extern uint8_t *Get_VER_string(void);
#define	MAX_VER_NO_LEN	(96)

#define MAX_SERIAL_GETS_LEN			(32)
extern char *serial_gets(char input_ch);

#endif /* STRING_DETECTOR_H_ */
