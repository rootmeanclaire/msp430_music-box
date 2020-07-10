#include "main.h"
#include "compose.h"

#ifndef RENDER_H
#define RENDER_H

#define TEMPO 120
#define SAMPLES_PER_TICK (SAMPLE_RATE / (TEMPO / 60))
#define T_NOTE_OFF ((SAMPLES_PER_TICK / 10) * 9)

void setSection(Section *newSection);
void stepSection();
void sampleSection();
void playFromStart();

#endif
