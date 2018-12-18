/*
 * @brief LPC11u6x USART0 chip driver
 *
 * @note
 * Copyright(C) NXP Semiconductors, 2013
 * All rights reserved.
 *
 * @par
 * Software that is described herein is for illustrative purposes only
 * which provides customers with programming information regarding the
 * LPC products.  This software is supplied "AS IS" without any warranties of
 * any kind, and NXP Semiconductors and its licensor disclaim any and
 * all warranties, express or implied, including all implied warranties of
 * merchantability, fitness for a particular purpose and non-infringement of
 * intellectual property rights.  NXP Semiconductors assumes no responsibility
 * or liability for the use of the software, conveys no license or rights under any
 * patent, copyright, mask work right, or any other intellectual property rights in
 * or to any products. NXP Semiconductors reserves the right to make changes
 * in the software without notification. NXP Semiconductors also makes no
 * representation or warranty that such application will be suitable for the
 * specified use without further testing or modification.
 *
 * @par
 * Permission to use, copy, modify, and distribute this software and its
 * documentation is hereby granted, under NXP Semiconductors' and its
 * licensor's relevant copyrights in the software, without fee, provided that it
 * is used in conjunction with NXP Semiconductors microcontrollers.  This
 * copyright, permission, and disclaimer notice must appear in all copies of
 * this code.
 */

#include "chip.h"

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

/* Initializes the pUART peripheral */
void Chip_UART0_Init(LPC_USART0_T *pUART)
{
	/* A USART 0 divider of 1 is used with this driver */
	Chip_Clock_SetUSART0ClockDiv(1);

	Chip_Clock_EnablePeriphClock(SYSCTL_CLOCK_UART0);

	/* Enable FIFOs by default, reset them */
	Chip_UART0_SetupFIFOS(pUART, (UART0_FCR_FIFO_EN | UART0_FCR_RX_RS | UART0_FCR_TX_RS));

	/* Disable fractional divider */
	pUART->FDR = 0x10;
}

/* De-initializes the pUART peripheral */
void Chip_UART0_DeInit(LPC_USART0_T *pUART)
{
	Chip_Clock_DisablePeriphClock(SYSCTL_CLOCK_UART0);
}

/* Transmit a byte array through the UART peripheral (non-blocking) */
int Chip_UART0_Send(LPC_USART0_T *pUART, const void *data, int numBytes)
{
	int sent = 0;
	uint8_t *p8 = (uint8_t *) data;

	/* Send until the transmit FIFO is full or out of bytes */
	while ((sent < numBytes) &&
		   ((Chip_UART0_ReadLineStatus(pUART) & UART0_LSR_THRE) != 0)) {
		Chip_UART0_SendByte(pUART, *p8);
		p8++;
		sent++;
	}

	return sent;
}

/* Transmit a byte array through the UART peripheral (blocking) */
int Chip_UART0_SendBlocking(LPC_USART0_T *pUART, const void *data, int numBytes)
{
	int pass, sent = 0;
	uint8_t *p8 = (uint8_t *) data;

	while (numBytes > 0) {
		pass = Chip_UART0_Send(pUART, p8, numBytes);
		numBytes -= pass;
		sent += pass;
		p8 += pass;
	}

	return sent;
}

/* Read data through the UART peripheral (non-blocking) */
int Chip_UART0_Read(LPC_USART0_T *pUART, void *data, int numBytes)
{
	int readBytes = 0;
	uint8_t *p8 = (uint8_t *) data;

	/* Send until the transmit FIFO is full or out of bytes */
	while ((readBytes < numBytes) &&
		   ((Chip_UART0_ReadLineStatus(pUART) & UART0_LSR_RDR) != 0)) {
		*p8 = Chip_UART0_ReadByte(pUART);
		p8++;
		readBytes++;
	}

	return readBytes;
}

/* Read data through the UART peripheral (blocking) */
int Chip_UART0_ReadBlocking(LPC_USART0_T *pUART, void *data, int numBytes)
{
	int pass, readBytes = 0;
	uint8_t *p8 = (uint8_t *) data;

	while (numBytes > 0) {
		pass = Chip_UART0_Read(pUART, p8, numBytes);
		numBytes -= pass;
		readBytes += pass;
		p8 += pass;
	}

	return readBytes;
}

