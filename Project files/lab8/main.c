#include <string.h>
#include "driverlib.h"
#include "USB_config/descriptors.h"
#include "USB_API/USB_Common/device.h"
#include "USB_API/USB_Common/usb.h"                 // USB-specific functions
#include "USB_API/USB_CDC_API/UsbCdc.h"
#include "USB_app/usbConstructs.h"
#include <math.h>
#include <stdio.h>

#define MAX_STR_LENGTH 64

/* NOTE: Modify hal.h to select a specific evaluation board and customize for your own board */
#include "hal.h"

// Function declarations
uint8_t retInString (char* string);
void initTimer(void);
void convertTwoDigBinToASCII(uint8_t bin, uint8_t* str);

//-------------------------------------------------
void write(uint8_t byte);
void stc();
void read(uint8_t *data, uint8_t count);
void assign_data();
void compute_pressure();
void sensors_init();
void sensors_get_data();
void convert_data_to_bin();
void calc_lux();
void set_adc_temp();
void set_adc_lux();
void swap_arrays();

float a0 = 0;
float b1 = 0;
float b2 = 0;
float c12 = 0;
int padc = 0;
int tadc = 0;
float pcomp = 0;
float i2c_pkpa[5] = {10,20,10,45,50};				//pressure kPa
float i2c_temp[5] = {50,50,50,50,50};			//temp.
int i;
uint8_t data[10];

//DHT
#define TRIGGER_DELAY 590		//ACLK, /1, 18 ms
#define DHT_HIGH_THRESH 100
volatile unsigned int hum[5] = {50,98,45,50,99};					//DHT HUM
volatile unsigned int temp_dht[5] = {10,20,15,35,39};			//DHT TEMP
//dht var.
uint16_t new_cap = 0;
uint16_t old_cap = 0;
uint16_t cap_diff = 0;

uint16_t diff_arr[42];			//40 (bits) + 1 (20 us) + 1 (ACK)
uint16_t cap_arr[42];
uint16_t dht_index = 0;

//------ADC------------
#define LUX 1
#define TEMP 2
#define ADC_MAX 2900
#define CALADC12_15V_30C  *((unsigned int *)0x1A1A)   // Temperature Sensor Calibration-30 C
#define CALADC12_15V_85C  *((unsigned int *)0x1A1C)   // Temperature Sensor Calibration-85 C

volatile unsigned int adc_out;
volatile float lux_unit = ADC_MAX/100.00;
volatile unsigned int mode = LUX;
unsigned int adc_temp_intm;
volatile unsigned int temp_adc[5] = {10,10,10,10,10};			//ADC TEMP
volatile unsigned int lux[5]= {50,35,20,57,99};					//ADC LUX

// Global flags set by events
volatile uint8_t bCDCDataReceived_event = FALSE; // Indicates data has been rx'ed
                                              // without an open rx operation

char wholeString[MAX_STR_LENGTH] = ""; // Entire input str from last 'return'

uint8_t timeStr[2];
int index;

//declaration of 50 temperature values
//char *temp[50] = {"59","40","32","27","47","44","66","77","22","88","00","11","24","35","47","56","28","90","63","54","67","32","45","67",
//"11","43","54","68","31","91","02","09","34","37","37","78","90","56","43","78","65","43","29","04","36","47","12","34","67","80","50"};
char *temp[10] = {"23","54","76","45","56","98","78","93","23","45"};
//temp[1] =


