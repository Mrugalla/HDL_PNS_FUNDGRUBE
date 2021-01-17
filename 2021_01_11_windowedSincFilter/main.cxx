string name="FIR SINC";
string description="test app";

// https://tomroelandts.com/articles/how-to-create-a-simple-low-pass-filter

array<string> inputParametersNames = { "Frequency (fc)", "Bandwidth (b)" };
array<string> inputParametersUnits = { "hz", "hz"};
array<string> inputParametersEnums = {};
array<double> inputParameters( inputParametersNames.length );
array<int> inputParametersSteps = {};
array<double> inputParametersMin = { 1000, 2000 };
array<double> inputParametersMax = { 20000, 6000 };

double tau = 6.28318530718;
double pi = tau * .5;
double tau2 = tau * 2.;

double fc = -1, b = -1;
int timer = int(sampleRate);

IR sincFilter;
Convolution cL, cR;
bool testInInit = false;

void initialize() {
	if(!testInInit) return;
	
	makeSincFilter(10000, 1000, 1000);
}

void processBlock(BlockData& data) {
	if(testInInit) return;
	
	processParameters(data);
	
	if(sincFilter.size() == 0) return;
	
	cL.processBlock(data, sincFilter, 0);
	cR.processBlock(data, sincFilter, 1);
}

void processParameters(BlockData& data) {
	if(fc != inputParameters[0] ||
		b != inputParameters[1]) {
			timer = 0;
			fc = inputParameters[0];
			b = inputParameters[1];
		}
	else {
		if(timer == -1) return;
		else if(timer < sampleRate) timer += data.samplesToProcess;
		else {
			timer = -1;
			sincFilter = makeSincFilter(sampleRate, fc, b);
		}
	}
}

double rint(double d) {
    if(d < 0)
		return d - .5;
    return d + .5;
}

double sinc(double x, double fc2) { x *= pi; return sin(fc2 * x) / x; }

// http://www.dspguide.com/ch16/1.htm
/* about 20% better roll-off than blackman. */
void synthesizeHammingWindow(array<double>& data, double N) {
	double TauNInv = tau / N;
	for(int n = 0; n < N; ++n)
		data[n] = .54 - .46 * cos(n * TauNInv);
}

/* better stopband-attenuation than hamming */
void synthesizeBlackmanWindow(array<double>& data, int N) {
	double max = N - 1;
	double maxInv = 1. / max;
	double maxInvTau = tau * maxInv;
	double max2InvTau = 2. * maxInvTau;
	for(int n = 0; n < N; ++n)
		data[n] = .42 - .5 * cos(maxInvTau * n) + .08 * cos(max2InvTau * n);
}

class IR {
	IR() { }
	IR(array<double> h, int l) {
		taps = h;
		latency = l;
	}
	int size() { return int(taps.length); }
	array<double> taps;
	int latency;
}

int getLatency() { return sincFilter.latency; }

/*
fc and b not allowed to be > Nyquist (Fs / 2)
*/
IR makeSincFilter(double Fs, double fc, double bw) {
	fc /= Fs; // cutoff frequency's ratio
	bw /= Fs;  // transition bandwith (roll-off) ratio
	int M = int(4. / bw); // aprox.
	int N = M;
	if(N % 2 == 0) ++N; // symmetrical FIR
	int latency = M / 2; // middle sample of FIR (highest peak)
	array<double> h(N); // make array of length N
	
	synthesizeBlackmanWindow(h, N); // synthesize blackman window
	
	double fc2 = fc * 2.;
	for(int n = 0; n < N; ++n) {
		int i = n - latency;
		if(i != 0)
			h[n] *= sinc(n - latency, fc2); // apply sinc function
		else h[n] *= fc2;
	}
	
	double sum = 0; // normalize
	for(int n = 0; n < N; ++n) sum += h[n];
	double sumInv = 1. / sum;
	for(int n = 0; n < N; ++n) h[n] *= sumInv;
		
	//dbg(h);
		
	return IR(h, latency);
}

void dbg(array<double>& h) {
	string t;
	for(uint n = 0; n < h.length; ++n)
		t += h[n] + " :: ";
	print(t);
}

class Convolution {
	void processBlock(BlockData& data, IR& ir, int ch) {
		if(buffer.length != ir.taps.length)
			buffer.length = ir.size();
		
		for(uint s = 0; s < data.samplesToProcess; ++s) {
			++wIdx;
			if(wIdx >= ir.size())
				wIdx = 0;
			buffer[wIdx] = data.samples[ch][s];
			
			double y = 0;
			int rIdx = wIdx;
			for(int i = 0; i < ir.size(); ++i) {
				y += buffer[rIdx] * ir.taps[i];
				
				--rIdx;
				if(rIdx == -1)
					rIdx = ir.size() - 1;
			}
			
			data.samples[ch][s] = y;
		}
	}
	
	array<double> buffer;
	int wIdx = 0;
}