//******************************************************************************
//   MSP430 USCI I2C Transmitter and Receiver
//
//  Description: This code configures the MSP430's USCI module as 
//  I2C master capable of transmitting and receiving bytes. 
//
//  ***THIS IS THE MASTER CODE***
//
//                    Master                   
//                 MSP430F2619             
//             -----------------          
//         /|\|              XIN|-   
//          | |                 |     
//          --|RST          XOUT|-    
//            |                 |        
//            |                 |        
//            |                 |       
//            |         SDA/P3.1|------->
//            |         SCL/P3.2|------->
//
// Note: External pull-ups are needed for SDA & SCL
//
// Uli Kretzschmar
// Texas Instruments Deutschland GmbH
// November 2007
// Built with IAR Embedded Workbench Version: 3.42A
//******************************************************************************
//#include "msp430x26x.h"                        // device specific header
//#include "msp430x22x4.h"
//#include "msp430x23x0.h"
//#include "msp430xG46x.h"
// ...                                         // more devices are possible

#include <msp430.h>
#include <signal.h>
#include "TI_USCI_I2C_master.h"

signed char byteCtr;
// set to UCTXSTT for RESTART condition, or 0 for hold
unsigned char stop_condition=UCTXSTP;
unsigned char *TI_receive_field;
unsigned char *TI_transmit_field;


//------------------------------------------------------------------------------
// void TI_USCI_I2C_receive(unsigned char byteCount, unsigned char *field)
//
// This function is used to start an I2C commuincation in master-receiver mode. 
//
// IN:   unsigned char byteCount  =>  number of bytes that should be read
//       unsigned char *field     =>  array variable used to store received data
//       void *recieve_done()     =>  callback when recieve completes
//------------------------------------------------------------------------------
void TI_USCI_I2C_receive(unsigned char byteCount,
                         unsigned char *field) {
                         
/* I2C Master Receiver Mode

After initialization, master receiver mode is initiated by writing the desired
slave address to the UCBxI2CSA register, selecting the size of the slave address
with the UCSLA10 bit, clearing UCTR for receiver mode, and setting UCTXSTT
to generate a START condition. The USCI module checks if the bus is available,
generates the START condition, and transmits the slave address. As soon as the
slave acknowledges the address the UCTXSTT bit is cleared.

After the acknowledge of the address from the slave the first data byte from
the slave is received and acknowledged and the UCBxRXIFG flag is set. Data is
received from the slave ss long as UCTXSTP or UCTXSTT is not set. If UCBxRXBUF
is not read the master holds the bus during reception of the last data bit and
until the UCBxRXBUF is read. If the slave does not acknowledge the transmitted
address the not-acknowledge interrupt flag UCNACKIFG is set. The master must
react with either a STOP condition or a repeated START condition.

Setting the UCTXSTP bit will generate a STOP condition. After setting UCTXSTP,
a NACK followed by a STOPcondition is generated after reception of the data from
the slave, or immediately if the USCI module is currently waiting for UCBxRXBUF
to be read.

If a master wants to receive a single byte only, the UCTXSTP bit must be set
while the byte is being received. For this case, the UCTXSTT may be polled
to determine when it is cleared:

BIS.B #UCTXSTT,&UCBOCTL1 ;Transmit START cond.
POLL_STT BIT.B #UCTXSTT,&UCBOCTL1 ;Poll UCTXSTT bit
JC POLL_STT ;When cleared,
BIS.B #UCTXSTP,&UCB0CTL1 ;transmit STOP cond.

Setting UCTXSTT will generate a repeated START condition. In this case, UCTR may be set or cleared to configure transmitter or receiver, and a different slave address may be written into UCBxI2CSA if desired.

*/                       
  TI_receive_field = field;
  UCB0CTL1 &= ~UCTR; // receiving, not transmitting
  if ( byteCount == 1 ){
    byteCtr = 0 ;
    __disable_interrupt();
    UCB0CTL1 |= UCTXSTT;                      // I2C start condition
    while (UCB0CTL1 & UCTXSTT);               // Start condition sent?
    UCB0CTL1 |= stop_condition;               // I2C stop condition
    stop_condition = UCTXSTP;                 // set stop on next transfer
    __enable_interrupt();
  } else if ( byteCount > 1 ) {
    byteCtr = byteCount - 1;
    UCB0CTL1 |= UCTXSTT;                      // I2C start condition
  } else
    while(1);                                  // illegal parameter
}

//------------------------------------------------------------------------------
// void TI_USCI_I2C_transmit(unsigned char byteCount, unsigned char *field)
//
// This function is used to start an I2C commuincation in master-transmit mode. 
//
// IN:   unsigned char byteCount  =>  number of bytes that should be transmitted
//       unsigned char *field     =>  array variable. Its content will be sent.
//------------------------------------------------------------------------------
void TI_USCI_I2C_transmit(unsigned char byteCount, unsigned char *field){
  TI_transmit_field = field;
  byteCtr = byteCount;
  UCB0CTL1 |= UCTR + UCTXSTT;                 // I2C TX, start condition
}

/* send start condition after transfer finishes instead of stop condition */
void TI_USCI_I2C_restart(void) {
  stop_condition = UCTXSTT;
}

// when hold is set, transmit will not send a stop condition afterwards
void TI_USCI_I2C_hold(void) {
  stop_condition = 0;
}

/* To A0/B0 I2C transmit/recieve shared vector: UCB0RXIFG, UCB0TXIFG flags*/
void TI_USCI_I2C_rx_tx_isr(void) {
	switch(__even_in_range(UCB0IV,12))
	{
	case 4:  // NACK
		if (UCB0STAT & UCNACKIFG) {            // send STOP if slave sends NACK
			UCB0CTL1 |= UCTXSTP;
			UCB0STAT &= ~UCNACKIFG;
		}
		break;
	case 10: // UCB0_RX
		if (byteCtr == 0) {
			UCB0CTL1 |= stop_condition;            // I2C stop condition
			stop_condition = UCTXSTP;
		}
		*TI_receive_field = UCB0RXBUF;
		TI_receive_field++;
		byteCtr--;
		break;
	case 12: // UCB0_RX
	    if (byteCtr == 0){
			UCB0CTL1 |= stop_condition;             // I2C stop condition
			stop_condition = UCTXSTP;
		}
		else {
			UCB0TXBUF = *TI_transmit_field;
			TI_transmit_field++;
	    }
	    byteCtr--;
	    break;
	default:
		break;
	}
}