/* ======== main ======== */
void main (void)
{

    WDT_A_hold(WDT_A_BASE); // Stop watchdog timer



    // Minumum Vcore setting required for the USB API is PMM_CORE_LEVEL_2 .
	#ifndef DRIVERLIB_LEGACY_MODE
		PMM_setVCore(PMM_CORE_LEVEL_2);
	#else
		PMM_setVCore(PMM_BASE, PMM_CORE_LEVEL_2);
	#endif

    initPorts();           // Config GPIOS for low-power (output low)
    initClocks(8000000);   // Config clocks. MCLK=SMCLK=FLL=8MHz; ACLK=REFO=32kHz

    //initTimer();           // Prepare timer for LED toggling
    USB_setup(TRUE, TRUE); // Init USB & events; if a host is present, connect

    char buffer[16] = "123";
    //int place = 10;
//    sprintf(buffer, "%d", place);
//    temp[0] = buffer;

    __enable_interrupt();  // Enable interrupts globally
	sensors_init();
    sensors_get_data();
    __no_operation();

//    while(1){
//    	sensors_get_data();
//    }

    while (1)
    {
        //uint8_t i;
        sensors_get_data();

        // Check the USB state and directly main loop accordingly
        switch (USB_connectionState())
        {

            // This case is executed while your device is enumerated on the USB host
            case ST_ENUM_ACTIVE:

                // Enter LPM0 (can't do LPM3 when active)
                __bis_SR_register(LPM0_bits + GIE);
                _NOP(); 

                // Exit LPM on USB receive and perform a receive operation
                // If true, some data is in the buffer; begin receiving a cmd
                if (bCDCDataReceived_event)
				{
                    // Holds the new addition to the string
                    char pieceOfString[MAX_STR_LENGTH] = "";

                    // Holds the outgoing string
                    char outString[MAX_STR_LENGTH] = "";

                    // Add bytes in USB buffer to the string
                    cdcReceiveDataInBuffer((uint8_t*)pieceOfString, MAX_STR_LENGTH, CDC0_INTFNUM); // Get the next piece of the string

                    // Append new piece to the whole
                    strcat(wholeString,pieceOfString);

                   /* // Echo back the characters received
                    * Is ineffective while communicating with Pyserial
                    cdcSendDataInBackground((uint8_t*)pieceOfString,
					strlen(pieceOfString),CDC0_INTFNUM,0);*/

                    // Has the user pressed return yet?
                    if (retInString(wholeString))
					{
                        if (!(strcmp(wholeString, "SENSOR_1")))			//I2C Pressure
						{
//							for(index=0;index<10;index++)
//							{
//								 strcpy(outString, temp[index]);
//
//								//  sending out data
//								cdcSendDataInBackground((uint8_t*) outString, strlen(outString),CDC0_INTFNUM,0);
//							}
                        	for(index=0;index<5;index++)
							{
                        		 sprintf(buffer, "%d", (int) ((i2c_pkpa[index] >= 100)? 99 : i2c_pkpa[index]) );
								 //temp[index] = buffer;
								 strcpy(outString, buffer);

								//  sending out data
								cdcSendDataInBackground((uint8_t*) outString, strlen(outString),CDC0_INTFNUM,0);
							}
						}
                        else if (!(strcmp(wholeString, "SENSOR_2")))			//I2C Temperature
						{
							for(index=0;index<5;index++)
							{
								 sprintf(buffer, "%d", (int) i2c_temp[index]);
								// temp[index] = buffer;
								 strcpy(outString, buffer);

								//  sending out data
								cdcSendDataInBackground((uint8_t*) outString, strlen(outString),CDC0_INTFNUM,0);
							}
						}
                        else if (!(strcmp(wholeString, "SENSOR_3")))			//ADC LUX
						{
							for(index=0;index<5;index++)
							{
								 sprintf(buffer, "%d", (int) lux[index]);
								 //temp[index] = buffer;
								 strcpy(outString, buffer);

								//  sending out data
								cdcSendDataInBackground((uint8_t*) outString, strlen(outString),CDC0_INTFNUM,0);
							}
						}
                        else if (!(strcmp(wholeString, "SENSOR_4")))			//ADC TEMP.
						{
							for(index=0;index<5;index++)
							{
								 sprintf(buffer, "%d", (int) temp_adc[index]);
								 //temp[index] = buffer;
								 strcpy(outString, buffer);

								//  sending out data
								cdcSendDataInBackground((uint8_t*) outString, strlen(outString),CDC0_INTFNUM,0);
							}
						}
						else
						{
							// Prepare the outgoing string
							strcpy(outString,"\r\nNo such command!\r\n\r\n");

							// Send the response over USB
							cdcSendDataInBackground((uint8_t*)outString, strlen(outString),CDC0_INTFNUM,0);
						}

						// Clear the string in preparation for the next one
						for (i = 0; i < MAX_STR_LENGTH; i++)
						{
							wholeString[i] = 0x00;
						}

                    }
                    bCDCDataReceived_event = FALSE;


                }
                break;

            // These cases are executed while your device is disconnected from
            // the host (meaning, not enumerated); enumerated but suspended
            // by the host, or connected to a powered hub without a USB host present.

            case ST_PHYS_DISCONNECTED:
            case ST_ENUM_SUSPENDED:
            case ST_PHYS_CONNECTED_NOENUM_SUSP:

			//sensors_get_data();
			__bis_SR_register(LPM3_bits + GIE);
			_NOP();
			break;

            // The default is executed for the momentary state
            // ST_ENUM_IN_PROGRESS.  Usually, this state only last a few
            // seconds.  Be sure not to enter LPM3 in this state; USB
            // communication is taking place here, and therefore the mode must
            // be LPM0 or active-CPU.

            case ST_ENUM_IN_PROGRESS:
            default:;
        }
    }  // while(1)
} // main()

