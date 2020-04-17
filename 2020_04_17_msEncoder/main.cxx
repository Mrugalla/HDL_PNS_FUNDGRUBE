string name="XY";
string description="xy";

#include "msEncoder.hxx"

/*

in order to encode the audio data from l/r to m/s
we include the msEncoder.hxx file that contains the
namespace MSProcessor with the methods encode and decode.

*/

array<double> inputParameters( inputParametersNames.length );
array<string> inputParametersNames = { "mid", "side" };
array<string> inputParametersUnits = {};
array<string> inputParametersEnums = {};
array<double> inputParametersMin = { 0., 0. };
array<double> inputParametersMax = { 1., 1. };
array<double> inputParametersStep = { -1., -1 };

float mid, side;

void processBlock(BlockData& data){
	if(audioInputsCount == 2){
		MSProcessor::encode(data);
		
		/*
		
		After encoding we can now process channel 0 and 1 as if
		they were mid/side. in this example 2 parameters just
		handle the output gain of each.
		
		*/
		
		for(uint s = 0; s < data.samplesToProcess; ++s){
			data.samples[0][s] *= mid;
			data.samples[1][s] *= side;
		}
		
		MSProcessor::decode(data);
	}	
}

void updateInputParametersForBlock(const TransportInfo@ info){
    mid = inputParameters[0];
	side = inputParameters[1];
}