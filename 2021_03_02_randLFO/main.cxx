string name="XY";
string description="xy";

array<string> inputParametersNames = { "freqMin", "freqMax", "gain", "filter", "order" };
array<string> inputParametersUnits = { "hz", "hz",,"filters" };
array<string> inputParametersEnums = {,,,"off;on"};
array<double> inputParameters(inputParametersNames.length);
array<int> inputParametersSteps = {,,,2,maxOrder };
array<double> inputParametersMin = { .1, .1, 0, 0, 1 };
array<double> inputParametersMax = { 20, 20, 1, 1, maxOrder };

const double tau = 6.28318530718;
const double pi = tau / 2;

RandLFO randLFO;
const int maxOrder = 8;

void processBlock(BlockData& data) {
	processParameters();
	
	randLFO.synthesizeBlock(data);
	
	copyChannels(data);
}

void processParameters() {
	randLFO.setFreq(inputParameters[0], inputParameters[1]);
	randLFO.gain = inputParameters[2];
	randLFO.setFilterEnabled(inputParameters[3] > .5);
	randLFO.setFilterOrder(int(rint(inputParameters[4])));
}

double rint(double d) {
    if(d < 0.) return floor(d - .5);
    return floor(d + .5);
}

class Range {
	Range(double mn = 0, double mx = 1) { set(mn, mx); }
	void set(double mn, double mx) {
		min = mn;
		max = mx;
		distance = max - min;
	}
	double min, max, distance;
}

class Phase {
	void setFreq(double f) { inc = f / sampleRate; }
	bool process() {
		phase += inc;
		if(phase >= 1) {
			--phase;
			return true;
		}
		return false;
	}
	
	double phase = 1;
	private double inc = 0;
}

class LowPass1P {
	void setCutoff(double hz) {
		if(freq != hz) {
			freq = hz;
			double x = hz / sampleRate;
			cutoff = 1 - exp(-tau * x);
		}
	}
	void processBlock(BlockData& data) {
		for(uint s = 0; s < data.samplesToProcess; ++s) {
			env += cutoff * (data.samples[0][s] - env);
			data.samples[0][s] = env;
		}
	}
	double freq, cutoff = 1, env = 0;
}

class LowPass1POrder {
	void setOrder(int o) {
		if(order != o) {
			filters.length = o;
			order = o;
			setCutoff(cutoff, true);
		}
	}
	void setCutoff(double f, bool forced = false) {
		if(cutoff != f || forced) {
			cutoff = f;
			for(uint i = 0; i < filters.length; ++i)
				filters[i].setCutoff(cutoff);
		}
	}
	void processBlock(BlockData& data) {
		for(uint i = 0; i < filters.length; ++i)
			filters[i].processBlock(data);
	}
	array<LowPass1P> filters;
	double cutoff = 1;
	int order;
	
	private double compensate(double t) {
		return t * 3. / (2. * order + 1.);
	}
}

class RandLFO {
	void setFreq(double fMin, double fMax) { freqRange.set(fMin, fMax); }
	void setFilterEnabled(bool e) { filterEnabled = e;  }
	void setFilterOrder(int o) { filter.setOrder(o); }
	
	void synthesizeBlock(BlockData& data) {
		for(uint s = 0; s < data.samplesToProcess; ++s) {
			bool phaseReset = phase.process();
			if(phaseReset) {
				double newFreq = freqRange.min + rand() * freqRange.distance;
				phase.setFreq(newFreq);
				filter.setCutoff(newFreq);
				amplitude = rand(-1., 1.);
			}
			data.samples[0][s] = amplitude * gain;
		}
		if(filterEnabled) filter.processBlock(data);
	}
	
	Phase phase;
	Range freqRange;
	LowPass1POrder filter;
	double amplitude, gain = 1;
	bool filterEnabled;
}

void copyChannels(BlockData& data) {
	for(uint ch = 1; ch < audioOutputsCount; ++ch)
		for(uint s = 0; s < data.samplesToProcess; ++s)
			data.samples[ch][s] = data.samples[0][s];
}