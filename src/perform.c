#include <stdint.h>
#include <msp430.h>
#include "compose.h"
#include "perform.h"

//Periods of one octave of C major starting from C0
static const uint16_t REF_PERIODS[INTVL_OCTAVE] = {
	674,
	601,
	535,
	505,
	450,
	401,
	357
};

//The section currently being played
Section *section = 0;
//Says which sample of the current tick it is
uint32_t iSample = 0;
//The current tick of the section
uint16_t iTick = 0;
//The index of the current note of the current measure
uint8_t iMelody = 0;
//The tick on which the last new melody note was loaded
uint16_t iLastNew = 0;
//Pointer to the current measure
Note *currMeasure;
//If a note is ending this tick
uint8_t melodyNoteEnd = 0;
uint8_t basslineNoteEnd = 0;
//Number of samples since the last high/low change of the square wave
uint16_t tMelody = 0;
uint16_t tBassline = 0;
//Period of the current frequency
uint16_t periodMelody;
uint16_t periodBassline;

inline void stepSection();
inline void sampleSection();


uint16_t getHalfPeriodMelody(uint8_t pitch) {
	return REF_PERIODS[pitch % INTVL_OCTAVE] >> (pitch / INTVL_OCTAVE);
}

uint16_t getHalfPeriodBassline(uint8_t root) {
	return REF_PERIODS[root] >> OCTAVE_BASSLINE;
}

void setSection(Section *newSection) {
	section = newSection;
	iTick = 0;
	stepSection();
}

void playFromStart() {
	iTick = 0;
	stepSection();
}

uint8_t donePlaying() {
	return iTick >= NOTE_PRECISION * LEN_CHORD_LOOP;
}

inline void stepSection() {
	iSample = 0;

	//Check if next measure should be loaded
	if (iTick % NOTE_PRECISION == 0) {
		//Load new bassline pitch
		periodBassline = getHalfPeriodBassline(
			section->bassline.chords.roots[
				iTick / NOTE_PRECISION
			]
		);
		
		//Load new melody measure
		if (iTick / NOTE_PRECISION == 1) {
			currMeasure = section->melody.continuous;
		} else if (iTick / NOTE_PRECISION == 3) {
			currMeasure = section->melody.conclusive;
		} else {
			currMeasure = section->melody.motive;
		}

		iMelody = 0;
		iLastNew = iTick;
		periodMelody = getHalfPeriodMelody(currMeasure[iMelody].pitch);
	}
	//Check if next melody note should be loaded
	else if (iTick - iLastNew >= currMeasure[iMelody].length) {
		iMelody++;
		iLastNew = iTick;
		periodMelody = getHalfPeriodMelody(currMeasure[iMelody].pitch);
	}

	//Check if any notes are ending this tick
	melodyNoteEnd = (
		(iTick - iLastNew) == (currMeasure[iMelody].length - 1)
	);
	basslineNoteEnd = (
		(section->bassline.strumMask & (1 << ((iTick + 1) % NOTE_PRECISION)))
	);

	iTick++;
}

inline void sampleSection() {
	if (currMeasure[iMelody].pitch != PITCH_REST && !(melodyNoteEnd && iSample > T_NOTE_OFF)) {
		if (tMelody >= periodMelody) {
			P1OUT ^= PIN_MELODY;
			tMelody = 0;
		} else {
			tMelody++;
		}
	}

	if (!(basslineNoteEnd && iSample > T_NOTE_OFF)) {
		if (tBassline >= periodBassline) {
			P1OUT ^= PIN_BASSLINE;
			tBassline = 0;
		} else {
			tBassline++;
		}
	}

	iSample++;
}

__attribute__((interrupt(TIMER0_A0_VECTOR)))
void ISR_TIMER0_A0(void) {
	if (iTick < NOTE_PRECISION * LEN_CHORD_LOOP) {
		sampleSection();
		if (iSample >= SAMPLES_PER_TICK) {
			stepSection();
		}
	} else {
		//Turn off audio sampling interrupt
		TA0CCTL0 &= ~CCIE;
		//Turn on wake interrupt
		P1IE |= PIN_BUTTON_HINGE;
		//Go to sleep
		__bis_SR_register(LPM4_bits | GIE);
	}
}

__attribute__((interrupt(PORT1_VECTOR)))
void ISR_PORT1(void) {
	//Turn off wake interrupt
	P1IE |= PIN_BUTTON_HINGE;

	//Compose new piece
	*section = composeSection();
	playFromStart();

	//Turn on audio sampling interrupt
	TA0CCTL0 &= ~CCIE;

	__bis_SR_register_on_exit(LPM4_bits);
}