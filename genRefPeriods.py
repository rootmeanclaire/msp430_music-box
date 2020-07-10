from math import pow

REF_OCTAVE = 0
PITCH_A4 = 45
LEN_SCALE = 7
SECOND_SMCLK = 1000000
SAMPLE_RATE = 22050
SCALE_MAJ = [0, 2, 4, 5, 7, 9, 11]

for pitch in range(0, LEN_SCALE):
	print(round(SAMPLE_RATE / (440 * pow(2, (
		SCALE_MAJ[pitch % LEN_SCALE]
		+ ((pitch / LEN_SCALE) * 12)
		- PITCH_A4
	) / 12.0))), end=",\n")