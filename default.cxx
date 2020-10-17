string name="XY";
string description="xy";

array<string> inputParametersNames = {};
array<string> inputParametersUnits = {};
array<string> inputParametersEnums = {};
array<double> inputParameters(inputParametersNames.length);
array<int> inputParametersSteps = {};
array<double> inputParametersMin = {};
array<double> inputParametersMax = {};

double tau = 6.28318530718;
double pi = tau / 2;

void processBlock(BlockData& data) {
	processParameters();
	
	for(uint s = 0; s < data.samplesToProcess; ++s) {
		
		for(uint ch = 0; ch < audioInputsCount; ++ch){
			
		}
	}
		
}

void processParameters() {
	
}

int rint(double d) {
    if(d < 0)
        return int(d - .5);
    return int(d + .5);
}