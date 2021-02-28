string name="XY";
string description="xy";

array<string> inputParametersNames = { "freq", "gain", "wtPos", "waveform", "interpolator", "alpha" };
array<string> inputParametersUnits = { "hz" };
array<string> inputParametersEnums = {,,,"SIN;SAW;SQR","Int;Lin;Cubic;Lanczos"};
array<double> inputParameters(inputParametersNames.length);
array<int> inputParametersSteps = { -1, -1, wtSize, waveformCount, interpolationCount, alphaMax - alphaMin + 1 };
array<double> inputParametersMin = { 20, 0, 0, 0, 0, alphaMin };
array<double> inputParametersMax = { 200, 1, wtSize - 1, waveformCount - 1, interpolationCount - 1, alphaMax };

const double tau = 6.28318530718;
const double pi = tau / 2;

const int wtSize = 32;
const int waveformCount = 3;
const int alphaMax = 12, alphaMin = 2;
const int interpolationCount = 4;
enum Waveform { SIN, SAW, SQR }
enum InterpolationType { Int, Lin, Cubic, Lanczos }

Phase phase;
Wavetable2d wt(wtSize);
Interpolator interpolator;
Syn synth;
double gain;
int wtPos, waveform = -1;

void processBlock(BlockData& data) {
	processParameters();
	
	phase.synthesizeBlock(data);
	synth.processBlock(data, wt.get(wtPos), interpolator, gain);
}

void processParameters() {
	phase.setFreq(inputParameters[0]);
	gain = inputParameters[1];
	wtPos = int(rint(inputParameters[2]));
	makeWaveform(int(rint(inputParameters[3])));
	interpolator.type = int(rint(inputParameters[4]));
	interpolator.alpha = int(rint(inputParameters[5]));
}

void makeWaveform(int wf) {
	if(waveform != wf) {
		waveform = wf;
		
		switch(waveform) {
			case Waveform::SIN: makeSIN(); break;
			case Waveform::SAW: makeSAW(); break;
			case Waveform::SQR: makeSQR(); break;
		}
	}
}
void makeSIN() {
	for(int w = 0; w < wtSize; ++w) {
		int curSize = 2 + w * 2;
		array<double> y(curSize);
		for(uint i = 0; i < y.length; ++i) {
			double x = double(i) / double(y.length);
			y[i] = cos(x * tau);
		}
		wt.set(y, w);
	}
}
void makeSAW() {
	for(int w = 0; w < wtSize; ++w) {
		int curSize = 2 + w * 2;
		array<double> y(curSize);
		double margin = .5 / curSize;
		for(uint i = 0; i < y.length; ++i) {
			double x = double(i) / double(y.length);
			y[i] = 2 * (x + margin) - 1;
		}
		wt.set(y, w);
	}
}
void makeSQR() {
	for(int w = 0; w < wtSize; ++w) {
		int curSize = 2 + w * 2;
		array<double> y(curSize);
		for(uint i = 0; i < y.length; ++i) {
			double x = double(i) / double(y.length);
			y[i] = 2 * floor(2 * x) - 1;
		}
		wt.set(y, w);
	}
}

double rint(double d) {
    if(d < 0) return floor(d - .5);
    return floor(d + .5);
}

double sinc(double xPi) { return sin(xPi) / xPi; }

class Phase {
	void setFreq(double f) { inc = f / sampleRate; }
	void synthesizeBlock(BlockData& data) {
		for(uint s = 0; s < data.samplesToProcess; ++s) {
			phase += inc;
			if(phase >= 1)
				--phase;
			data.samples[0][s] = phase;
		}
	}
	double phase = 1, inc = 0;
}

class Wavetable {
	void set(array<double>& w) { wt = w; size = wt.length; }
	
	array<double> wt;
	int size;
}

class Wavetable2d {
	Wavetable2d(int s) {
		wt.length = s;
		size = s;
	}
	void set(array<double>& a, int index) { wt[index].set(a); }
	Wavetable& get(int index) { return wt[index]; }
	
	array<Wavetable> wt;
	int size;
}

class Interpolator {
	double process(array<double>& buffer, double readHead) {
		switch(type) {
			case InterpolationType::Int: return processInt(buffer, readHead);
			case InterpolationType::Lin: return processLin(buffer, readHead);
			case InterpolationType::Cubic: return processCubic(buffer, readHead);
			case InterpolationType::Lanczos: return processLanczos(buffer, readHead);
		}
		return 0;
	}
	
	int iMod(int idx, int size) {
		while (idx >= size) idx -= size;
		while (idx < 0) idx += size;
		return idx;
	}
	
	int type, alpha;
	
	double processInt(array<double>& buffer, double readHead) { return buffer[int(readHead)]; }
	double processLin(array<double>& buffer, double readHead) {
		int iFloor = int(readHead);
		int iCeil = (iFloor + 1) % buffer.length;
		double m = readHead - iFloor;
		return buffer[iFloor] + m * (buffer[iCeil] - buffer[iFloor]);
	}
	double processCubic(array<double>& buffer, double& readHead) {
		int sze = int(buffer.length);
		int idx1 = int(readHead);
		double x = readHead - idx1;
		int idx0 = idx1 - 1;
		int idx2 = idx1 + 1;
		int idx3 = idx1 + 2;
		if(idx0 < 0)
			idx0 += sze;
		if(idx3 >= sze)
			idx3 -= sze;
		if(idx2 >= sze)
			idx2 -= sze;
		
		double pow2 = x * x, pow3 = pow2 * x, y = 0;

		y += buffer[idx0] * (-pow3 + 2. * pow2 - x);
		y += buffer[idx1] * (3. * pow3 - 5. * pow2 + 2.);
		y += buffer[idx2] * (-3. * pow3 + 4. * pow2 + x);
		y += buffer[idx3] * (pow3 - pow2);
		
		return y / 2.;
	}
	double processLanczos(array<double>& buffer, double& readHead) {
		int iFloor = int(readHead);
		double x = readHead - iFloor;
		
		double sum = 0;
		for(int i = -alpha + 1; i < alpha; ++i) {
			int iLegal = iMod(i + iFloor, buffer.length);
			
			double xi = x - i;
			if(xi == 0) xi = 1;
			else if(x > -alpha && x < alpha) {
				double xPi = xi * pi;
				xi = sinc(xPi) * sinc(xPi / alpha);
			}
			else xi = 0;
			
			sum += buffer[iLegal] * xi;
		}
		return sum;
	}
}

class Syn {
	void processBlock(BlockData& data, Wavetable& wt, Interpolator& interpolator, double amp) {
		for(uint s = 0; s < data.samplesToProcess; ++s) {
			double readHead = data.samples[0][s] * wt.size;
			data.samples[0][s] = interpolator.process(wt.wt, readHead) * amp;
		}
		
		for(uint ch = 1; ch < audioOutputsCount; ++ch)
			for(uint s = 0; s < data.samplesToProcess; ++s)
				data.samples[ch][s] = data.samples[0][s];
	}
}