/* Determines and sets best dividers to get a target bit rate */
uint32_t Chip_UART0_SetBaud(LPC_USART0_T *pUART, uint32_t baudrate)
{
	uint32_t div, divh, divl, clkin, dval, mval;

	/* USART clock input divider of 1 */
	Chip_Clock_SetUSART0ClockDiv(1);

	/* Determine UART clock in rate without FDR */
	clkin = Chip_Clock_GetMainClockRate();
	div = clkin / (baudrate * 16);

	/* High and low halves of the divider */
	divh = div / 256;
	divl = div - (divh * 256);

	Chip_UART0_EnableDivisorAccess(pUART);
	Chip_UART0_SetDivisorLatches(pUART, divl, divh);
	Chip_UART0_DisableDivisorAccess(pUART);

#ifdef _REAL_UPDATEKIT_V2_BOARD_
	if(baudrate>=115200)
	{
		/* Set best fractional divider */
		dval=1;
		mval=12;
		pUART->FDR = (UART0_FDR_MULVAL(mval) | UART0_FDR_DIVADDVAL(dval));
		/* Return actual baud rate */
		return ( clkin / (16 * div + 16 * div * dval / mval) );
	}
	else
	{
		/* Set best fractional divider */
		dval=0;
		mval=1;
		pUART->FDR = (UART0_FDR_MULVAL(mval) | UART0_FDR_DIVADDVAL(dval));

		/* Return actual baud rate */
		return ( clkin / (16 * div + 16 * div * dval / mval) );
	}
#else
	/* Fractional FDR already setup for 1 in UART init */
	return clkin / (div * 16);
#endif // #ifdef _REAL_UPDATEKIT_V2_BOARD_

}

/* UART receive-only interrupt handler for ring buffers */
void Chip_UART0_RXIntHandlerRB(LPC_USART0_T *pUART, RINGBUFF_T *pRB)
{
	/* New data will be ignored if data not popped in time */
	while (Chip_UART0_ReadLineStatus(pUART) & UART0_LSR_RDR) {
		uint8_t ch = Chip_UART0_ReadByte(pUART);
		RingBuffer_Insert(pRB, &ch);
	}
}

/* UART transmit-only interrupt handler for ring buffers */
void Chip_UART0_TXIntHandlerRB(LPC_USART0_T *pUART, RINGBUFF_T *pRB)
{
	uint8_t ch;

	/* Fill FIFO until full or until TX ring buffer is empty */
	while ((Chip_UART0_ReadLineStatus(pUART) & UART0_LSR_THRE) != 0 &&
		   RingBuffer_Pop(pRB, &ch)) {
		Chip_UART0_SendByte(pUART, ch);
	}
}

/* Populate a transmit ring buffer and start UART transmit */
uint32_t Chip_UART0_SendRB(LPC_USART0_T *pUART, RINGBUFF_T *pRB, const void *data, int bytes)
{
	uint32_t ret;
	uint8_t *p8 = (uint8_t *) data;

	/* Don't let UART transmit ring buffer change in the UART IRQ handler */
	Chip_UART0_IntDisable(pUART, UART0_IER_THREINT);

	/* Move as much data as possible into transmit ring buffer */
	ret = RingBuffer_InsertMult(pRB, p8, bytes);
	Chip_UART0_TXIntHandlerRB(pUART, pRB);

	/* Add additional data to transmit ring buffer if possible */
	ret += RingBuffer_InsertMult(pRB, (p8 + ret), (bytes - ret));

	/* Enable UART transmit interrupt */
	Chip_UART0_IntEnable(pUART, UART0_IER_THREINT);

	return ret;
}

/* Copy data from a receive ring buffer */
int Chip_UART0_ReadRB(LPC_USART0_T *pUART, RINGBUFF_T *pRB, void *data, int bytes)
{
	(void) pUART;

	return RingBuffer_PopMult(pRB, (uint8_t *) data, bytes);
}

