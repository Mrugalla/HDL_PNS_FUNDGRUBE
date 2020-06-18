string name="XY";
string description="xy";

#include "hdl.hxx"

array<string> inputParametersNames = { "in gain", "room size", "feedback", "filterType", "filter", "mix", "out gain" };
array<string> inputParametersUnits = {};
array<string> inputParametersEnums = {,,,"LP;HP",,};
array<double> inputParameters( inputParametersNames.length );
array<int> inputParametersSteps = { -1, -1, -1, 2, -1, -1, -1 };
array<double> inputParametersMin = { 1, 0, 0, 0, 0, 0, 1 };
array<double> inputParametersMax = { 3, size, .6, 1, 1, 1, 3 };

int size = int(sampleRate / 12.);
hdl::Reverb rvb(size, .3f);
float inGain, mix, outGain;

void processBlock(BlockData& data){
	for(uint s = 0; s < data.samplesToProcess; ++s){
		for(uint ch = 0; ch < audioOutputsCount; ++ch){
			data.samples[ch][s] = hdl::mix(
				data.samples[ch][s],
				inGain * rvb.process(data.samples[ch][s], ch),
				mix) * outGain;
		}
	}
}

void updateInputParametersForBlock(const TransportInfo@ info){
	inGain = inputParameters[0];
	rvb.setRoomSize(inputParameters[1]);
	rvb.setFeedback(inputParameters[2]);
	rvb.setFilterType(rint(inputParameters[3]));
	rvb.setFilterCutoff(inputParameters[4]);
	mix = inputParameters[5];
	outGain = inputParameters[6];
}

int rint(double d){
    if(d < 0.) return int(d - .5);
    else return int(d + .5);
}