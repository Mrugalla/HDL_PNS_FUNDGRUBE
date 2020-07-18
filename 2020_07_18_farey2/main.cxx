string name="Farey Crusher";
string description="crushes bits with farey algorithm, lol";

#include "farey2.hxx"

array<string> inputParametersNames = { "Order", "Crunch", "Burst", "Out Gain" };
array<string> inputParametersUnits = {,,,"%"};
array<string> inputParametersEnums = {};
array<double> inputParameters( inputParametersNames.length );
array<int> inputParametersSteps = { maxOrder, -1., -1., -1 };
array<double> inputParametersMin = { 1, 0., 0., 0. };
array<double> inputParametersMax = { maxOrder, .5, .5, 200. };

int maxOrder = 16;
hdl::Farey farey2;
double maxStep = -1, stepDrop = -1, outGain;
int order = -1;

void processBlock(BlockData& data) {
	updateParameters();
	
	for(uint s = 0; s < data.samplesToProcess; ++s)
		for(uint ch = 0; ch < audioOutputsCount; ++ch)
			data.samples[ch][s] = farey.process(data.samples[ch][s]) * outGain;
}

void updateParameters(){
	int o = rint(inputParameters[0]);
	if(order != o){
		order = o;
		farey.setOrder(order);
	}
	if(maxStep != inputParameters[1]){
		maxStep = inputParameters[1];
		farey.setMaxStep(maxStep);
	}
	if(stepDrop != inputParameters[2]){
		stepDrop = inputParameters[2];
		farey.setStepDrop(stepDrop);
	}
	outGain = inputParameters[3] / 100.;
}


int rint(double d){
    if(d < 0.) return int(d - .5);
    else return int(d + .5);
}