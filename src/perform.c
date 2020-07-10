#include <stdint.h>
#include <msp430.h>
#include "compose.h"
#include "perform.h"

//Periods of one octave of C major starting from C0
static const uint16_t REF_PERIODS[INTVL_OCTAVE] = {
	674,
	544,
	439,
	375,
	303,
	244,
	197
};

//The section currently being played
Section *section;
//Says which sample of the current tick it is
uint32_t iSample = 0;
//The current tick of the section
uint16_t iTick = 0;
//The index of the current note of the current measure
uint8_t iMelody;
//The tick on which the last new melody note was loaded
uint8_t iLastNew;
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

uint16_t getHalfPeriod(uint8_t pitch) {
	return REF_PERIODS[pitch % INTVL_OCTAVE] >> (pitch / INTVL_OCTAVE);
}

void setSection(Section *newSection) {
	section = newSection;
	stepSection();
	iSample = 0;
	iTick = 0;
}

void playFromStart() {
	iSample = 0;
	iTick = 0;
}

uint8_t donePlaying() {
	return iTick >= NOTE_PRECISION * LEN_CHORD_LOOP;
}

void stepSection() {
	iSample = 0;

	//Check if next measure should be loaded
	if (iTick % NOTE_PRECISION == 0) {

		//Load new bassline pitch
		periodBassline = getHalfPeriod(
			section->bassline.chords.roots[
				iTick / NOTE_PRECISION
			]
			+ (OCTAVE_BASSLINE * INTVL_OCTAVE)
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
		periodMelody = getHalfPeriod(currMeasure[iMelody].pitch);
	}
	//Check if next melody note should be loaded
	else if (iTick - iLastNew >= currMeasure[iMelody].length) {
		iMelody++;
		iLastNew = iTick;
		periodMelody = getHalfPeriod(currMeasure[iMelody].pitch);
	}

	//Check if any notes are ending this tick
	melodyNoteEnd = iTick - iLastNew == currMeasure[iMelody].length - 1;
	basslineNoteEnd = (
		section->bassline.strumMask & (1 << ((iTick % NOTE_PRECISION) + 1))
	);
}

void sampleSection() {
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
	if (iSample >= SAMPLES_PER_TICK) {
		stepSection();
		P1OUT ^= BIT0;
	}
	sampleSection();
}