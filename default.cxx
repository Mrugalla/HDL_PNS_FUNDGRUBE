string name="XY";
string description="xy";

array<double> inputParameters( inputParametersNames.length );
array<string> inputParametersNames = { "xy" };
array<string> inputParametersUnits = {};
array<string> inputParametersEnums = {};
array<double> inputParametersMin = { 0. };
array<double> inputParametersMax = { 1. };
array<double> inputParametersStep = { -1. };

void processBlock(BlockData& data){
	for(uint s = 0; s < data.samplesToProcess; ++s)
		for(uint ch = 0; ch < audioInputsCount; ++ch){
			
		}
}

void updateInputParametersForBlock(const TransportInfo@ info){
    
}

int rint(double& d){
    if(d < 0)
        return int(d - .5);
    else
        return int(d + .5);
}