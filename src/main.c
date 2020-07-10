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
	P1DIR = PIN_MELODY | PIN_BASSLINE | BIT0;
	//Clear all output pins
	P1OUT = 0x00;
	//Disable secondary outputs
	P1SEL &= ~(PIN_MELODY | PIN_BASSLINE | BIT0);

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

	//Enable interrupts
	__enable_interrupt();

	while (1);
}

void seedRandom(void) {
	//Set up ADC
	ADC10CTL0 = (
		//Set reference to VCC and VSS (aka GND)
		SREF_0 |
		//Sample and hold for 64 clock cycles
		ADC10SHT_3 |
		//Set sample rate to low
		//ADC10SR |
		//Enable ADC10 interrupt
		ADC10IE |
		//Enable ADC10
		ADC10ON
	);
	ADC10CTL1 = (
		//Set input from P1.4 (A4)
		INCH_4 |
		//Set clock divider to 8
		ADC10DIV_7 |
		//Set clock source to ADC10OSC
		ADC10SSEL_0 |
		//Single channel conversion
		CONSEQ_0
	);
	//Enable P1.4 for ADC input
	ADC10AE0 = BIT4;
	
	//Start ADC
	ADC10CTL0 |= ENC | ADC10SC;

	//Go into low power mode until result is ready
	__enable_interrupt();
	__bis_SR_register(CPUOFF + GIE);
	__disable_interrupt();
	
	//Seed random
	srand(ADC10MEM);
}

__attribute__((interrupt(ADC10_VECTOR)))
void ISR_ADC10(void) {
	__bic_SR_register_on_exit(CPUOFF);
}