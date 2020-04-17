string name="XY";
string description="xy";

#include "filter.hxx"

/*

let's improve the filter by adding the option to turn up its order
and the ability to change between low- and highpass.

*/

array<double> inputParameters( inputParametersNames.length );
array<string> inputParametersNames = { "cutoff", "order", "type" };
array<string> inputParametersUnits = {};
array<string> inputParametersEnums = {,,"lp;hp"};
array<double> inputParametersMin = { 0., 1, 0 };
array<double> inputParametersMax = { 1., 14, 1 };
array<int> inputParametersSteps = { -1., 14, 2 };

DSP::Filter filter;

void processBlock(BlockData& data){
	for(uint s = 0; s < data.samplesToProcess; ++s)
		for(uint ch = 0; ch < audioInputsCount; ++ch)
			filter.process(data.samples[ch][s], ch);
}

void updateInputParametersForBlock(const TransportInfo@ info){
	filter.setType(rint(inputParameters[2]));
    filter.setCutoff(inputParameters[0]);
	filter.setOrder(rint(inputParameters[1]));
}

int rint(double& d){
    if(d < 0)
        return int(d - .5);
    else
        return int(d + .5);
}