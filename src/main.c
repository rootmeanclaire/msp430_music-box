#include <stdint.h>
#include <stdlib.h>
#include <msp430.h>
#include "main.h"
#include "compose.h"
#include "perform.h"

static Section section;

void seedRandom(void);

void main(void) {
	//Calibrate DCO for 16 MHz
	BCSCTL1 = CALBC1_16MHZ;
	DCOCTL = CALDCO_16MHZ;

	//Stop watchdog timer
	WDTCTL = WDTPW | WDTHOLD;

	//Initialize unused ports to save power
	P2DIR = 0xFF;
	P2OUT = 0x00;

	//Set pins for output
	P1DIR = PIN_MELODY | PIN_BASSLINE | PIN_LED;
	//Set to wake on ing edge
	P1IES = PIN_BUTTON_HINGE;
	//Clear all output pins
	P1OUT = 0x00;
	//Disable secondary outputs
	P1SEL &= ~(PIN_MELODY | PIN_BASSLINE | PIN_LED);

	seedRandom();

	//Generate piece
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

	//Turn off CPU
	//Everythin else is done with interrupts
	__bis_SR_register(LPM0_bits | GIE);
}

void seedRandom(void) {
	//Set up ADC
	ADC10CTL1 = (
		//Set input from P1.3 (A3)
		INCH_3 |
		//Set clock divider to 4
		ADC10DIV_3 |
		//Set clock source to ADC10OSC
		ADC10SSEL_0 |
		//Single channel conversion
		CONSEQ_0
	);
	ADC10CTL0 = (
		//Set reference to VCC and VSS (aka GND)
		SREF_0 |
		//Sample and hold for 64 clock cycles
		ADC10SHT_3 |
		//Set sample rate to low
		ADC10SR |
		//Enable ADC10
		ADC10ON
	);
	//Enable P1.3 for ADC input
	ADC10AE0 = BIT3;
	
	//Start ADC
	ADC10CTL0 |= ENC | ADC10SC;

	//Wait for result
	while ((ADC10CTL0 & ADC10IFG) == 0);

	//Seed random
	srand(ADC10MEM & 0x3F);
}