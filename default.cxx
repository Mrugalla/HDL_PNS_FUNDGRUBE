string name="XY";
string description="xy";

array<string> inputParametersNames = {};
array<string> inputParametersUnits = {};
array<string> inputParametersEnums = {};
array<double> inputParameters(inputParametersNames.length);
array<int> inputParametersSteps = {};
array<double> inputParametersMin = {};
array<double> inputParametersMax = {};

const double tau = 6.28318530718;
const double pi = tau / 2;

void processBlock(BlockData& data) {
	processParameters();
	
	for(uint ch = 0; ch < audioOutputsCount; ++ch) {
		for(uint s = 0; s < data.samplesToProcess; ++s) {
			
		}
	}
}

void processParameters() {
	
}

double rint(double d) {
    if(d < 0.) return floor(d - .5);
    return floor(d + .5);
}