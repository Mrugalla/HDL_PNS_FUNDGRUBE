string name="XY";
string description="xy";

#include "msEncoder.hxx"

array<double> inputParameters( inputParametersNames.length );
array<string> inputParametersNames = { "width" };
array<string> inputParametersUnits = {};
array<string> inputParametersEnums = {};
array<double> inputParametersMin = { 0. };
array<double> inputParametersMax = { 2. };
array<double> inputParametersStep = { -1. };

float width;

void processBlock(BlockData& data){
	if(audioInputsCount == 2){
		MSProcessor::encode(data);
		
		/*
		
		instead of controlling mid and side individually
		we can also combine them to a single width-parameter
		that mixes in more mid below 1.f and more side above 1.f
		
		*/
		
		if(width < 1.f)
			for(uint s = 0; s < data.samplesToProcess; ++s)
			data.samples[1][s] *= width;
		else
			for(uint s = 0; s < data.samplesToProcess; ++s)
				data.samples[0][s] *= 2.f - width;
		
		
		MSProcessor::decode(data);
	}	
}

void updateInputParametersForBlock(const TransportInfo@ info){
    width = inputParameters[0];
}