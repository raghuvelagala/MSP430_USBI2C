#include <msp430.h> 
#include <stdint.h>
#include <math.h>

/*
 * main.c
 */


void write(uint8_t byte);
void stc();
void read(uint8_t *data, uint8_t count);
void assign_data();
void compute_pressure();

float a0 = 0;
float b1 = 0;
float b2 = 0;
float c12 = 0;
int padc = 0;
int tadc = 0;
float pcomp = 0;
float i2c_pkpa = 0;				//pressure kPa
float i2c_temp = 0;			//temp.
int i;


uint8_t data[10];

int main(void) {
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer

    P4SEL |= BIT2|BIT1;
    P4REN |= BIT2|BIT1;
    P4OUT |= BIT2|BIT1;

    UCB1CTL1 |= UCSWRST;
    UCB1CTL1 |= UCSSEL__SMCLK;				//SMCLK
    UCB1CTL0 |= UCMST|UCMODE_3|UCSYNC;		//Master, I2C, SYNC mode
    UCB1I2CSA = 0x60;				//slave addr.
    UCB1BR0 = 0x10;
    UCB1BR1 = 0;
    UCB1CTL1 &= ~UCSWRST;

//    read();
    write(0x04);
	for(i=0;i<0xfff;i++);
    read(data, 8);//read 8 co-efficients
    assign_data();

    while(1)
    {
    	stc();
    	for(i=0;i<0xfff;i++);
        write(0x0);//read from the pressure register
    	for(i=0;i<0xfff;i++);
        read(data, 5);
        compute_pressure();

//    	 write(0x10);
//    	for(i=0;i<0xfff;i++);
//		read(data, 8);//read 8 co-efficients
//		stc();
    }


	return 0;
}

void write(uint8_t byte){
	while(UCB1CTL1 & UCTXSTP);

	UCB1CTL1 |= UCTR;		//transmitter
	UCB1CTL1 |= UCTXSTT;		//start condition

	while(!(UCB1IFG & UCTXIFG ));
	UCB1TXBUF = byte;		//a0 MSB

	while(!(UCTXIFG & UCB1IFG));
	UCB1CTL1 |= UCTXSTP;		//stop condition

}

void stc(){
	while(UCB1CTL1 & UCTXSTP);

	UCB1CTL1 |= UCTR;		//transmitter
	UCB1CTL1 |= UCTXSTT;		//start condition

	while(!(UCTXIFG & UCB1IFG));
	UCB1TXBUF = 0x12;		//start conversion command

	while(!(UCTXIFG & UCB1IFG));
	UCB1TXBUF = 0x00;

	while(!(UCTXIFG & UCB1IFG));
	UCB1CTL1 |= UCTXSTP;		//stop condition

}

void read(uint8_t *data, uint8_t count){
	int i = 0;

	UCB1CTL1 &= ~UCTR;		//Receiver
	UCB1CTL1 |= UCTXSTT;		//start condition

	for(i=0;i<count;i++){
		while(!(UCRXIFG & UCB1IFG));
		data[i] = UCB1RXBUF;
	}

	UCB1CTL1 |= UCTXSTP;		//stop condition


}

void assign_data(){
	a0 = 0;
	b1 = 0;
	b2 = 0;
	c12 = 0;
	unsigned a0temp = (data[0]<<8) + data[1];
	unsigned b1temp = (data[2]<<8) + data[3];
	unsigned b2temp = (data[4]<<8) + data[5];
	unsigned c12temp = (data[6]<<8) + data[7];

	__no_operation();
	//---------------a0-------------------
	//negative?
	if( (data[0] & BIT7) != 0){
		//temp = ~temp + 1;
		//a0 *= -1;
		a0 = -1*powf(2, 12);
	}


	//integer bits
	for(i=0;i<12;i++){
		if( (a0temp & (BIT3<<i)) != 0 ){
			a0 += powf(2,i);
		}
	}
	//fractional
	for(i=0;i<3;i++){
		if( (a0temp & (BIT0<<(2-i))) != 0 ){
			a0 += powf(2, -1*(i+1));
		}
	}

	//-----------------b1---------------------------
	//negative
	if( (data[2] & BIT7) != 0 ){
		b1 = -1*powf(2, 2);
	}

	//integer bits
	for(i=0;i<2;i++){
		if( (b1temp & (BITD<<i) ) != 0 ){
			b1 += powf(2,i);
		}
	}

	//fractional bits
	for(i=0;i<13;i++){
		if( (b1temp & (BITC>>i) ) != 0 ){
			b1 += powf(2, -1*(i+1));
		}
	}

	//---------------b2-------------------
	//negative
	if( (data[4] & BIT7) != 0 ){
		b2 = -1*powf(2, 1);
	}

	//integer bits
	if( (b2temp & BITE) != 0){
		b2 += powf(2, 0);
	}

	//fractional bits
	for(i=0;i<14;i++){
		if( (b2temp & (BITD>>i)) != 0 ){
			b2 += powf(2, -1*(i+1));
		}
	}

	//--------------c12---------------------
	//negative
	if( (data[6] & BIT7) != 0 ){
		c12 = -1*powf(2, 1);
	}

	//fractional bits
	for(i=0;i<13;i++){
		if( (c12temp & (BITE>>i)) != 0 ){
			c12 += powf(2, -1*(i+10));
		}
	}

}

void compute_pressure(){
	padc = (data[1]<<8) + data[2];
	padc = (padc>>6) & (BIT9|BIT8|BIT7|BIT6|BIT5|BIT4|BIT3|BIT2|BIT1|BIT0);
	tadc = (data[3]<<8) + data[4];
	tadc = (tadc>>6) & (BIT9|BIT8|BIT7|BIT6|BIT5|BIT4|BIT3|BIT2|BIT1|BIT0);

	pcomp = a0 + ((b1+(c12*tadc))*padc) + (b2*tadc);
	i2c_pkpa = (pcomp*((115-50)/1023.00)) + 50;

	i2c_temp = ((tadc - 498.00)/(-5.35)) + 25.0;

}


