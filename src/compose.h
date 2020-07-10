#include <stdint.h>

#ifndef COMPOSE_H
#define COMPOSE_H

#define NOTE_PRECISION 16
#define PITCH_REST 0
#define PITCH_A4 45
#define LEN_CHORD_LOOP 4
#define NUM_CHORD_LOOPS 2
#define INTVL_OCTAVE 7
#define OCTAVE_BASSLINE 3

#define GET_SCALE_DEG(pitch) (pitch % INTVL_OCTAVE)
#define GET_OCTAVE(pitch) (pitch / INTVL_OCTAVE)

typedef struct _Note {
	uint8_t pitch;
	uint8_t length;
} Note;

typedef struct _ChordLoop {
	uint8_t roots[LEN_CHORD_LOOP];
} ChordLoop;

typedef struct _Melody {
	Note motive[NOTE_PRECISION];
	Note continuous[NOTE_PRECISION];
	Note conclusive[NOTE_PRECISION];
} Melody;

typedef struct _Bassline {
	ChordLoop chords;
	uint16_t strumMask;
} Bassline;

typedef struct _Section {
	Melody melody;
	Bassline bassline;
} Section;

static const ChordLoop chordLoops[NUM_CHORD_LOOPS];
Section composeSection();

#endif
