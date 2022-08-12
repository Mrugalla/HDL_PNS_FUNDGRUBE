string name="XY";
string description="xy";

array<double> inputParameters(inputParametersNames.length);

array<string>  inputParametersNames=	{ "smooth1", "smooth2", "dly", "smoothLen" };
array<string>  inputParametersUnits=	{ "", "", "", "ms" };
array<string>  inputParametersEnums=	{ "noSmooth;block;linear;lp1x;lp2x", "noSmooth;linear;lp1x;lp2x" };
array<double> inputParametersMin=		{ 0., 0., 0., 1. };
array<double> inputParametersMax=		{ 4., 3., 1., 120. };
array<int> inputParametersSteps=		{ 5,  4, -1, -1 };

const float pi = 3.14159265359f;
const float tau = pi * 2.f;

array<double> rateBuffer(maxBlockSize);

smooth::Block delaySmoothBlock;
smooth::Linear delaySmoothLin, delaySmoothLin2;
smooth::Lowpass delaySmoothLP, delaySmoothLP2x;
smooth::Lowpass delaySmoothLP2, delaySmoothLP2x2;

const int delaySize = int(sampleRate * .1);
dsp::WHead wHead;
dsp::RHead rHead;
dsp::Delay delay;

void initialize()
{
	wHead.resize(delaySize);
	rHead.resize(delaySize);
	delay.resize(delaySize);
}

void processBlock(BlockData& data)
{
	uint numSamples = data.samplesToProcess;
	
	int smoothType1 = int(math::rint(inputParameters[0]));
	int smoothType2 = int(math::rint(inputParameters[1]));
	double rate = inputParameters[2] * double(delaySize - 1);
	double smoothLen = math::msToSamples(inputParameters[3]);

	switch (smoothType1)
	{
	case 0: // no smoothing
		for (uint s = 0; s < numSamples; ++s)
			rateBuffer[s] = rate;
		break;
	case 1: // block
		delaySmoothBlock.process(rateBuffer, rate, numSamples);
		break;
	case 2: // lin smoothing
		delaySmoothLin.process(rateBuffer, rate, smoothLen, numSamples);
		break;
	case 3: // lowpass smoothing
		delaySmoothLP.process(rateBuffer, rate, smoothLen, numSamples);
		break;
	case 4: // lowpass 2x smoothing
		delaySmoothLP.process(rateBuffer, rate, smoothLen, numSamples);
		delaySmoothLP2x.process(rateBuffer, smoothLen, numSamples);
		break;
	default: // no smoothing
		for(uint s = 0; s < numSamples; ++s)
			rateBuffer[s] = rate;
		break;
	}
	
	switch (smoothType2)
	{
	case 1: // lin smoothing
		delaySmoothLin2.process(rateBuffer, smoothLen, numSamples);
		break;
	case 2: // lowpass smoothing
		delaySmoothLP2.process(rateBuffer, smoothLen, numSamples);
		break;
	case 3: // lowpass 2x smoothing
		delaySmoothLP2.process(rateBuffer, smoothLen, numSamples);
		delaySmoothLP2x2.process(rateBuffer, smoothLen, numSamples);
		break;
	}
	
	wHead.process(numSamples);
	rHead.process(wHead.buffer, rateBuffer, numSamples);
	delay.process(data.samples, numSamples, wHead.buffer, rHead.buffer);
}

namespace math
{
	double log2(double x)
	{
		return log(x) * 3.32192809489;
	}

	float log2(float x)
	{
		return log(x) * 3.32192809489f;
	}

	double rint(double x)
	{
		return floor(x + .5);
	}

	float rint(float x)
	{
		return floor(x + .5);
	}

	double todB(double x)
	{
		return 20. * log10(x);
	}

	float todB(float x)
	{
		return 20.f * log10(x);
	}

	double fromdB(double x)
	{
		return pow(10., x * .05);
	}

	float fromdB(float x)
	{
		return pow(10.f, x * .05f);
	}

	double noteToHz(double note, double masterTune = 440., double baseNote = 69., double xen = 12.)
	{
		return pow(2., (note - baseNote) / xen) * masterTune;
	}

	float noteToHz(float note, float masterTune = 440.f, float baseNote = 69.f, float xen = 12.f)
	{
		return pow(2.f, (note - baseNote) / xen) * masterTune;
	}
	
	double hzToNote(double hz, double masterTune = 440., double baseNote = 69., double xen = 12.)
	{
		return xen * log2(hz / masterTune) + baseNote;
	}

	float hzToNote(float hz, float masterTune = 440.f, float baseNote = 69.f, float xen = 12.f)
	{
		return xen * log2(hz / masterTune) + baseNote;
	}

	double msToSamples(double ms)
	{
		return sampleRate * ms * .001;
	}

	float msToSamples(float ms)
	{
		return sampleRate * ms * .001f;
	}
}

namespace interpolate
{
	double lerp(array<double>& buffer, double x, int size)
	{
		double xFloor = floor(x);
		int i0 = int(xFloor);
		int i1 = i0 + 1;
		if (i1 == size)
			i1 = 0;
		double xFrac = x - xFloor;
		double x0 = buffer[i0];
		double x1 = buffer[i1];
		return x0 + xFrac * (x1 - x0);
	}

	double cubicHermiteSpline(array<double>& buffer, double x, int size)
	{
		double xFloor = floor(x);
		int i0 = int(xFloor);
		int i1 = i0 + 1;
		int i2 = i0 + 2;
		int i3 = i0 + 3;
		if (i3 >= size) i3 -= size;
		if (i2 >= size) i2 -= size;
		if (i1 >= size) i1 -= size;
		if (i0 >= size) i0 -= size;

		double t = x - xFloor;

		double v0 = buffer[i0];
		double v1 = buffer[i1];
		double v2 = buffer[i2];
		double v3 = buffer[i3];

		double c0 = v1;
		double c1 = .5 * (v2 - v0);
		double c2 = v0 - 2.5 * v1 + 2. * v2 - .5 * v3;
		double c3 = 1.5 * (v1 - v2) + .5 * (v3 - v0);

		return ((c3 * t + c2) * t + c1) * t + c0;
	}
}

