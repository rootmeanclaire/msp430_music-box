#include "main.h"
#include "compose.h"

#ifndef RENDER_H
#define RENDER_H

#define TEMPO 90
#define SAMPLES_PER_TICK ((uint16_t) (SAMPLE_RATE / ((float) (TEMPO * (NOTE_PRECISION / 4)) / 60)))
#define T_NOTE_OFF ((SAMPLES_PER_TICK / 10) * 9)

void setSection(Section *newSection);
void playFromStart();

#endif