/* ======== UNMI_ISR ======== */
#pragma vector = UNMI_VECTOR
__interrupt void UNMI_ISR (void)
{
    switch (__even_in_range(SYSUNIV, SYSUNIV_BUSIFG))
    {
        case SYSUNIV_NONE:
            __no_operation();
            break;

        case SYSUNIV_NMIIFG:
            __no_operation();
            break;

        case SYSUNIV_OFIFG:
			#ifndef DRIVERLIB_LEGACY_MODE
				UCS_clearFaultFlag(UCS_XT2OFFG);
				UCS_clearFaultFlag(UCS_DCOFFG);
				SFR_clearInterrupt(SFR_OSCILLATOR_FAULT_INTERRUPT);
			#else
				UCS_clearFaultFlag(UCS_BASE, UCS_XT2OFFG);
				UCS_clearFaultFlag(UCS_BASE, UCS_DCOFFG);
				SFR_clearInterrupt(SFR_BASE, SFR_OSCILLATOR_FAULT_INTERRUPT);
			#endif
        break;

        case SYSUNIV_ACCVIFG:
            __no_operation();
            break;

        case SYSUNIV_BUSIFG:
            // If the CPU accesses USB memory while the USB module is
            // suspended, a "bus error" can occur.  This generates an NMI.  If
            // USB is automatically disconnecting in your software, set a
            // breakpoint here and see if execution hits it.  See the
            // Programmer's Guide for more information.

            SYSBERRIV = 0; //clear bus error flag
            USB_disable(); //Disable
    }
}

/* ======== retInString ======== */
// This function returns true if there's an 0x0D character in the string; and if
// so, it trims the 0x0D and anything that had followed it.

uint8_t retInString (char* string)
{

    uint8_t retPos = 0,i,len;
    char tempStr[MAX_STR_LENGTH] = "";

    strncpy(tempStr,string,strlen(string));  // Make a copy of the string
    len = strlen(tempStr);

    // Find 0x0D; if not found, retPos ends up at len
    while ((tempStr[retPos] != 0x0A) && (tempStr[retPos] != 0x0D) && (retPos++ < len)) ;

    // If 0x0D was actually found...
    if ((retPos < len) && (tempStr[retPos] == 0x0D))
	{
        for (i = 0; i < MAX_STR_LENGTH; i++){ // Empty the buffer
            string[i] = 0x00;
        }

        //...trim the input string to just before 0x0D
        strncpy(string,tempStr,retPos);

        //...and tell the calling function that we did so
        return ( TRUE) ;

    // If 0x0D was actually found...
    }
	else if ((retPos < len) && (tempStr[retPos] == 0x0A))
	{
        // Empty the buffer
        for (i = 0; i < MAX_STR_LENGTH; i++)
		{
            string[i] = 0x00;
        }

        //...trim the input string to just before 0x0D
        strncpy(string,tempStr,retPos);

        //...and tell the calling function that we did so
        return ( TRUE) ;
    }
	else if (tempStr[retPos] == 0x0D)
	{
        for (i = 0; i < MAX_STR_LENGTH; i++)  // Empty the buffer
		{
            string[i] = 0x00;
        }

        // ...trim the input string to just before 0x0D
        strncpy(string,tempStr,retPos);

        // ...and tell the calling function that we did so
        return ( TRUE) ;
    }
	else if (retPos < len)
	{
        for (i = 0; i < MAX_STR_LENGTH; i++)  // Empty the buffer
        {
			string[i] = 0x00;
        }

        //...trim the input string to just before 0x0D
        strncpy(string,tempStr,retPos);

        //...and tell the calling function that we did so
        return ( TRUE) ;
    }
    return ( FALSE) ; // Otherwise, it wasn't found
}

void convertTwoDigBinToASCII(uint8_t bin, uint8_t* str)
{
    str[0] = '0';
    if (bin >= 10)
    {
        str[0] = (bin / 10) + 48;
    }
    str[1] = (bin % 10) + 48;
}

///* ======= TIMER1_A0_ISR ======== */
//#pragma vector=TIMER0_A0_VECTOR
//__interrupt void TIMER0_A0_ISR (void)
//{
//}
//Released_Version_4_10_02

//----------------Sensor Functions--------------------------
void sensors_init(){

	///----------------I2C----------------
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

    write(0x04);
	for(i=0;i<0xfff;i++);
    read(data, 8);//read 8 co-efficients
    assign_data();

}

