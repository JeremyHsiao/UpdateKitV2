/*
 * adc.h
 *
 *  Created on: 2018年11月26日
 *      Author: jeremy.hsiao
 */

#ifndef ADC_H_
#define ADC_H_

extern bool sequenceComplete, thresholdCrossed;
extern uint32_t	adc_voltage;

#define BOARD_ADC_CH 0
#define	ADC_SAMPLE_ERROR_VALUE		(~0UL)	// all ff in hex
#define	ADC_VREFP_VALUE_DEFAULT		(332)
#define ADC_VREFP_DIVIDER			(100)
#define ADC_RESULT_VOLTAGE_MUL		(1000)
#define ADC_RESOLUTION_BIT			(10)

extern void Init_ADC(void);
extern void DeInit_ADC(void);
extern void Set_ADC_VREFP(uint32_t vrefp_value);
extern uint32_t Get_ADC_VREFP(void);
extern uint32_t Read_ADC_Voltage(void);

#endif /* ADC_H_ */
