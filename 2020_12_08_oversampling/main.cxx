string name="XY";
string description="xy";

#include "convolution.hxx"

array<string> inputParametersNames = { "order" };
array<string> inputParametersUnits = {};
array<string> inputParametersEnums = {};
array<double> inputParameters( inputParametersNames.length );
array<int> inputParametersSteps = { maxOrder };
array<double> inputParametersMin = { 1 };
array<double> inputParametersMax = { maxOrder };

double tau = 6.28318530718;
double pi = tau / 2;
int maxOrder = 2;

Osc osc;
convolution::ImpulseResponse irUpsample(convolution::IRTest::make3());
convolution::ImpulseResponse irDownsample(convolution::IRTest::make4());
array<Oversampling> oversampling = {
	Oversampling(irUpsample.size, irDownsample.size), Oversampling(irUpsample.size, irDownsample.size)
};

void initialize() {
	osc.setFreq(2500); // PREPARE TEST SIGNAL. 2.5khz sine wave
}

void processBlock(BlockData& data) {
	processParameters(data);
	
	osc.process(data); // SYNTHESIZE TEST SIGNAL.
	
	for(uint ch = 0; ch < audioOutputsCount; ++ch) // UPSAMPLE + LP-FILTER (Smoothing)
		oversampling[ch].upsample(data, irUpsample, ch);
	
	for(uint ch = 0; ch < audioOutputsCount; ++ch) // SIGNAL PROCESSING
		distort(oversampling[ch].buffer, 6);
	
	for(uint ch = 0; ch < audioOutputsCount; ++ch) // LP-FILTER (Anti-Aliasing) + DOWNSAMPLE
		oversampling[ch].downsample(data, irDownsample, ch);
}

void processParameters(BlockData& data) {
	int order = int(rint(inputParameters[0]));
	for(uint ch = 0; ch < audioOutputsCount; ++ch) {
		oversampling[ch].setOrder(data, order);
	}
}

double rint(double d) {
    if(d < 0)
		return d - .5;
    return d + .5;
}

void distort(array<double>& data, double drive) {
	for(uint i = 0; i < data.length; ++i)
		data[i] = tanh(data[i] * drive);
}

class Osc {
	void setFreq(double f) { inc = tau * f / sampleRate; }
	double process() {
		phase += inc;
		if(phase >= tau) phase -= tau;
		return sin(phase);
	}
	void process(BlockData& data) {
		for(uint s = 0; s < data.samplesToProcess; ++s) {
			double wave = process();
			for(uint ch = 0; ch < audioOutputsCount; ++ch)
				data.samples[ch][s] = wave;
		}
	}
	double phase = 0, inc = 0;
}

class Oversampling {
	Oversampling(){}
	Oversampling(int upsampleFilterSize, int downsampleFilterSize) {
		upsampleFilter = convolution::Processor(upsampleFilterSize);
		downsampleFilter = convolution::Processor(downsampleFilterSize);
	}
	
	void setOrder(BlockData& data, int o) {
		order = o;
		int newSize = data.samplesToProcess * order;
		if(numSamples != newSize) {
			numSamples = newSize;
			Fs = sampleRate * order;
			buffer.length = numSamples;
		}
	}
	
	void upsample(BlockData& data, convolution::ImpulseResponse& ir, int ch) {
		zeroStuffingInterpolation(data, ch);
		if(order == 2) upsampleFilter.process(buffer, ir);
	}
	void downsample(BlockData& data, convolution::ImpulseResponse& ir, int ch) {
		if(order == 2) downsampleFilter.process(buffer, ir);
		for(uint i = 0; i < data.samplesToProcess; ++i)
			data.samples[ch][i] = buffer[int(i * order)];
	}
	
	
	convolution::Processor upsampleFilter;
	convolution::Processor downsampleFilter;
	double Fs;
	array<double> buffer;
	int order, numSamples;
	
	private void zeroStuffingInterpolation(BlockData& data, int ch) {
		for(int i = 0; i < numSamples; ++i)
			buffer[i] = 0;
		for(uint i = 0; i < data.samplesToProcess; ++i)
			buffer[i * order] = data.samples[ch][i];
	}
}