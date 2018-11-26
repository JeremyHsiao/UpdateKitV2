/*

	UpdateKitV2 main code

 */

#include "chip.h"
#include "board.h"
#include "string.h"
#include "pwm.h"
#include "uart_0_rb.h"
#include "gpio.h"

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/

/*****************************************************************************
 * Private functions
 ****************************************************************************/

/*****************************************************************************
 * Public functions
 ****************************************************************************/

/**
 * @brief	Main UpdateKitV2 program body
 * @return	Always returns 1
 */
int main(void)
{
	uint8_t key, dutyCycle = 50;  	/* Start at 50% duty cycle */
	int bytes, countdir = 10;

	SystemCoreClockUpdate();
	Board_Init();
	//Init_UART_PinMux();
	//Board_LED_Set(0, false);
	Init_GPIO();
	Init_PWM();
	Init_UART0();

	/* Poll the receive ring buffer for the ESC (ASCII 27) key */
	key = 0;
	while (key != 27) {
		bytes = UART0_GetChar(&key);
		if (bytes > 0) {
			/* Wrap value back around */
			UART0_PutChar((char)key);
		}

		if(GPIOGoup0_Int==true)
		{
			GPIOGoup0_Int = false;

			dutyCycle += countdir;
			if ((dutyCycle  == 0) || (dutyCycle >= 100)) {
				countdir = -countdir;
			}

			/* Update duty cycle in SCT/PWM by change match 1 reload time */
			setPWMRate(0, dutyCycle);
			setPWMRate(1, dutyCycle);
			setPWMRate(2, dutyCycle);
		}
	}

	/* DeInitialize peripherals before ending */
	DeInit_UART0();
	DeInit_PWM();
	DeInit_GPIO();
	return 1;
}