/* UART receive/transmit interrupt handler for ring buffers */
void Chip_UART0_IRQRBHandler(LPC_USART0_T *pUART, RINGBUFF_T *pRXRB, RINGBUFF_T *pTXRB)
{
	/* Handle transmit interrupt if enabled */
	if (pUART->IER & UART0_IER_THREINT) {
		Chip_UART0_TXIntHandlerRB(pUART, pTXRB);

		/* Disable transmit interrupt if the ring buffer is empty */
		if (RingBuffer_IsEmpty(pTXRB)) {
			Chip_UART0_IntDisable(pUART, UART0_IER_THREINT);
		}
	}

	/* Handle receive interrupt */
	Chip_UART0_RXIntHandlerRB(pUART, pRXRB);
}

/* Determines and sets best dividers to get a target baud rate */
uint32_t Chip_UART0_SetBaudFDR(LPC_USART0_T *pUART, uint32_t baudrate)

{
	uint32_t uClk;
	uint32_t dval, mval;
	uint32_t dl;
	uint32_t rate16 = 16 * baudrate;
	uint32_t actualRate = 0;

	/* Get Clock rate */
	uClk = Chip_Clock_GetMainClockRate();

	/* The fractional is calculated as (PCLK  % (16 * Baudrate)) / (16 * Baudrate)
	 * Let's make it to be the ratio DivVal / MulVal
	 */
	dval = uClk % rate16;

	/* The PCLK / (16 * Baudrate) is fractional
	 * => dval = pclk % rate16
	 * mval = rate16;
	 * now mormalize the ratio
	 * dval / mval = 1 / new_mval
	 * new_mval = mval / dval
	 * new_dval = 1
	 */
	if (dval > 0) {
		mval = rate16 / dval;
		dval = 1;

		/* In case mval still bigger then 4 bits
		 * no adjustment require
		 */
		if (mval > 12) {
			dval = 0;
		}
	}
	dval &= 0xf;
	mval &= 0xf;
	dl = uClk / (rate16 + rate16 * dval / mval);

	/* Update UART registers */
	Chip_UART0_EnableDivisorAccess(pUART);
	Chip_UART0_SetDivisorLatches(pUART, UART0_LOAD_DLL(dl), UART0_LOAD_DLM(dl));
	Chip_UART0_DisableDivisorAccess(pUART);

	/* Set best fractional divider */
	pUART->FDR = (UART0_FDR_MULVAL(mval) | UART0_FDR_DIVADDVAL(dval));

	/* Return actual baud rate */
	actualRate = uClk / (16 * dl + 16 * dl * dval / mval);
	return actualRate;
}

/*
 *
 * Table for FDR future use
 *
DVAL	MVAL	Result
1	14	0.071428571
1	13	0.076923077
1	12	0.083333333
1	11	0.090909091
1	10	0.1
1	9	0.111111111
1	8	0.125
2	14	0.142857143
2	13	0.153846154
2	12	0.166666667
2	11	0.181818182
2	10	0.2
3	14	0.214285714
2	9	0.222222222
3	13	0.230769231
3	12	0.25
2	8	0.25
3	11	0.272727273
4	14	0.285714286
3	10	0.3
4	13	0.307692308
4	12	0.333333333
3	9	0.333333333
5	14	0.357142857
4	11	0.363636364
3	8	0.375
5	13	0.384615385
4	10	0.4
5	12	0.416666667
6	14	0.428571429
4	9	0.444444444
5	11	0.454545455
6	13	0.461538462
7	14	0.5
6	12	0.5
5	10	0.5
4	8	0.5
7	13	0.538461538
6	11	0.545454545
5	9	0.555555556
8	14	0.571428571
7	12	0.583333333
6	10	0.6
8	13	0.615384615
5	8	0.625
7	11	0.636363636
9	14	0.642857143
8	12	0.666666667
6	9	0.666666667
9	13	0.692307692
7	10	0.7
10	14	0.714285714
8	11	0.727272727
9	12	0.75
6	8	0.75
10	13	0.769230769
7	9	0.777777778
11	14	0.785714286
8	10	0.8
9	11	0.818181818
10	12	0.833333333
11	13	0.846153846
12	14	0.857142857
7	8	0.875
8	9	0.888888889
9	10	0.9
10	11	0.909090909
11	12	0.916666667
12	13	0.923076923
13	14	0.928571429

 */