namespace smooth
{
	class Block
	{
		void process(array<double>& buffer, double x, uint numSamples)
		{
			if(cur == x)
				for (uint s = 0; s < numSamples; ++s)
					buffer[s] = x;
			else
			{
				double dif = x - cur;
				double inc = dif / double(numSamples);
				for (uint s = 0; s < numSamples; ++s)
				{
					buffer[s] = cur;
					cur += inc;
				}
			}
		}

		double cur;
	};

	class Linear
	{
		Linear(double startVal = 0.)
		{
			start = startVal;
			end = startVal;
			range = 0.;
			phase = 2.;
		}
		
		void process(array<double>& buffer, double x, double length, uint numSamples)
		{
			for (uint s = 0; s < numSamples; ++s)
			{
				if (phase > 1.) // current end value has been reached
				{
					if(x != end) // new end required
					{
						start = end;
						end = x;
						range = end - start;
						phase = 0.;
						inc = 1. / length;
					}
					else // no smoothing needed
					{
						start = end = x;
						range = .5;
						inc = 0.;
						phase = 2.;
					}
				}

				buffer[s] = start + phase * range;
				phase += inc;
			}
		}

		void process(array<double>& buffer, double length, uint numSamples)
		{
			for (uint s = 0; s < numSamples; ++s)
			{
				if (phase > 1.) // current end value has been reached
				{
					double x = buffer[s];

					if (x != end) // new end required
					{
						start = end;
						end = x;
						range = end - start;
						phase = 0.;
						inc = 1. / length;
					}
					else // no smoothing needed
					{
						start = end = x;
						range = .5;
						inc = 0.;
						phase = 2.;
					}
				}

				buffer[s] = start + phase * range;
				phase += inc;
			}
		}

		double start, end, range, phase, inc;
	};

	class Lowpass
	{
		Lowpass(double startVal = 0.)
		{
			y1 = startVal;
		}

		void process(array<double>& buffer, double x, double length, uint numSamples)
		{
			makeFromDecayInSamples(length);

			for (uint s = 0; s < numSamples; ++s)
				buffer[s] = processSample(x);
		}

		void process(array<double>& buffer, double length, uint numSamples)
		{
			makeFromDecayInSamples(length);

			for (uint s = 0; s < numSamples; ++s)
				buffer[s] = processSample(buffer[s]);
		}

		double y1, a0, b1;

		private double processSample(double x0)
		{
			y1 = x0 * a0 + y1 * b1;
			return y1;
		}

		private void makeFromDecayInSamples(double d)
		{
			setX(exp(-1. / d));
		}

		private void setX(double x)
		{
			a0 = 1. - x;
			b1 = x;
		}
	};
}

namespace dsp
{
	class WHead
	{
		WHead()
		{
			buffer.length = maxBlockSize;
			idx = 0;
		}
		
		void resize(int delaySize)
		{
			size = delaySize;
		}
		
		void process(uint numSamples)
		{
			for (uint s = 0; s < numSamples; ++s)
			{
				buffer[s] = idx;
				idx = (idx + 1) % size;
			}
		}

		array<int> buffer;
		int idx, size;
	};

	class RHead
	{
		RHead()
		{
			for(uint ch = 0; ch < audioOutputsCount; ++ch)
			{
				buffer[ch].length = maxBlockSize;
			}
		}

		void resize(int s)
		{
			sizeD = double(s);
		}
		
		void process(array<int>& wHead, double x/*[0,sizeD[*/, uint numSamples)
		{
			for (uint s = 0; s < numSamples; ++s)
			{
				double w = double(wHead[s]);
				double r = w - x;
				if (r < 0.)
					r += sizeD;
				buffer[0][s] = r;
			}

			for (uint ch = 1; ch < audioOutputsCount; ++ch)
				for (uint s = 0; s < numSamples; ++s)
					buffer[ch][s] = buffer[0][s];
		}

		void process(array<int>& wHead, array<double> x/*[0,sizeD[*/, uint numSamples)
		{
			for (uint s = 0; s < numSamples; ++s)
			{
				double w = double(wHead[s]);
				double r = w - x[s];
				if (r < 0.)
					r += sizeD;
				buffer[0][s] = r;
			}

			for (uint ch = 1; ch < audioOutputsCount; ++ch)
				for (uint s = 0; s < numSamples; ++s)
					buffer[ch][s] = buffer[0][s];
		}

		array<array<double>> buffer(audioOutputsCount);
		double sizeD;
	};

	class Delay
	{
		Delay()
		{
			
		}

		void resize(int s)
		{
			size = s;
			for (uint ch = 0; ch < audioOutputsCount; ++ch)
			{
				ringBuffer[ch].length = size;
			}
		}

		void process(array<array<double>>& samples, uint numSamples,
			array<int>& wHead, array<array<double>>& rHead)
		{
			for (uint ch = 0; ch < audioOutputsCount; ++ch)
			{
				for (uint s = 0; s < numSamples; ++s)
				{
					int w = wHead[s];
					double r = rHead[ch][s];

					double x = samples[ch][s];
					double y = interpolate::cubicHermiteSpline(ringBuffer[ch], r, size);

					ringBuffer[ch][w] = x;
					samples[ch][s] = y;
				}
			}
		}
		
		array<array<double>> ringBuffer(audioOutputsCount);
		int size;
	};
}






