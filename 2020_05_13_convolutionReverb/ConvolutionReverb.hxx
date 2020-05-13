class ConvolutionReverb{
	array<float> impulseResponse;
	array<double> delay;
	int size;
	int idx = 0;
	float tune = 1.f;

	void setImpulseResponse(array<float>& ir){
		impulseResponse = ir;
		size = ir.length;
		delay.length = size;
	}
	
	void setTune(float& t){ tune = t; }
	
	private double getInterpolatedDelay(float& i){
		int iFloor = int(i);
		int iCeil = (iFloor + 1) % size;
		float iFrac = i - iFloor;
		
		
		return delay[iFloor] + iFrac * (delay[iCeil] - delay[iFloor]);
	}
	
	void process(double& sample){
		++idx %= size;
		delay[idx] = sample;
		
		sample = 0.;
		float irIdx = 0.f;
		for(int i = 0; i < size; ++i, irIdx += tune){
			if(!(irIdx < size))
				return;
			float tIdx = idx - irIdx;
			if(tIdx < 0.f)
				tIdx = size + tIdx - 1;
			
			
			sample += delay[int(tIdx)] * impulseResponse[i];
		}
	}
}