string name="Spectral Inverter";
string description="inverts the spectrum linearly";

int i = 1;

void processBlock(BlockData& data) {
	for(uint s = 0; s < data.samplesToProcess; ++s) {
		i *= -1;
		for(uint ch = 0; ch < audioOutputsCount; ++ch)
			data.samples[ch][s] *= i;
	}
}