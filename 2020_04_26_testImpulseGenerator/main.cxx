string name = "xy";
string description = "xy";

/*

this script will send out a test impulse of value 1 in regular intervals. you can
use it to test your envelope generators or envelope followers if you apply the
plugins that have them after an instance of pns with this script.

the polarity switch switches the polarity of one channel which enables you
to debug your mid/side processing.

*/

array<string> inputParametersNames = {"intervalSamples", "polarity"};
array<string> inputParametersUnits = {};
array<string> inputParametersEnums = {};
array<double> inputParameters( inputParametersNames.length );
array<int> inputParametersSteps = { -1, 2 };
array<double> inputParametersDefault = { 0.f, 0 };
array<double> inputParametersMin = { 5000.f, 1 };
array<double> inputParametersMax = { int(sampleRate), 1 };

int intervalParam;
int polarityParam;

bool sendImpulse = false;
int impulseIndex = 0;

void processBlock(BlockData& data){
	for(uint i = 0; i < data.samplesToProcess; i++){
		++impulseIndex;
		
		if(impulseIndex > intervalParam){
			impulseIndex = 0;
			if(sendImpulse)
				sendImpulse = false;
			else
				sendImpulse = true;
		}
		
		if(audioInputsCount == 1)
			if(sendImpulse)	
				data.samples[0][i] = 1.;
			else
				data.samples[0][i] = 0.;
		else
			if(polarityParam == 1)
				if(sendImpulse){
					data.samples[0][i] = 1.;
					data.samples[1][i] = -1.;
				}
				else{
					data.samples[0][i] = 0.;
					data.samples[1][i] = 0.;
				}
			else
				if(sendImpulse){
					data.samples[0][i] = 1.;
					data.samples[1][i] = 1.;
				}
				else{
					data.samples[0][i] = 0.;
					data.samples[1][i] = 0.;
				}
	}
}

void updateInputParameters(){
	intervalParam = int(inputParameters[0]);
	polarityParam = rint(inputParameters[1]);
}

int rint(double d){
	if(d < 0.f)
		return int(d - .5f);
	else
		return int(d + .5f);
}