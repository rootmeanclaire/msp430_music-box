#include <stdint.h>
#include <msp430.h>
//#include <msp430g2352.h>
#include "main.h"
#include "compose.h"
#include "perform.h"

Section section;

void main(void) {
	//Calibrate DCO for 1 MHz
	BCSCTL1 = CALBC1_1MHZ;
	DCOCTL = CALDCO_1MHZ;

	//Stop watchdog timer
	WDTCTL = WDTPW | WDTHOLD;

	//Initialize unused ports to save power
	P2DIR = 0xFF;
	P2OUT = 0x00;

	//Set P1.3 for output
	P1DIR = PIN_MELODY | PIN_BASSLINE;
	//Clear all output pins
	P1OUT = 0x00;
	//Disable secondary output for P1.3
	P1SEL &= ~(PIN_MELODY | PIN_BASSLINE);

	TACCR0 = (SECOND_SMCLK / SAMPLE_RATE);

	//Init timer
	TA0CTL |= (
		//Set clock source to SMCLK
		TASSEL_2 |
		//Count continuously
		MC_1 |
		//Set clock divider to 1
		ID_0 |
		//Enable timer interrupt
		TAIE
	);

	TA0CCTL0 |= CCIE;

	section = composeSection();
	setSection(&section);

	//Enable interrupts
	__enable_interrupt();

	while (1);
}