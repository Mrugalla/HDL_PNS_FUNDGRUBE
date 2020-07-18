string name="Farey Crusher";
string description="crushes bits with farey algorithm, lol";

#include "farey.hxx"

array<string> inputParametersNames = { "Order" };
array<string> inputParametersUnits = {};
array<string> inputParametersEnums = {};
array<double> inputParameters( inputParametersNames.length );
array<int> inputParametersSteps = { maxOrder };
array<double> inputParametersMin = { 1 };
array<double> inputParametersMax = { maxOrder };

int maxOrder = 16;
hdl::Farey farey(maxOrder);
int order = -1;

void processBlock(BlockData& data) {
	updateParameters();
	for(uint s = 0; s < data.samplesToProcess; ++s)
		for(uint ch = 0; ch < audioOutputsCount; ++ch)
			data.samples[ch][s] = farey.process(data.samples[ch][s]);
}

void updateParameters(){
	int o = rint(inputParameters[0]);
	if(order != o){
		order = o;
		farey.setOrder(order);
	}
}


int rint(double d){
    if(d < 0.) return int(d - .5);
    else return int(d + .5);
}