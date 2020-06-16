string name="XY";
string description="xy";

#include "hdl.hxx"

array<string> inputParametersNames = { "cutoff", "low sat", "hi sat", "low crush", "hi crush", "low amp", "hi amp" };
array<string> inputParametersUnits = {,,,,"db","db"};
array<string> inputParametersEnums = {};
array<double> inputParameters( inputParametersNames.length );
array<int> inputParametersStep = { -1, -1, -1, -1, -1, -1, -1 };
array<double> inputParametersMin = { 0, 0, 0, 0, 0, -6, -6 };
array<double> inputParametersMax = { 1, 1, 1, 1, 1, 6, 6 };

hdl::StereoFilter filter;
hdl::MBSaturation sat;
hdl::MBBitcrusher crusher;
hdl::MBGain gain;

void processBlock(BlockData& data){
	for(uint s = 0; s < data.samplesToProcess; ++s){
		for(uint ch = 0; ch < audioOutputsCount; ++ch){
			double sampleLow = filter.process(data.samples[ch][s], ch);
			double sampleHigh = data.samples[ch][s] - sampleLow;
			
			sat.process(sampleLow, sampleHigh);
			crusher.process(sampleLow, sampleHigh);
			gain.process(sampleLow, sampleHigh);
			
			data.samples[ch][s] = sampleLow + sampleHigh;
		}
	}
}

void updateInputParametersForBlock(const TransportInfo@ info){
	filter.setCutoff(inputParameters[0]);
	sat.setDrive(inputParameters[1], inputParameters[2]);
	crusher.setCrush(inputParameters[3], inputParameters[4]);
	gain.setGainInDb(inputParameters[5], inputParameters[6]);
}

int rint(double d){
    if(d < 0.) return int(d - .5);
    else return int(d + .5);
}