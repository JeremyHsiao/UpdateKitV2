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

void Init_ADC(void);
void DeInit_ADC(void);

#endif /* ADC_H_ */
