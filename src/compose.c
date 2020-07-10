#include <stdint.h>
#include <stdlib.h>
#include "compose.h"

static const ChordLoop chordLoops[NUM_CHORD_LOOPS] = {
	{{0, 4, 5, 3}},
	{{0, 3, 1, 4}}
};

void composeMeasure(Note *measure, uint8_t startRoot, uint8_t endRoot, uint8_t harmony) {
	uint8_t currLen = 0;
	uint8_t i = 0;
	uint8_t r;
	//Directional bias
	//Prevents melody from getting too low or high
	uint8_t dirBias = 0;

	for (i = 0; i < NOTE_PRECISION; ++i) {
		if (currLen > NOTE_PRECISION) {
			measure[i].length -= NOTE_PRECISION - currLen;
			break;
		} else if (currLen == NOTE_PRECISION) {
			break;
		} else {
			r = rand();
			
			if (i == 0) {
				if (startRoot) {
					measure[i] = (Note) {INTVL_OCTAVE * 4, NOTE_PRECISION / 8};
				} else {
					measure[i] = (Note) {(INTVL_OCTAVE * 5) - (r % (INTVL_OCTAVE - 2)), NOTE_PRECISION / 8};
				}
			} else {
				if (measure[i - 1].pitch < INTVL_OCTAVE * 4) {
					dirBias = 2;
				} else if (measure[i - 1].pitch > INTVL_OCTAVE * 5) {
					dirBias = -2;
				} else {
					dirBias = 0;
				}
				
				//Move towards or away from root
				if (endRoot) {
					measure[i].pitch = measure[i - 1].pitch - (r % 4) - 1 + dirBias;
				} else {
					measure[i].pitch = measure[i - 1].pitch + (r % 4) - 1 + dirBias;
				}

				//Add longer notes strategically to give illusion of structure
				if (currLen == NOTE_PRECISION / 4 || r % 2 == 0) {
					measure[i].length = NOTE_PRECISION / 8;

					//Prevent long clashing notes
					if (GET_SCALE_DEG(measure[i].pitch) + 1 == harmony) {
						measure[i].pitch += 1;
					} else if (GET_SCALE_DEG(measure[i].pitch) - 1 == harmony) {
						measure[i].pitch -= 1;
					}

					//Prevent unisons
					if (measure[i].pitch == harmony + (OCTAVE_BASSLINE * INTVL_OCTAVE)) {
						measure[i].pitch += 2;
					}
				} else {
					measure[i].length = NOTE_PRECISION / 16;
				}
				
			}
		
			currLen += measure[i].length;
		}
	}

	if (endRoot) {
		//Adjust pitch to end on root
		if (GET_SCALE_DEG(measure[i - 1].pitch) != 0) {
			measure[i - 1].pitch = INTVL_OCTAVE * 4;
		}
		//Adjust final note length for emphasis
		if (measure[i - 1].length < NOTE_PRECISION / 8) {
			uint8_t diffLen = (NOTE_PRECISION / 8) - measure[i - 1].length;

			measure[i - 1].length = NOTE_PRECISION / 8;
			
			//Adjust preceding notes to prevent measure overflow
			for (i = i - 2; i > 0; --i) {
				if (measure[i].length > diffLen) {
					measure[i].length -= diffLen;
					break;
				} else {
					diffLen -= measure[i].length;
					measure[i] = measure[i + 1];
				}
			}
		}
	}
}

Section composeSection() {
	Section section;

	section.bassline.chords = chordLoops[rand() % NUM_CHORD_LOOPS];
	section.bassline.strumMask = (1 << 0) | (1 << 2);

	composeMeasure(section.melody.motive, 1, 0, section.bassline.chords.roots[2]);
	composeMeasure(section.melody.continuous, 0, 0, section.bassline.chords.roots[1]);
	composeMeasure(section.melody.conclusive, 0, 1, section.bassline.chords.roots[3]);


	return section;
}
