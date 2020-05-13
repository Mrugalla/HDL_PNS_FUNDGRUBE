string name = "xy";
string description = "xy";

#include "ConvolutionReverb.hxx"

/*

whenever this script is loaded it creates randomized impulse responses for the
convolutionReverb-objects and then you can mix dry/wet, dial in a 2nd set of
randomized ir-values with the width-knob, tune the playback speed of the
reverb with tune and change the reverb's gain with gain.

one thing to note though: this won't sound like a reverb. it sounds more like
a guitar amp cabinet simulation. that's just because you can't make
convolution buffers of a very large size without killing the cpu
in plug'n script. you could easily choose higher sizes in actual c++.

this script is just meant to give you an idea of the process.

*/

array<string> inputParametersNames = {"mix", "width", "tune", "gain"};
array<string> inputParametersUnits = {};
array<string> inputParametersEnums = {};
array<double> inputParameters( inputParametersNames.length );
array<int> inputParametersSteps = { -1, -1, -1, -1 };
array<double> inputParametersDefault = { 0.f, 0.f, 0.f, 0.f };
array<double> inputParametersMin = { 0.f, 0.f, .5f, 0.f };
array<double> inputParametersMax = { 1.f, 1.f, 2.f, 2.f };

ConvolutionReverb reverbL, reverbR, reverbRW;

float mix, width, gain;

void initialize(){ initRandomIRs(); }

void initRandomIRs(){
	/*
	
	the randomized values are being multiplied by an envelope to create fairly
	realistic impulse responses.
	
	*/
	
	float size = float(sampleRate / 2056.);
	array<float> ir1(int(size));
	float j = 0.f;
	for(uint i = 0; i < ir1.length; ++i, ++j){
		float envelope = pow(1.f - (j / (size - 1.f)), 2.f);
		ir1[i] = envelope * rand(-1.f, 1.f);
	}
	reverbL.setImpulseResponse(ir1);
	reverbR.setImpulseResponse(ir1);
	
	for(uint i = 0; i < ir1.length; ++i, ++j){
		float envelope = pow(1.f - (j / (size - 1.f)), 2.f);
		ir1[i] = envelope * rand(-1.f, 1.f);
	}
	reverbRW.setImpulseResponse(ir1);
}

void processBlock(BlockData& data){
	for(uint i = 0; i < data.samplesToProcess; i++){
		double dryL = data.samples[0][i];
		double dryR = data.samples[1][i];
		
		double inReverbR = data.samples[1][i];
		reverbL.process(data.samples[0][i]);
		reverbR.process(data.samples[1][i]);
		reverbRW.process(inReverbR);
		
		data.samples[1][i] += width * (inReverbR - data.samples[1][i]);
		
		data.samples[0][i] = dryL + mix * (gain * data.samples[0][i] - dryL);
		data.samples[1][i] = dryR + mix * (gain * data.samples[1][i] - dryR);
	}
}

void updateInputParameters(){
	mix = inputParameters[0];
	width = inputParameters[1];
	float tune = inputParameters[2];
	reverbL.setTune(tune);
	reverbR.setTune(tune);
	reverbRW.setTune(tune);
	gain = inputParameters[3];
}