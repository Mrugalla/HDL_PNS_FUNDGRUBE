string name="XY";
string description="xy";

#include "filter.hxx"

/*

in this script we utilize the typical mix-formular to make a
very simplistic lowpass-filter.

*/

array<double> inputParameters( inputParametersNames.length );
array<string> inputParametersNames = { "cutoff" };
array<string> inputParametersUnits = {};
array<string> inputParametersEnums = {};
array<double> inputParametersMin = { 0. };
array<double> inputParametersMax = { 1. };
array<double> inputParametersStep = { -1. };

array<DSP::Filter> filter(2);

void processBlock(BlockData& data){
	for(uint s = 0; s < data.samplesToProcess; ++s)
		for(uint ch = 0; ch < audioInputsCount; ++ch)
			filter[ch].process(data.samples[ch][s]);
}

void updateInputParametersForBlock(const TransportInfo@ info){
    filter[0].setCutoff(inputParameters[0]);
	filter[1].setCutoff(inputParameters[0]);
}

int rint(double& d){
    if(d < 0)
        return int(d - .5);
    else
        return int(d + .5);
}