#include <stdint.h>
#include <stdlib.h>
#include <msp430.h>
#include "main.h"
#include "compose.h"
#include "perform.h"

static Section section;

void main(void) {
	//Calibrate DCO for 8 MHz
	BCSCTL1 = CALBC1_8MHZ;
	DCOCTL = CALDCO_8MHZ;

	//Stop watchdog timer
	WDTCTL = WDTPW | WDTHOLD;

	//Initialize unused ports to save power
	P2DIR = 0xFF;
	P2OUT = 0x00;

	//Set pins for output
	P1DIR = PIN_MELODY | PIN_BASSLINE | BIT0;
	//Clear all output pins
	P1OUT = 0x00;
	//Disable secondary outputs
	P1SEL &= ~(PIN_MELODY | PIN_BASSLINE | BIT0);

	section = composeSection();
	setSection(&section);

	//Init timer
	//Set timer comparison value
	TACCR0 = (SECOND_SMCLK / SAMPLE_RATE);
	TA0CTL |= (
		//Set clock source to SMCLK
		TASSEL_2 |
		//Count up to TACCR0
		MC_1 |
		//Set clock divider to 1
		ID_0
	);
	//Enable timer interrupt
	TA0CCTL0 |= CCIE;

	//Enable interrupts
	__enable_interrupt();

	while (1);
}