void sensors_get_data(){
	swap_arrays();

	//----------------I2C------------------------
//    write(0x04);
//	for(i=0;i<0xfff;i++);
//    read(data, 8);//read 8 co-efficients
//    assign_data();
    stc();
	for(i=0;i<0xfff;i++);
	write(0x0);//read from the pressure register
	for(i=0;i<0xfff;i++);
	read(data, 5);
	compute_pressure();

//	//---------------DHT---------------------------
//	P2SEL &= ~BIT3;
//	P2DIR |= BIT3;					//pull down for 18 ms
//	P2OUT &= ~BIT3;
//
//	TA0CCR0 = TRIGGER_DELAY;
//	TA0CCTL0 = CCIE;					//trigger interrupt enable
//	TA0CTL = TASSEL_1 | MC_1 | ID_0 | TACLR;		//ACLK, Upmode, /1, TA0R is cleared
//	__low_power_mode_0();				//exits when temp and hum is read from sensor
//	convert_data_to_bin();			//conversion of data from DHT sensor

	//----------lux and temp------------------
	set_adc_lux();
	ADC12IE = 0x001;
	ADC12CTL0 |= ADC12SC;
	__low_power_mode_0();
	set_adc_temp();
	ADC12IE = 0x001;
	ADC12CTL0 &= ~ADC12SC;
	ADC12CTL0 |= ADC12SC;
	__low_power_mode_0();

	calc_lux();
	temp_adc[4] = (int) ((float)(((long)adc_temp_intm - CALADC12_15V_30C) * (85 - 30)) / (CALADC12_15V_85C - CALADC12_15V_30C) + 30.0f);
}

//drops oldest value in arrays
void swap_arrays(){
	unsigned int j = 0;

	for(j=0; j<4; j++){
		i2c_pkpa[j] = i2c_pkpa[j+1];
		i2c_temp[j] = i2c_temp[j+1];
		temp_adc[j] = temp_adc[j+1];
		lux[j] = lux[j+1];
//		temp_dht[j] = temp_dht[j+1];
//		temp_adc[j] = temp_adc[j+1];
	}

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
	i2c_pkpa[4] = (pcomp*((115-50)/1023.00)) + 50;

	i2c_temp[4] = ((tadc - 498.00)/(-5.35)) + 25.0;

}


void convert_data_to_bin(){
		//diff_arr indexes:	0 - Resp. start, 1 - ACK, [2:17] - HUM, [18:33] - Temp, [34:41] - Chk. sum

		i = 0;
		//int chksum_calc = 0;

		uint16_t hum_bin = 0;		//clear data
		uint16_t temp_bin = 0;
		//uint16_t chksum_bin = 0;

		//chksum_flag = 0;

		//extract data
		for (i=2; i<=17; i++){		//HUMIDITY
			if(diff_arr[i]>DHT_HIGH_THRESH){
				hum_bin |= (BIT0 << (17-i));		//shift: 15-i+2
			}
		}

		for (i=18; i<=33; i++){		//Temperature
			if(diff_arr[i]>DHT_HIGH_THRESH){
				temp_bin |= (BIT0 << (33-i));		//shift: 15-i+18
			}
		}

//		for (i=34; i<=41; i++){		//Chk. sum
//			if(diff_arr[i]>DHT_HIGH_THRESH){
//				chksum_bin |= (BIT0 << (41-i));		//shift: 7-i+34
//			}
//		}

		//perform check sum
		//chksum_calc = (hum_bin >> 8)  +  ((hum_bin << 8) >> 8) + (temp_bin >> 8)  +  ((temp_bin << 8) >> 8);		//logical shifts

		//check sum flag
		//chksum_flag = ((chksum_calc & 255) == chksum_bin)? 1 : 0;			//8-bit check sum

		//final values after removing the fractional part
		hum[4] = (int) hum_bin/10;
		temp_dht[4] = (int) temp_bin/10;

}

//Sensor trigger timer
#pragma vector = TIMER0_A0_VECTOR
__interrupt void timerA0_ISR (void){

	TA0CCTL0 = ~CCIE;		//disable trigger interrupt
	P2DIR &= ~BIT3;			//sensor input

	//start response capture timer
	P2SEL |= BIT3;									//sensor pin - triggers interrupt
	TA2CCTL0 = CM_2 | SCS | CCIS_0 | CAP | CCIE; 	// falling edge, synchronized, Continuous up mode, Capture mode, Enable interrutps
	TA2CTL = TASSEL__SMCLK | MC_2 | TACLR;                  // SMCLK + Continuous Up Mode


}

