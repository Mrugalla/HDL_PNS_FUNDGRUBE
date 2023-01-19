string name="Beat Frequency Osc";
string description="xy";

array<double> inputParameters(inputParametersNames.length);
array<string> inputParametersNames = { "freq", "amp", "beat" };
array<string> inputParametersUnits = { "hz", "db", "" };
array<string> inputParametersEnums = { "", "", beatStrings };
array<int> inputParametersSteps = { -1, -1, 4 };
array<double> inputParametersMin = { 1., -60., 0 };
array<double> inputParametersMax = { 420., 0., 3 };

string beatStrings("1/4;1/8;1/16;1/32");

const double tau = 6.28318530718;
const double pi = tau / 2;

Phasor phasor0, phasor1;

void initialize()
{
}

void processBlock(BlockData& data)
{
	uint numSamples = data.samplesToProcess;
	
	double freqHz = inputParameters[0];
	phasor0.inc = freqHz / sampleRate;
	
	double bpm = data.transport.bpm;
	double bps = bpm / 60.;
	
	double beatFreq = pow(2, math::rint(inputParameters[2]));
	print(beatFreq);
	
	double freqOffset = bps * beatFreq;
	phasor1.inc = (freqHz + freqOffset) / sampleRate;
	
	phasor0.process(numSamples);
	phasor1.process(numSamples);
	
	double gain = math::dbInGain(inputParameters[1]);
	
	for(uint ch = 0; ch < audioOutputsCount; ++ch)
		for(uint s = 0; s < numSamples; ++s)
		{
			data.samples[ch][s] = sin(phasor0.buf[s] * tau) + sin(phasor1.buf[s] * tau);
			data.samples[ch][s] *= gain * .5f;
		}
			
	
}

void print(double i)
{
	print(i + "");
}

void print(float i)
{
	print(i + "");
}

void print(int i)
{
	print(i + "");
}

namespace math
{
	float rint(float d)
	{
		if(d < 0.f)
			return floor(d - .5f);
		return floor(d + .5f);
	}

	double rint(double d)
	{
		if(d < 0.)
			return floor(d - .5);
		return floor(d + .5);
	}

	double msInSamples(double ms)
	{
		return ms * sampleRate * .001;
	}

	float msInSamples(float ms, float Fs)
	{
		return ms * Fs * .001f;
	}

	double dbInGain(double db)
	{
		return pow(10., db / 20.);
	}

	float dbInGain(float db)
	{
		return pow(10.f, db / 20.f);
	}
	
	double gainInDb(double gain)
	{
		return 20. * log10(gain);
	}
	
	float gainInDb(float gain)
	{
		return 20.f * log10(gain);
	}
}

class WHead
{
	WHead()
	{
		buf.length = maxBlockSize;
	}
	
	void process(uint samplesToProcess)
	{
		for(uint i = 0; i < samplesToProcess; ++i)
		{
			buf[i] = idx;
			
			++idx;
			if(idx == size)
				idx = 0;
		}
	}
	
	array<int> buf;
	int idx, size;
}

class Phasor
{
	Phasor()
	{
		buf.length = maxBlockSize;
	}
	
	void process(uint samplesToProcess)
	{
		for(uint s = 0; s < samplesToProcess; ++s)
		{
			buf[s] = phase;
			
			phase += inc;
			if(phase >= 1.)
				--phase;
			if(phase < 0.)
				++phase;
		}
	}
	
	array<double> buf;
	double phase, inc;
}

class Window
{
	Window()
	{
		buf.length = maxBlockSize;
	}
	
	void process(array<double>& phasor, uint samplesToProcess)
	{
		for(uint s = 0; s < samplesToProcess; ++s)
			buf[s] = cos(phasor[s] * tau + pi) * .5 + .5;
	}
	
	array<double> buf;
}

class Delay
{
	Delay()
	{
		readHead.length = maxBlockSize;
	}
	
	void resize(int s)
	{
		size = s;
		for(uint ch = 0; ch < audioOutputsCount; ++ch)
			ringBuffer[ch].length = size + 4;
	}
	
	void process(array<array<double>>& samples, uint samplesToProcess,
		array<int>& wHead, array<double>& modulator/*[0, 1[*/,
		double feedback/*[-1,1]*/, array<double>& window)
	{
		for(uint s = 0; s < samplesToProcess; ++s)
		{
			double r = wHead[s] - modulator[s] * size;
			if(r < 0.)
				r += size;
			readHead[s] = r;
		}
			
		
		for(uint ch = 0; ch < audioOutputsCount; ++ch)
		{
			for(uint s = 0; s < samplesToProcess; ++s)
			{
				int w = wHead[s];
				double r = readHead[s];
				double wndw = window[s];
				
				double sOut = ringBuffer[ch][int(r)] * wndw;
				double sIn = samples[ch][s] + sOut * feedback;
				
				ringBuffer[ch][w] = sIn;
				samples[ch][s] = sOut;
			}
		}
	}
	
	array<float> readHead;
	array<array<float>> ringBuffer(audioOutputsCount);
	int size;
}
