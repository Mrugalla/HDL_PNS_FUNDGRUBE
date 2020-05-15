string name = "xy";
string description = "xy";

/*
a little test tone generator. i made this so i can test the behaviour of 
fft algorithms but you can use it for whatever you want.
*/

array<string> inputParametersNames = {"osc freq", "osc0", "osc1", "osc2", "osc3", "osc4", "osc5", "osc6", "osc7", "masterGain"};
array<string> inputParametersUnits = {};
array<string> inputParametersEnums = {};
array<double> inputParameters( inputParametersNames.length );
array<int> inputParametersSteps = { -1, -1 };
array<double> inputParametersDefault = { 50.f, 0.f };
array<double> inputParametersMin = { 0.f, 0.f };
array<double> inputParametersMax = { float(sampleRate * .5), 1.f };

array<float>oscGain(8);
float masterGain;

Osc osc;

void initialize(){
}

void processBlock(BlockData& data){
	for(uint i = 0; i < data.samplesToProcess; i++){
		osc.process();
		double wave = 0.;
		for(uint h = 0; h < 8; ++h)
			wave += osc.get(h) * oscGain[h];
		wave *= masterGain;
		
		for(uint ch = 0; ch < audioInputsCount; ch++)
			data.samples[ch][i] = wave;
	}
}

void updateInputParameters(){
	uint i = 0;
	osc.setFreq(inputParameters[i]);
	++i;
	while(i <= 8){
		oscGain[i - 1] = inputParameters[i];
		++i;
	}
	masterGain = inputParameters[i];
}

int rint(double d){
	if(d < 0.f)
		return int(d - .5f);
	else
		return int(d + .5f);
}

class Osc{
	double sampleRateInv = 2. / sampleRate, pi = 3.14159265359;
	double phase = 0., inc = 0., piTwo = pi * 2.;
	array<double> harmonics(8);
	
	void setFreq(double freq){ inc = pi * sampleRateInv * freq; }
	
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