/***********************************************************************
* $Id:: main.c 221 2012-01-13 02:54:53Z usb06052                              $
*
* Project: USB device ROM Stack test module
*
* Description:
*     USB Communication Device Class User module.
*
***********************************************************************
*   Copyright(C) 2011, NXP Semiconductor
*   All rights reserved.
*
* Software that is described herein is for illustrative purposes only
* which provides customers with programming information regarding the
* products. This software is supplied "AS IS" without any warranties.
* NXP Semiconductors assumes no responsibility or liability for the
* use of the software, conveys no license or title under any patent,
* copyright, or mask work right to the product. NXP Semiconductors
* reserves the right to make changes in the software without
* notification. NXP Semiconductors also make no representation or
* warranty that such application will be suitable for the specified
* use without further testing or modification.
**********************************************************************/
#include <string.h>
#include <stdint.h>
#include "LPC11Uxx.h"            
#include "usb_cdc.h"

enum
{
	StateIdle,
	StateCmdReceived,	// Entire command received: go parse it
};

#define CMD_BUF_SIZE 200
uint8_t commandBuffer[CMD_BUF_SIZE];
volatile uint16_t commandBufferPtr;
volatile uint16_t state;


#if defined(UART_BRIDGE)
void init_uart1_bridge(VCOM_DATA_T* pVcom, CDC_LINE_CODING* line_coding)
{
	if( line_coding)
	{
		uint32_t Fdiv, baud = 9600;
		uint8_t  lcr = 0x3;    	/* 8 bits, no Parity, 1 Stop bit */
		if ( line_coding->bCharFormat)
			lcr |= (1 << 2);                 /* Number of stop bits */

		if ( line_coding->bParityType) { /* Parity bit type */
			lcr |= (1 << 3);
			lcr |=  (((line_coding->bParityType - 1) & 0x3) << 4);
		}
		if ( line_coding->bDataBits) {      /* Number of data bits */
			lcr |= ((line_coding->bDataBits - 5) & 0x3);
		}
		else {
			lcr |= 0x3;
		}
		baud = line_coding->dwDTERate;
		/* enable SOF after we are connected */
		pUsbApi->hw->EnableEvent(pVcom->hUsb, 0, USB_EVT_SOF, 1);

		// Set baud rate and line properties
		Fdiv = ( (SystemCoreClock/LPC_SYSCON->UARTCLKDIV) / 16 ) / baud ;	//baud rate
		LPC_USART->IER = 0;
		LPC_USART->LCR = lcr | 0x80; 	// DLAB = 1
		LPC_USART->DLM = Fdiv / 256;
		LPC_USART->DLL = Fdiv % 256;
		LPC_USART->LCR = lcr;			// DLAB = 0
		LPC_USART->FCR = 0x07;		// Enable and reset TX and RX FIFO. Rx trigger level 4 chars
	}
	else
	{
		uint32_t regVal;
		// Initialize UART for 9600 baud, 8-N-1 on RX=PIO1_26, TX=PIO1_27

		// Configure PIO1_26/p10 = RX and PIO1_27/p9 = TX
		LPC_IOCON->PIO1_26 &= ~0x07;	// Clear func bits
		LPC_IOCON->PIO1_26 |= 0x02;		// RXD
		LPC_IOCON->PIO1_27 &= ~0x07;	// Clear func bits
		LPC_IOCON->PIO1_27 |= 0x02;		// TXD

		// Clock
		LPC_SYSCON->SYSAHBCLKCTRL |= (1<<12);	// USART clock enable
		LPC_SYSCON->UARTCLKDIV = 0x1;			// Divide by 1

		// Delay?
		for (regVal = 0; regVal < 100000; regVal++)
			__NOP();

		// USART mode
		LPC_USART->LCR = 0x83;					// 8-N-1 + enable access to Divisor latches

		// Baud rate
		/* The following is calculated for 9600 baud on a 48M PCLK.
		 * It results in an actual baud rate of 9600 for an error rate of 0%.
		 * Refer to section 12.5.14.1 of the manual.
		 * Use this form to calculate values: http://prototalk.net/forums/showthread.php?t=11
		 */
		LPC_USART->DLM = 0;
		LPC_USART->DLL = 250;
		LPC_USART->FDR = 0x41;		// MULVAL = 4, DIVADDVAL = 1
		LPC_USART->LCR = 0x03;		// Disable DLAB again

		// FIFO
		LPC_USART->FCR = 0x07;		// Enable and reset TX and RX FIFO

		// Line status
		regVal = LPC_USART->LSR;	// Read register to clear status

		// Ensure a clean start, no data in either TX or RX FIFO
		while( (LPC_USART->LSR & (LSR_THRE | LSR_TEMT)) != (LSR_THRE | LSR_TEMT) )
			;
		while( LPC_USART->LSR & LSR_RDR )
		{
			regVal = LPC_USART->RBR;	/* Dump data from RX FIFO */
		}
	}

	// Enable the UART Interrupt
	LPC_USART->IER = IER_RBR | IER_THRE | IER_RLS;	/* Enable UART interrupts: receive, line status and transmit */
}

