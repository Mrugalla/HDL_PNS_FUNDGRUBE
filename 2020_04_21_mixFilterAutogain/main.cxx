string name="XY";
string description="xy";

#include "WaveFile.hxx"
#include "filter.hxx"
#include "autoGain.hxx"

/*
*	in this script we add an autogain feature to the mix-filter
*	basically by processing a pink-noise sample and
*	capturing the resulting rms-values.
*/

array<double> inputParameters( inputParametersNames.length );
array<string> inputParametersNames = { "cutoff", "order", "type" };
array<string> inputParametersUnits = {};
array<string> inputParametersEnums = {,,"lp;hp"};
array<double> inputParametersMin = { 0., 1, 0 };
array<double> inputParametersMax = { 1., 10, 1 };
array<int> inputParametersSteps = { -1., 10, 2 };

DSP::AutoGainModule autoGainModule;
DSP::Filter filter;

void processBlock(BlockData& data){
	for(uint s = 0; s < data.samplesToProcess; ++s)
		for(uint ch = 0; ch < audioInputsCount; ++ch){
			filter.process(data.samples[ch][s], ch);
			autoGainModule.process(data.samples[ch][s]);
		}	
}

void updateInputParametersForBlock(const TransportInfo@ info){
	int type = rint(inputParameters[2]);
	int order = rint(inputParameters[1]);
	float cutoff = inputParameters[0];
	filter.setParameters(type, order, cutoff);
	autoGainModule.setParameters(type, order, cutoff);
}

int rint(double& d){
    if(d < 0)
        return int(d - .5);
    else
        return int(d + .5);
}