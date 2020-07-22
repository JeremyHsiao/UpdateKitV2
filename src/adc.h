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
#define	ADC_VREFP_VALUE				(332)
#define ADC_VREFP_DIVIDER			(100)

extern void Init_ADC(void);
extern void DeInit_ADC(void);
extern uint32_t Read_ADC_Voltage(void);

#endif /* ADC_H_ */