/* Sends a string over UART with using VCOM_DATA_T structures.
 */
/// JWJ
void UARTSend(uint8_t *BufferPtr, uint32_t Length)
{
	while( Length != 0 )
	{
		// Only send if UART Transmit Holding Register Empty
		if (LPC_USART->LSR & LSR_THRE)
      	{
			LPC_USART->THR = *BufferPtr;
			BufferPtr++;
			Length--;
		}
	}
}

/* Sends data on UART.
 * Data buffer is pVcom->rxBuf and length is pVcom->rxlen.
 */
static void uart_write(VCOM_DATA_T* pVcom)
{
  uint8_t *pbuf = pVcom->rxBuf;
  uint32_t tx_cnt =  TX_FIFO_SIZE;
  /* find space in TX fifo */
  tx_cnt = 0xF - (tx_cnt & 0xF);

  if (tx_cnt > (pVcom->rxlen - pVcom->ser_pos)) {
    tx_cnt = (pVcom->rxlen - pVcom->ser_pos);
  }

  while(tx_cnt) {
      if (LPC_USART->LSR & LSR_THRE)
      {
        LPC_USART->THR = pbuf[pVcom->ser_pos++];
        tx_cnt--;
      }
  }

  /* if done check anything pending */
  if (pVcom->ser_pos == pVcom->rxlen) {
    /* Tx complete free the buffer */
    pVcom->ser_pos = 0;
    pVcom->rxlen = 0;
    if(pVcom->usbrx_pend) {
      pVcom->usbrx_pend = 0;
      VCOM_bulk_out_hdlr(pVcom->hUsb, (void*)pVcom, USB_EVT_OUT);
    }
  }
  return ;
}

static void uart_read(VCOM_DATA_T* pVcom)
{
  uint8_t *pbuf;

  pbuf = pVcom->txBuf;
  
  if( (LPC_USART->LSR & LSR_RDR) && (pVcom->txlen < USB_HS_MAX_BULK_PACKET) ) { 
    pbuf[pVcom->txlen++] = LPC_USART->RBR;
  } 

  if (pVcom->txlen == USB_HS_MAX_BULK_PACKET) {
    VCOM_usb_send(pVcom);
  }
  pVcom->last_ser_rx = pVcom->sof_counter;
}

/* Send received data on UART.
 * This method simply calls uart_write().
 * Set the send_func in VCOM_DATA struct in order to send data received on USB on UART (bridge mode).
 */
void VCOM_uart_send(VCOM_DATA_T* pVcom)
{
  /* data received on USB send it to UART */
  uart_write(pVcom);  
}

/*****************************************************************************
** Function name:		UART1_IRQHandler
**
** Descriptions:		UART1 interrupt handler
**
** parameters:			None
** Returned value:		None
** 
*****************************************************************************/
void UART_IRQHandler (void) 
{
  VCOM_DATA_T* pVcom = &g_vCOM;
  uint8_t intId, lsr;
  uint16_t serial_state = 0; 
	
  intId = (LPC_USART->IIR >> 1) & 0x07; /* check bit 1~3, interrupt identification */
    
  switch (intId)
  {
    case  IIR_RLS:    /* Receive Line Status */
   	  /* Read LSR will clear the interrupt */
      lsr = LPC_USART->LSR;
     	  /* There are errors or break interrupt update serial_state */
      if (lsr & LSR_OE)
        serial_state |= CDC_SERIAL_STATE_OVERRUN;

      if (lsr & LSR_PE)
        serial_state |= CDC_SERIAL_STATE_PARITY;

      if (lsr & LSR_FE)
        serial_state |= CDC_SERIAL_STATE_FRAMING;
        
      if (lsr & LSR_BI)
        serial_state |= CDC_SERIAL_STATE_BREAK;

      pUsbApi->cdc->SendNotification (pVcom->hCdc, CDC_NOTIFICATION_SERIAL_STATE, 
        serial_state);  
      break;
    case IIR_CTI: 	/* Character timeout indicator */
      /* send packet to USB */
      /* read chars until FIFO empty */
      uart_read(pVcom);
      break;
    case IIR_RDA:  /* Receive Data Available */
	pUsbApi->hw->WriteEP( pVcom->hUsb, USB_CDC_EP_BULK_IN, (uint8_t*)"USB test\r\n", 10 );
      /* 4 chars are available in FIFO */
      uart_read(pVcom);
      break;
    case  IIR_THRE: 	/* THRE, transmit holding register empty */
      if (pVcom->rxlen) {
          /* transmit */
          uart_write(pVcom);
      }
      break;
  }
  return;
}

