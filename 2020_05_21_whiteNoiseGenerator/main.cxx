string name = "xy";
string description = "xy";

array<string> inputParametersNames = {"amp"};
array<string> inputParametersUnits = {};
array<string> inputParametersEnums = {};
array<double> inputParameters( inputParametersNames.length );
array<int> inputParametersSteps = { -1 };
array<double> inputParametersDefault = { 0.f };
array<double> inputParametersMin = { 0.f };
array<double> inputParametersMax = { 1.f };

float amp;

void processBlock(BlockData& data){
	for(uint i = 0; i < data.samplesToProcess; i++){
		for(uint ch = 0; ch < audioInputsCount; ch++){
			data.samples[ch][i] = rand(-1.f, 1.f) * amp;
		}
	}
}

void updateInputParameters(){
	amp = inputParameters[0];
}

int rint(double d){
	if(d < 0.f)
		return int(d - .5f);
	else
		return int(d + .5f);
}