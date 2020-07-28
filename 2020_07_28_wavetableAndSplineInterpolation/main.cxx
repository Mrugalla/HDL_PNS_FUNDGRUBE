string name="XY";
string description="xy";

#include "hdl.hxx"

array<string> inputParametersNames = { "freq", "amp" };
array<string> inputParametersUnits = {};
array<string> inputParametersEnums = {};
array<double> inputParameters( inputParametersNames.length );
array<int> inputParametersSteps = { };
array<double> inputParametersMin = { 50 };
array<double> inputParametersMax = { 420 };

//hdl::Osc osc;
//hdl::OscLinearInterpolation osc;
hdl::OscSplineInterpolation osc;
double freq = -1, amp;

void processBlock(BlockData& data){
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

int rint(double d){
    if(d < 0.) return int(d - .5);
    else return int(d + .5);
}