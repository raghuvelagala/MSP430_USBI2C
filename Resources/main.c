#include <string.h>
#include "driverlib.h"
#include "USB_config/descriptors.h"
#include "USB_API/USB_Common/device.h"
#include "USB_API/USB_Common/usb.h"                 // USB-specific functions
#include "USB_API/USB_CDC_API/UsbCdc.h"
#include "USB_app/usbConstructs.h"

#define MAX_STR_LENGTH 64

/* NOTE: Modify hal.h to select a specific evaluation board and customize for your own board */
#include "hal.h"

// Function declarations
uint8_t retInString (char* string);
void initTimer(void);
void convertTwoDigBinToASCII(uint8_t bin, uint8_t* str);

// Global flags set by events
volatile uint8_t bCDCDataReceived_event = FALSE; // Indicates data has been rx'ed
                                              // without an open rx operation

char wholeString[MAX_STR_LENGTH] = ""; // Entire input str from last 'return'

uint8_t timeStr[2];
int index;

//declaration of 50 temperature values
char *temp[50] = {"59","40","32","27","47","44","66","77","22","88","00","11","24","35","47","56","28","90","63","54","67","32","45","67",
"11","43","54","68","31","91","02","09","34","37","37","78","90","56","43","78","65","43","29","04","36","47","12","34","67","80","50"};

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

    __enable_interrupt();  // Enable interrupts globally

    while (1)
    {
        uint8_t i;

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
                    cdcReceiveDataInBuffer((uint8_t*)pieceOfString,

					MAX_STR_LENGTH,
					CDC0_INTFNUM); // Get the next piece of the string

                    // Append new piece to the whole
                    strcat(wholeString,pieceOfString);

                   /* // Echo back the characters received
                    * Is ineffective while communicating with Pyserial
                    cdcSendDataInBackground((uint8_t*)pieceOfString,
					strlen(pieceOfString),CDC0_INTFNUM,0);*/

                    // Has the user pressed return yet?
                    if (retInString(wholeString))
					{
                        if (!(strcmp(wholeString, "GO")))
						{
							for(index=0;index<50;index++)
							{
								 strcpy(outString, temp[index]);

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

/* ======= TIMER1_A0_ISR ======== */
#pragma vector=TIMER0_A0_VECTOR
__interrupt void TIMER0_A0_ISR (void)
{
}
//Released_Version_4_10_02

