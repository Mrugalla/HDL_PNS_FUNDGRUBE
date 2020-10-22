string name="XY";
string description="xy";

#include "hdl.hxx"

array<string> inputParametersNames = { "freq", "amp" };
array<string> inputParametersUnits = {};
array<string> inputParametersEnums = {};
array<double> inputParameters( inputParametersNames.length );
array<int> inputParametersSteps = { };
array<double> inputParametersMin = { 100 };
array<double> inputParametersMax = { 1000 };

int wtSize = 512, splineSize = 1024, lagRangeOrder = 4;
//hdl::Osc osc(wtSize);
//hdl::OscLinearInterpolation osc(wtSize);
//hdl::OscSplineInterpolation osc(wtSize, splineSize);
hdl::OscLagRangeInterpolation osc(wtSize, lagRangeOrder);
double freq = -1, amp;

void initialize() {
	//array<double> wt = { -1, 1 };
	//osc.setWT(wt);
}

void processBlock(BlockData& data) {
	if(!data.transport.get_isPlaying()) return;
	setParameters();
	
	for(uint s = 0; s < data.samplesToProcess; ++s) {
		double wave = osc.process();
		for(uint ch = 0; ch < audioOutputsCount; ++ch)
			data.samples[ch][s] = wave * amp;
	}
}

void setParameters() {
	if(freq != inputParameters[0]) {
		freq = inputParameters[0];
		osc.setFreq(freq);
	}
	amp = inputParameters[1];
}

double rint(double d) {
    if(d < 0)
		return floor(d - .5);
    return floor(d + .5);
}