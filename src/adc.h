/*
 * adc.h
 *
 *  Created on: 2018年11月26日
 *      Author: jeremy.hsiao
 */

#ifndef ADC_H_
#define ADC_H_

extern bool sequenceComplete, thresholdCrossed;

#define BOARD_ADC_CH 0
#define	ADC_SAMPLE_ERROR_VALUE		(0xffff)	// length is 16 bits

extern void Init_ADC(void);
extern void DeInit_ADC(void);
extern void Read_ADC(void);

#endif /* ADC_H_ */
