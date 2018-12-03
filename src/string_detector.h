/*
 * string_detector.h
 *
 *  Created on: 2018年11月27日
 *      Author: Jeremy.Hsiao
 */

#ifndef STRING_DETECTOR_H_
#define STRING_DETECTOR_H_

extern void reset_string_detector(void);
extern uint32_t locate_OK_pattern_process(char input_ch);

extern void locate_POWERON_pattern_process(char input_ch);
extern bool Get_POWERON_pattern(void);
extern void Clear_POWERON_pattern(void);

#endif /* STRING_DETECTOR_H_ */
