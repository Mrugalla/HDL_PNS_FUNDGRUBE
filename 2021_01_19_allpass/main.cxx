string name="Allpass";
string description="allpass";

array<string> inputParametersNames = { "delay", "feedback", "order", "bias", "gain", "mix" };
array<string> inputParametersUnits = {};
array<string> inputParametersEnums = {};
array<double> inputParameters( inputParametersNames.length );
array<int> inputParametersSteps = { -1, -1, 12, -1, -1, -1 };
array<double> inputParametersMin = { 1, 0, 1, 0, 0, 0 };
array<double> inputParametersMax = { maxDelayInMs - 1, maxFeedback, 12, 1, 1, 1 };

double maxDelayInMs = 20.;
double maxFeedback = 1. / sqrt(2.);

double tau = 6.28318530718;
double pi = tau * .5;
double piHalf = pi / 2.;

array<Ultrapass> allpass;
Smooth delaySmooth;
double delay = -1, feedback = -1, bias = -1, order = -1, gain = -1, mix = -1;

void initialize() {
	delaySmooth.setCutoffInHz(10);
	allpass.length = audioOutputsCount;
	for(uint ch = 0; ch < audioOutputsCount; ++ch)
		allpass[ch].setMaxDelayInMs(maxDelayInMs);
}

void processBlock(BlockData& data) {
	processParameters();
	
	for(uint s = 0; s < data.samplesToProcess; ++s) {
		double smoothedDelay = delaySmooth.process(delay);
		for(uint ch = 0; ch < audioOutputsCount; ++ch) {
			allpass[ch].setDelayInMs(smoothedDelay);
			double apOutput = allpass[ch].processSample(data.samples[ch][s]);
			data.samples[ch][s] += mix * (apOutput - data.samples[ch][s]);
			data.samples[ch][s] *= gain;
		}
	}	
}

void processParameters() {
	double d = inputParameters[0];
	if(delay != d) {
		delay = d;
		delay = inputParameters[0];
	}
	
	double f = inputParameters[1];
	if(feedback != f) {
		feedback = f;
		for(uint ch = 0; ch < audioOutputsCount; ++ch)
			allpass[ch].setFeedback(feedback);
	}
	
	double o = rint(inputParameters[2]);
	if(order != o) {
		order = o;
		for(uint ch = 0; ch < audioOutputsCount; ++ch)
			allpass[ch].setOrder(int(order));
	}
	
	double b = inputParameters[3];
	if(bias != b) {
		bias = b;
		for(uint ch = 0; ch < audioOutputsCount; ++ch)
			allpass[ch].setBias(bias);
	}
	
	gain = inputParameters[4];
	mix = inputParameters[5];
}

double rint(double d) {
    if(d < 0)
		return d - .5;
    return d + .5;
}

class Smooth {
	void setCutoffInHz(double x) { cutoff = x / sampleRate; }
	
	double process(double x) {
		env += cutoff * (x - env);
		return env;
	}
	double env = 0., cutoff;
}

class Allpass {
	// SET
	void setMaxDelayInSamples(int x) { size = x; buffer.length = x; }
	void setMaxDelayInMs(double ms) { setMaxDelayInSamples(int(sampleRate * ms / 1000.)); }
	// PARAM
	void setDelayInSamples(int x) { delay = x; }
	void setDelayInMs(double x) { setDelayInSamples(int(sampleRate * x / 1000.)); }
	void setFeedback(double x) { feedback = x; }
	// PROCESS
	double processSample(double x) {
		inc();
		double zm = getReadHead();
		double ym = x + zm * -feedback;
		buffer[wIdx] = ym;
		return ym * feedback + zm;
	}
	
	private array<double> buffer;
	private double feedback, rIdx = 0;
	private int size, delay, wIdx = 0;
	
	private double getReadHead() {
		int f = int(rIdx);
		int c = (f + 1) % size;
		double m = rIdx - f;
		return buffer[f] + m * (buffer[c] - buffer[f]);
	}
	
	private void inc() {
		++wIdx;
		if(wIdx >= size) wIdx -= size;
		rIdx = wIdx - delay;
		if(rIdx < 0) rIdx += size;
	}
}

class Ultrapass {
	// SET
	void setOrder(int x) {
		order = x;
		allpass.length = order;
		oInv = 1. / (order + 1);
		setMaxDelayInSamples(maxDelayInSamples);
		setDelayInSamples(delayInSamples);
		setFeedback(feedback);
		setBias(bias);
	}
	void setMaxDelayInSamples(int x) {
		for(int o = 0; o < order; ++o)
			allpass[o].setMaxDelayInSamples(x);
	}
	void setMaxDelayInMs(double ms) {
		maxDelayInSamples = int(sampleRate * ms / 1000.) + 1;
		setMaxDelayInSamples(maxDelayInSamples);
	}
	// PARAM
	void setDelayInSamples(double d) {
		for(int o = 0; o < order; ++o) {
			double x = double(o + 1) * oInv;
			double y = sin(x * piHalf);
			double z = x + bias * (y - x);
			allpass[o].setDelayInSamples(int(d * z));
		}
	}
	void setDelayInMs(double x) {
		delayInSamples = sampleRate * x / 1000.;
		setDelayInSamples(delayInSamples);
	}
	void setFeedback(double x) {
		feedback = x;
		for(int o = 0; o < order; ++o)
			allpass[o].setFeedback(feedback);
		calcGainApprox();
	}
	void setBias(double x) {
		bias = x;
		setDelayInSamples(delayInSamples);
	}
	// PROCESS
	double processSample(double x) {
		double sum = 0.;
		for(int o = 0; o < order; ++o)
			sum += allpass[o].processSample(x);
		return sum * gainApprox;
	}
	
	private array<Allpass> allpass;
	double feedback, bias, delayInSamples, oInv, gainApprox;
	int order, maxDelayInSamples;
	
	private void calcGainApprox() { gainApprox = 1. / sqrt(order * feedback * 4); }
}