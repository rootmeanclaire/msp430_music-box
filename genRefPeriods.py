from math import pow

REF_OCTAVE = 0
PITCH_A4 = 45
LEN_SCALE = 7
SECOND_SMCLK = 1000000
SAMPLE_RATE = 22050
SCALE_MAJ = [0, 2, 4, 5, 7, 9, 11]

for pitch in range(REF_OCTAVE, REF_OCTAVE + LEN_SCALE):
	print(round(SAMPLE_RATE / (440 * pow(2, (
		# Get semitone within octave
		SCALE_MAJ[pitch % LEN_SCALE]
		# Get octave
		+ ((pitch // LEN_SCALE) * 12)
		# Shift relative ot reference pitch
		- PITCH_A4
	) / 12))), end=",\n")