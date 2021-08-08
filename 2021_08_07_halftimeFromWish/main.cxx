string name="wish time";
string description="when you order halftime at wish";

array<string> inputParametersNames = { "grainlen", "tune", "mix", "reverse" };
array<string> inputParametersUnits = { , , " %"};
array<string> inputParametersEnums = { lengthsEnums, lengthsEnums, , "off;on" };
array<double> inputParameters(inputParametersNames.length);
array<int> inputParametersSteps = { 5, 5, -1, 2 };
array<double> inputParametersMin = { 0, 0, 0, 0 };
array<double> inputParametersMax = { 4, 4, 100, 1 };

const int resamplerSize = int(sampleRate * 10.); // 10 sec

string lengthsEnums = "4;2;1;1 / 2;1 / 4";
array<double> tuneMap = { 4., 2., 1., .5, .25 };
double bpm, ppqRaw, tune;

double grainlenParam = -1, grainlenValue;
bool reverse;

Sequencer seq;
array<Resampler> resampler(audioOutputsCount);

void processBlock(BlockData& data) {
	bpm = data.transport.get_bpm();
	double quarterNoteLen = 60. * sampleRate / bpm;
	ppqRaw = data.transport.get_positionInQuarterNotes() * .25;
	double barLen = 4. * quarterNoteLen;
	
	if(grainlenParam != inputParameters[0]) {
		grainlenParam = inputParameters[0];
		grainlenValue = tuneMap[int(rint(grainlenParam))];
		seq.setGrainLength(barLen / grainlenValue);
	}
	ppqRaw *= grainlenValue;
	
	tune = tuneMap[int(rint(inputParameters[1]))];
	double mixValue = inputParameters[2] * .01;
	for(uint ch = 0; ch < audioOutputsCount; ++ch)
		resampler[ch].setMix(mixValue);
	reverse = inputParameters[3] > .5;
	
	// actual processing:
	seq.processBlock(data);
	//seq.dbgBlock(data); // debugging sequencer
	processBlockResampler(data);
}

void processBlockResampler(BlockData& data) {
	for(uint ch = 0; ch < audioOutputsCount; ++ch)
		resampler[ch].processBlock(data.samples[ch], seq.buffer, data.samplesToProcess);
}

double rint(double d) {
    if(d < 0.) return floor(d - .5);
    return floor(d + .5);
}

float spline(array<double>& data, float x) {
			double iFloor = floor(x);
			int size = int(data.length);
			int i1 = int(iFloor);
			int i0 = (i1 - 1); if(i0 < 0) i0 += size;
			int i2 = (i0 + 1); if(i2 >= size) i2 -= size;
			int i3 = (i0 + 2); if(i3 >= size) i3 -= size;

			double frac = x - iFloor;
			double v0 = data[i0];
			double v1 = data[i1];
			double v2 = data[i2];
			double v3 = data[i3];

			double c0 = v1;
			double c1 = .5 * (v2 - v0);
			double c2 = v0 - 2.5 * v1 + 2. * v2 - .5 * v3;
			double c3 = 1.5 * (v1 - v2) + .5 * (v3 - v0);

			return ((c3 * frac + c2) * frac + c1) * frac + c0;
}

class Sequencer {
	Sequencer() {
		buffer.length = maxBlockSize;
	}
	void setGrainLength(double gr) {
		inc = 1. / gr;
	}
	void processBlock(BlockData& data) {
		if(data.transport.get_isPlaying()) {
			double ppqFloor = floor(ppqRaw);
			ppq = ppqRaw - ppqFloor;
		}
		for(uint s = 0; s < data.samplesToProcess; ++s)
			processSample(s);
	}
	void dbgBlock(BlockData& data) {
		for(uint ch = 0; ch < audioOutputsCount; ++ch)
			for(uint s = 0; s < data.samplesToProcess; ++s){
				data.samples[ch][s] = buffer[s];
				print(buffer[s] + "");
			}
				
	}
	array<double> buffer;
	private double ppq, inc;
	
	void processSample(uint s) {
		if(ppq >= 1.)
			--ppq;
		buffer[s] = ppq;
		ppq += inc;
	}
}

class Resampler {
	Resampler() {
		size = resamplerSize;
		buffer.length = size + 1;
		sizeD = double(size);
	}
	void setMix(double m) {
		mixDry = sqrt(1 - m);
		mixWet = sqrt(m);
	}
	void processBlock(array<double>& samples, array<double>& seq, uint numSamples) {
		for(uint s = 0; s < numSamples; ++s) {
			incW();
			incR(seq, s);
			processSample(samples, s);
		}
	}
	
	array<double> buffer;
	double mixDry, mixWet, rHead, sizeD, seqFol;
	uint wHead, size;
	
	private void incW() {
		++wHead;
		if(wHead >= size)
			wHead = 0;
	}
	private void incR(array<double>& seq, uint s) {
		double seqDif = seq[s] - seqFol;
		if(seqDif > 0.) {
			if(reverse) {
				rHead -= tune;
				while(rHead < 0.)
					rHead += sizeD;
			}
			else {
				rHead += tune;
				while(rHead >= sizeD)
					rHead -= sizeD;
			}
		}
		else {
			if(reverse) {
				rHead = wHead;
			}
			else {
				rHead = wHead - grainlenValue - 4;
				while(rHead < 0.)
					rHead += sizeD;
			}
		}
		seqFol = seq[s];
	}
	private void processSample(array<double>& samples, uint s) {
		buffer[wHead] = samples[s];
		samples[s] = mixDry * samples[s] + mixWet * spline(buffer, rHead);
	}
}



