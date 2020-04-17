string name="XY";
string description="xy";

#include "lofi.hxx"

/*

sample&hold distortion works by holding the current
sample for some amount of time

*/

array<double> inputParameters( inputParametersNames.length );
array<string> inputParametersNames = { "bit", "s&h" };
array<string> inputParametersUnits = {};
array<string> inputParametersEnums = {};
array<double> inputParametersMin = { 0., 0. };
array<double> inputParametersMax = { 1., 1. };
array<double> inputParametersStep = { -1., -1 };

float xy;
DSP::SampleAndHold hold;

void processBlock(BlockData& data){
	for(uint s = 0; s < data.samplesToProcess; ++s)
		for(uint ch = 0; ch < audioInputsCount; ++ch){
			data.samples[ch][s] = floor(data.samples[ch][s] * xy) / xy;
			hold.process(data.samples[ch][s], ch);
		}
}

void updateInputParametersForBlock(const TransportInfo@ info){
    xy = 64.f - tanh(2.f * inputParameters[0]) * 63.f;
	hold.setDrive(inputParameters[1]);
}

int rint(double& d){
    if(d < 0)
        return int(d - .5);
    else
        return int(d + .5);
}