//Sensor response capture timer
#pragma vector = TIMER2_A0_VECTOR
__interrupt void timer2_ISR (void){
		new_cap = TA2CCR0;		//TA2R;
		cap_diff = new_cap - old_cap;

		diff_arr[dht_index] = cap_diff;            // record difference to RAM array
		cap_arr[dht_index++] = new_cap;
		old_cap = new_cap;                       // store this capture value

		if (dht_index == 42)		//end of response
		{
			dht_index = 0;
			old_cap = 0;

			//index_test++;

			TA2CCTL0 &= ~CCIE;				//disable data capture timer
			__low_power_mode_off_on_exit();
			//lock=0;

			//DHT is put in IDLE mode
			P2SEL &= ~BIT3;				//set sensor output to HIGH during IDLE
			P2DIR |= BIT3;
			P2OUT |= BIT3;


		}

}

void set_adc_lux(){
	mode = LUX;

	ADC12CTL0 &= ~ADC12SC;                     // Start conversion
	ADC12CTL0 &= ~ADC12ENC;                    // Enable conversions


    P7SEL |= BIT0;
	ADC12CTL0 = ADC12ON+ADC12SHT1_8+ADC12MSC; // Turn on ADC12, set sampling time

	// set multiple sample conversion
	ADC12CTL1 = ADC12SHP+ADC12CONSEQ_2;       // Use sampling timer, set mode
	ADC12IE |= 0x01;                          // Enable ADC12IFG.0
	ADC12MCTL0 = ADC12INCH_12;//+ADC12EOS;
	ADC12CTL0 |= ADC12ENC;                    // Enable conversions
	//ADC12CTL0 |= ADC12SC;                     // Start conversion

}

void set_adc_temp(){
	mode = TEMP;

	ADC12CTL0 &= ~ADC12SC;                     // Start conversion
	ADC12CTL0 &= ~ADC12ENC;                    // Enable conversions

	REFCTL0 &= ~REFMSTR;                      // Reset REFMSTR to hand over control to
											// ADC12_A ref control registers

	ADC12CTL0 = ADC12SHT0_8 + ADC12REFON + ADC12ON;
											// Internal ref = 1.5V
	ADC12CTL1 = ADC12SHP;                     // enable sample timer
	ADC12MCTL0 = ADC12SREF_1 + ADC12INCH_10;  // ADC i/p ch A10 = temp sense i/p
	ADC12IE |= BIT0;//0x001;                          // ADC_IFG upon conv result-ADCMEMO
	__delay_cycles(100);                       // delay to allow Ref to settle
	ADC12CTL0 |= ADC12ENC;


}
#pragma vector=ADC12_VECTOR
__interrupt void ADC12ISR (void)
{


  switch(__even_in_range(ADC12IV,34))
  {
  case  0: break;                           // Vector  0:  No interrupt
  case  2: break;                           // Vector  2:  ADC overflow
  case  4: break;                           // Vector  4:  ADC timing overflow
  case  6:                                  // Vector  6:  ADC12IFG0
     //adc_out = ADC12MEM0;             // Move results
	    if (mode==LUX){
	    	adc_out = ADC12MEM0;
	    	ADC12IE &= ~0x01;
	    }else if (mode==TEMP){
	    	adc_temp_intm = ADC12MEM0;
	    }

	    __low_power_mode_off_on_exit();
  case  8: break;                           // Vector  8:  ADC12IFG1
  case 10: break;                           // Vector 10:  ADC12IFG2
  case 12: break;                           // Vector 12:  ADC12IFG3
  case 14: break;                           // Vector 14:  ADC12IFG4
  case 16: break;                           // Vector 16:  ADC12IFG5
  case 18: break;                           // Vector 18:  ADC12IFG6
  case 20: break;                           // Vector 20:  ADC12IFG7
  case 22: break;                           // Vector 22:  ADC12IFG8
  case 24: break;                           // Vector 24:  ADC12IFG9
  case 26: break;                           // Vector 26:  ADC12IFG10
  case 28: break;                           // Vector 28:  ADC12IFG11
  case 30: break;	                           // Vector 30:  ADC12IFG12
  case 32: break;                           // Vector 32:  ADC12IFG13
  case 34: break;                           // Vector 34:  ADC12IFG14
  default: break;
  }

//  ADC12IE &= ~0x01;
//  __low_power_mode_off_on_exit();
  //lock=0;
}

void calc_lux(){
	lux[4] = (int)adc_out/lux_unit;
	lux[4] = (lux[4]>99)? 99 : lux[4];
}

