string name="XY";
string description="xy";

#include "stepSequencer.hxx"
#include "envelopeGenerator.hxx"

array<string> inputParametersNames = { "Rate", "Attack", "Decay", "Shape" };
array<string> inputParametersUnits = {,"ms","ms"};
array<string> inputParametersEnums = {"1/4;1/8;1/16;1/32;1/64"};
array<double> inputParameters( inputParametersNames.length );
array<int> inputParametersSteps = { 5, -1., -1., -1. };
array<double> inputParametersMin = { 0, 1., 1., -1. };
array<double> inputParametersMax = { 4, 10., 420., 1. };

hdl::StepSequencer sequencer;
hdl::EnvelopeGenerator envelope;
double attack = -1, decay = -1, shape = -2.;
int rate = -1;

void processBlock(BlockData& data){
	setParameters();
	sequencer.setTransportInfo(data.transport);
	
	for(uint s = 0; s < data.samplesToProcess; ++s){
		
		bool trigger = sequencer.process();
		double env = envelope.process(trigger);
		
		for(uint ch = 0; ch < audioOutputsCount; ++ch){
			data.samples[ch][s] *= env;
		}
	}
}

void setParameters(){
	if(rate != inputParameters[0]){
		rate = rint(inputParameters[0]);
		sequencer.setRate(rate);
	}
	if(attack != inputParameters[1]){
		attack = inputParameters[1];
		envelope.setAttackInMS(attack);
	}
	if(decay != inputParameters[2]){
		decay = inputParameters[2];
		envelope.setDecayInMS(decay);
	}
	if(shape != inputParameters[3]){
		shape = inputParameters[3];
		envelope.setShape(shape);
	}
}

int rint(double d){
    if(d < 0.) return int(d - .5);
    else return int(d + .5);
}