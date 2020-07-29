string name="XY";
string description="xy";

#include "hdl.hxx"

array<string> inputParametersNames = { "x", "y", "feedback", "retuneSpeed" };
array<string> inputParametersUnits = {};
array<string> inputParametersEnums = {};
array<double> inputParameters( inputParametersNames.length );
array<int> inputParametersSteps = { 64, 64, -1, -1 };
array<double> inputParametersMin = { 1, 1, 0, .1 };
array<double> inputParametersMax = { 64, 64, 1, 1. };

hdl::FBDelay delayL, delayR;
hdl::RateHandler rateHandler;
double feedback = -1, retune = -1;

void processBlock(BlockData& data) {
	setParameters(data.transport.get_bpm());
	
	for(uint s = 0; s < data.samplesToProcess; ++s)
		data.samples[0][s] = delayL.process(data.samples[0][s]);
	if(audioOutputsCount != 1)
		for(uint s = 0; s < data.samplesToProcess; ++s)
			data.samples[1][s] = delayR.process(data.samples[1][s]);
}

void setParameters(double bpm) {
	if(rateHandler.process(bpm, inputParameters[0], inputParameters[1])) {
		delayL.setDelayInSamples(rateHandler.getRate());
		delayR.setDelayInSamples(rateHandler.getRate());
	}
	
	feedback = inputParameters[2];
	delayL.setFeedback(feedback);
	delayR.setFeedback(feedback);
	
	if(retune != inputParameters[3]) {
		retune = inputParameters[3];
		delayL.setRetuneInSecounds(retune);
		delayR.setRetuneInSecounds(retune);
	}
}

double rint(double d) {
    if(d < 0.) return floor(d - .5);
    else return floor(d + .5);
}