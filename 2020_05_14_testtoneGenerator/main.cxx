string name = "xy";
string description = "xy";

/*
a little test tone generator. i made this so i can test the behaviour of 
fft algorithms but you can use it for whatever you want.
*/

array<string> inputParametersNames = {"autogain", "osc freq", "osc0", "osc1", "osc2", "osc3", "osc4", "osc5", "osc6", "osc7", "masterGain"};
array<string> inputParametersUnits = {};
array<string> inputParametersEnums = {};
array<double> inputParameters( inputParametersNames.length );
array<int> inputParametersSteps = { 2, -1 };
array<double> inputParametersDefault = { 0, 50.f };
array<double> inputParametersMin = { 0, 0.f };
array<double> inputParametersMax = { 1, sampleRate * .5 };

array<float>oscGain(8);
float masterGain;
int autogain, agIdx, agSize = int(sampleRate / 16.);
float agMax, agValue = 1.f;

Osc osc;

void initialize(){
}

void processBlock(BlockData& data){
	for(uint i = 0; i < data.samplesToProcess; i++){
		osc.process();
		double wave = 0.;
		for(uint h = 0; h < 8; ++h)
			wave += osc.get(h) * oscGain[h];
		
		processAutogain(wave);
		
		wave *= masterGain;
		
		for(uint ch = 0; ch < audioInputsCount; ch++)
			data.samples[ch][i] = wave;
	}
}

void processAutogain(double& wave){
	if(autogain == 1){
		if(agMax < wave)
		agMax = wave;
		++agIdx;
		if(!(agIdx < agSize)){
			agIdx = 0;
			agValue = 1.f / agMax;
			agMax = 0.f;
		}
		wave *= agValue;
	}
}

void updateInputParameters(){
	uint i = 0;
	autogain = rint(inputParameters[0]);
	++i;
	osc.setFreq(inputParameters[i]);
	++i;
	uint idx;
	for(uint h = 0; h < 8; ++h){
		idx = i + h;
		oscGain[h] = inputParameters[idx];
	}
	i = idx + 1;
	masterGain = inputParameters[i];
}

int rint(double d){
	if(d < 0.f)
		return int(d - .5f);
	else
		return int(d + .5f);
}

class Osc{
	double pi = 3.14159265359, piTwo = pi * 2., sampleRatePiInv = piTwo / sampleRate;
	double phase = 0., inc = 0.;
	array<double> harmonics(8);
	
	void setFreq(double freq){ inc = sampleRatePiInv * freq; }
	
	void process(){
		phase += inc;

		double j = 1.;
		for(uint i = 0; i < harmonics.length; ++i, ++j)
			harmonics[i] = sin(j * phase);
		
		if(phase > pi)
			phase -= piTwo;
	}
	
	double& get(uint i = 0){ return harmonics[i]; }
}