/* USB serial has changed line coding.
 * Resets buffers and inits USB with new line coding.
 */
ErrorCode_t VCOM_SetLineCode (USBD_HANDLE_T hCDC, CDC_LINE_CODING* line_coding)
{
  VCOM_DATA_T* pVcom = &g_vCOM;
  //int i;
  /* baud rate change reset buffers */
  pVcom->ser_pos = 0;
  pVcom->rxlen = pVcom->txlen = 0;
  init_uart1_bridge(pVcom, line_coding);

  return LPC_OK;
}

ErrorCode_t VCOM_sof_event(USBD_HANDLE_T hUsb)
{
  VCOM_DATA_T* pVcom = &g_vCOM;
  uint8_t lcr;
  uint32_t diff = pVcom->sof_counter - pVcom->last_ser_rx;

  pVcom->sof_counter++;

  if (pVcom->break_time) {
    pVcom->break_time--;
    if (pVcom->break_time == 0) {
      lcr = LPC_USART->LCR;
      if (lcr & (1 << 6)) {
        lcr &= ~(1 << 6);
        LPC_USART->LCR = lcr;
      }
    }
  }

  if ( pVcom->last_ser_rx && (diff > 5)) {
      VCOM_usb_send(pVcom);
    }
    
  return LPC_OK;
}

#endif

/* Utility function for printing USB messages.
 */
void USB_CDC_print( char* string )
{
	USB_CDC_send( (uint8_t*)string, strlen( string ));
//	USB_CDC_send( (uint8_t*)"\r\n", 2 );
}

/* Parse the command in the command buffer and perform required operations.
 */
void parseCommand(void)
{
	if( strncasecmp( "sys", (char*)commandBuffer, 3 ) == 0 )
	{
		// SYS command
		USB_CDC_print( "Got SYS command.\r\n" );
	}
	else
	{
		// Unknown command
		USB_CDC_print( "ERROR unknown command\r\n" );
	}

	// Reset command buffer and state
	commandBufferPtr = 0;
	state = StateIdle;
}

/*****************************************************************************
**   Main Function  main()
*****************************************************************************/
int main (void)
{
    SystemCoreClockUpdate ();
    USB_CDC_init();

    // Init state machine
    state = StateIdle;

    // Init command buffer
    memset( commandBuffer, CMD_BUF_SIZE, 0 );
    commandBufferPtr = 0;

    // Send "ready"
 //   USB_CDC_send( (uint8_t*)"Ready...\r\n", 10 );

	// Empty main loop
	while( 1 )
	{
		switch( state )
		{
			case StateCmdReceived:
				parseCommand();
				break;

			default:
				break;
		}
	}
}

void USB_CDC_receive( uint8_t *bufferPtr, uint32_t length )
{
	// Copy into command buffer
	// Prevent buffer overflow
	if( commandBufferPtr + length > CMD_BUF_SIZE )
	{
		// Overflow: ignore this entire chunk (since it will be invalid anyway) and send error
		USB_CDC_print( "ERROR command buffer overflow!\r\n" );

		// Reset command buffer
		commandBufferPtr = 0;
		return;
	}

	// No overflow: copy data into command buffer
	memcpy( commandBuffer+commandBufferPtr, bufferPtr, length );
	commandBufferPtr += length;

	// Check if we have received \n to terminate command
	if( commandBuffer[ commandBufferPtr-1 ] == '\n' )
		// Yes, we have: parse the command
		state = StateCmdReceived;

	// Data received: echo
//	USB_CDC_send( bufferPtr, length );
}

/**********************************************************************
 **                            End Of File
 **********************************************************************/
