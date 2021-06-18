string name="XY";
string description="xy";

array<string> inputParametersNames = { "rate", "bias", "width" };
array<string> inputParametersUnits = { "hz" };
array<string> inputParametersEnums = {};
array<double> inputParameters(inputParametersNames.length);
array<int> inputParametersSteps = {};
array<double> inputParametersMin = { .01, 0, 0 };
array<double> inputParametersMax = { 20, 1, 1 };

const double tau = 6.28318530718;
const double pi = tau / 2;

RandLFOSpline lfo;

void initialize() { lfo.init(); }

void processBlock(BlockData& data) {
	processParameters();
	
	lfo.processBlock(data);
	
	
	for(uint ch = 0; ch < audioOutputsCount; ++ch)
		for(uint s = 0; s < data.samplesToProcess; ++s)
			if(abs(data.samples[ch][s]) >= 1.)
				print("overshoot at: " + data.samples[ch][s]);
}

void processParameters() {
	lfo.setRateInHz(inputParameters[0]);
	lfo.setBias(inputParameters[1]);
	lfo.setWidth(inputParameters[2]);
}

double rint(double d) {
    if(d < 0.) return floor(d - .5);
    return floor(d + .5);
}

namespace spline {
	int Size = 4;
	double process(array<double>& data, double x) {
		double iFloor = floor(x); 
		uint i0 = uint(iFloor);
		uint i1 = i0 + 1;
		uint i2 = i0 + 2;
		uint i3 = i0 + 3;
		
		double frac = x - iFloor;
		double v0 = data[i0];
		double v1 = data[i1];
		double v2 = data[i2];
		double v3 = data[i3];

		double c0 = v1;
		double c1 = .5f * (v2 - v0);
		double c2 = v0 - 2.5f * v1 + 2.f * v2 - .5f * v3;
		double c3 = 1.5f * (v1 - v2) + .5f * (v3 - v0);

		return ((c3 * frac + c2) * frac + c1) * frac + c0;
	}
	double processSafe(array<double>& data, double x) {
		double iFloor = floor(x); 
		uint i0 = uint(iFloor);
		uint i1 = (i0 + 1) % data.length;
		uint i2 = (i0 + 2) % data.length;
		uint i3 = (i0 + 3) % data.length;
		
		double frac = x - iFloor;
		double v0 = data[i0];
		double v1 = data[i1];
		double v2 = data[i2];
		double v3 = data[i3];

		double c0 = v1;
		double c1 = .5f * (v2 - v0);
		double c2 = v0 - 2.5f * v1 + 2.f * v2 - .5f * v3;
		double c3 = 1.5f * (v1 - v2) + .5f * (v3 - v0);

		return ((c3 * frac + c2) * frac + c1) * frac + c0;
	}
	double getMaxOvershoot() {
		array<double> sqr(Size);
		for(int s = 0; s < Size; ++s) {
			double ratio = double(s) / Size;
			sqr[s] = rint(ratio) * 2. - 1.;
		}
		array<double> y(Size);
		for(int s = 0; s < Size; ++s) {
			double x = 2. * double(s) / Size; 
			y[s] = processSafe(sqr, x);
		}
		double max = 0.;
		for(int s = 0; s < Size; ++s) {
			double absSample = abs(y[s]);
			if(max < absSample)
				max = absSample;
		}
		return max;
	}
}

class RandLFOSpline {
	// PARAM
	void setRateInHz(double r) {
		double rateInHz = r;
		inc = rateInHz / sampleRate;
	}
	void setWidth(double w) { width = w; }
	void setBias(double b) { bias = b; }
	
	// PROCESS
	void init() {
		for(uint ch = 0; ch < audioOutputsCount; ++ch) {
			randValue[ch].length = spline::Size * 2;
			for(int r = 0; r < spline::Size; ++r)
				randValue[ch][r] = 0.;
		}
		phase = 1;
		inc = 0;
		rIdx = 0;
		
		overshootGainCompensation = 1. / spline::getMaxOvershoot();
	}
	
	void processBlock(BlockData& data) {
		for(uint s = 0; s < data.samplesToProcess; ++s) {
			phase += inc;
			if(phase >= 1) {
				--phase;
				++rIdx;
				if(rIdx == spline::Size)
					rIdx = 0;
				for(uint ch = 0; ch < audioOutputsCount; ++ch) {
					double newValue = (getBias(rand()) * 2. - 1.) * overshootGainCompensation;
					randValue[ch][rIdx] = newValue;
					randValue[ch][rIdx + spline::Size] = newValue;
				}
			}
			double x = rIdx + phase + 1;
			if(x >= spline::Size) x -= spline::Size;
			for(uint ch = 0; ch < audioOutputsCount; ++ch)
				data.samples[ch][s] = spline::process(randValue[ch], x);
		}
	
		double narrow = 1 - width;
		for(uint ch = 1; ch < audioOutputsCount; ++ch)
			for(uint s = 0; s < data.samplesToProcess; ++s)
				data.samples[ch][s] += narrow * (data.samples[0][s] - data.samples[ch][s]);
	}
	
	private array<array<double>> randValue(audioOutputsCount);
	private double phase, inc, width, bias, overshootGainCompensation;
	private int rIdx;
	
	double getBias(double value) {
		if(bias < .5f) {
			double a = bias * 2;
			return atan(tan(value * pi - .5 * pi) * a) / pi + .5;
		}
		double a = 1. / (1. - (2. * bias - 1.));
		return atan(tan(value * pi - .5 * pi) * a) / pi + .5;
	}
}