string name="XY";
string description="xy";

#include "envelope.hxx"

array<string> inputParametersNames = { "freq" };
array<string> inputParametersUnits = {};
array<string> inputParametersEnums = {};
array<double> inputParameters( inputParametersNames.length );
array<int> inputParametersSteps = { };
array<double> inputParametersMin = { 20 };
array<double> inputParametersMax = { 120 };

hdl::EFSpline env;
double freq = 0;

void initialize() {
	hdl::Range spectralRange(inputParametersMin[0], inputParametersMax[0]);
	env.setSpectralRange(spectralRange);
}

void processBlock(BlockData& data) {
	processParameters();
	
	for(uint s = 0; s < data.samplesToProcess; ++s) {
		double mid = 0.;
		for(uint ch = 0; ch < audioOutputsCount; ++ch)
			mid += data.samples[ch][s];
		mid /= audioOutputsCount;
		
		double envelope = env.process(mid);
		
		for(uint ch = 0; ch < audioOutputsCount; ++ch)
			data.samples[ch][s] = envelope;
	}
}

void processParameters() {
	if(freq != inputParameters[0]) {
		freq = inputParameters[0];
		env.setReloadFrequency(freq);
	}
}

int rint(double d){
    if(d < 0.) return int(d - .5);
    else return int(d + .5);
}