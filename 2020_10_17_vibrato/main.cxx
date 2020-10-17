string name="Vibrato";
string description="vibrates signals lmao";

/*
a vibrato plugin. please watch the video for a more detailed description.
too lazy to make comments everywhere now
https://youtu.be/EIAv2YUUE-I
*/

array<string> inputParametersNames = { "depth", "freq", "width", "mix" };
array<string> inputParametersUnits = {};
array<string> inputParametersEnums = {};
array<double> inputParameters( inputParametersNames.length );
array<int> inputParametersSteps = {};
array<double> inputParametersMin = { 0, 0, 0, 0 };
array<double> inputParametersMax = { 1, 7, 1, 1 };

double tau = 6.28318530718;
double pi = tau * .5;

double ms = 3;
int size = ms2samples(ms);
Vibrato v0(size), v1(size);

void processBlock(BlockData& data) {
	processParameters();
	
	for(uint s = 0; s < data.samplesToProcess; ++s)
		data.samples[0][s] = v0.process(data.samples[0][s]);
	
	if(audioOutputsCount > 1)
		for(uint s = 0; s < data.samplesToProcess; ++s)
			data.samples[1][s] = v1.process(data.samples[1][s]);
}

void processParameters() {
	double depth = inputParameters[0];
	v0.setDepth(depth);
	v1.setDepth(depth);
	double freq = inputParameters[1];
	v0.setFreq(freq);
	v1.setFreq(freq);
	double width = inputParameters[2];
	v1.setOffset(width);
	double mix = inputParameters[3];
	v0.setMix(mix);
	v1.setMix(mix);
}

double rint(double d) {
    if(d < 0)
		return d - .5;
    return d + .5;
}

int ms2samples(double ms) { return int(ms * sampleRate / 1000.); }

namespace midSide {
	void encode(BlockData& data) {
		for(uint s = 0; s < data.samplesToProcess; ++s) {
			double l = data.samples[0][s];
			double r = data.samples[1][s];
			
			data.samples[0][s] = (l + r) / 2;
			data.samples[1][s] = (l - r) / 2;
		}
	}
	void decode(BlockData& data) {
		for(uint s = 0; s < data.samplesToProcess; ++s) {
			double mid = data.samples[0][s];
			double side = data.samples[1][s];
			
			data.samples[0][s] = mid + side;
			data.samples[1][s] = mid - side;
		}
	}
}

class Range {
	Range(double s = 0, double e = 1) { setRange(s, e); }
	void setRange(double s, double e) {
		start = s;
		end = e;
		interval = end - start;
	}
	
	double start, end, interval;
}

class FFDelay {
	void resize(int size) { data.length = size; }
	
	double process(double sample, int wIdx, double rIdx) {
		data[wIdx] = sample;
		return data[int(rIdx)];
	}
	
	array<double> data;
}

class Osc {
	void setFreq(double freq) { inc = tau * freq / sampleRate;  }
	void setOffset(double o) { offset = o * pi; }
	double process() {
		phase += inc;
		if(phase > tau)
			phase -= tau;
		return sin(phase + offset);
	}
	
	double phase = 0, offset = 0, inc = 0;
}

class LFO {
	void setFreq(double f) { osc.setFreq(f); }
	void setOffset(double o) { osc.setOffset(o); }
	double process(Range& range) {
		return range.start + range.interval * (osc.process() * .5 + .5);
	}
	Osc osc;
}

class Vibrato {
	Vibrato(int s = 0) {
		resize(s);
	}
	void resize(int s) {
		mix = 1;
		size = s;
		max = size - 1;
		wIdx = 0;
		rIdx = 0;
		delay.resize(size + 1);
	}
	
	void setDepth(double d) { range.setRange(0, d * max); }
	void setFreq(double f) { lfo.setFreq(f); }
	void setOffset(double o) { lfo.setOffset(o); }
	void setMix(double m) { mix = m; }
	
	double process(double sample) {
		inc();
		rIdx = lfo.process(range);
		rIdx = wIdx - rIdx;
		if(rIdx < 0)
			rIdx += size;
		return sample + mix * (delay.process(sample, wIdx, rIdx) - sample);
	}
	
	FFDelay delay;
	LFO lfo;
	Range range;
	int size, max, wIdx;
	double rIdx, mix;
	
	private void inc() {
		++wIdx;
		if(wIdx > max)
			wIdx = 0;
	}
}