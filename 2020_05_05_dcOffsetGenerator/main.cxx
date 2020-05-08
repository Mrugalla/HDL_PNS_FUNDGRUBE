string name = "xy";
string description = "xy";

/*

another script that you can use to test envelope followers' impact

*/

array<string> inputParametersNames = {"dcOffset"};
array<string> inputParametersUnits = {};
array<string> inputParametersEnums = {};
array<double> inputParameters( inputParametersNames.length );
array<int> inputParametersSteps = { 21 };
array<double> inputParametersDefault = { 0.f };
array<double> inputParametersMin = { -1.f };
array<double> inputParametersMax = { 1.f };

float dcOffset;

void initialize(){
}

void processBlock(BlockData& data){
	for(uint i = 0; i < data.samplesToProcess; i++){
		for(uint ch = 0; ch < audioInputsCount; ch++){
			data.samples[ch][i] = dcOffset;
		}
	}
}

void updateInputParameters(){
	dcOffset = inputParameters[0];
}

int rint(double d){
	if(d < 0.f)
		return int(d - .5f);
	else
		return int(d + .5f);
}