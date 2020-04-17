namespace DSP{
	class SampleAndHold{
		void setDrive(float d){ drive = 1.f - tanh(2.f * d); }
		void process(double& sample, uint ch){
			if(idx > 1.f) {
				envelope[ch] = sample;
				if(ch == audioInputsCount - 1)
					idx -= 1.f;
			}
			else {
				sample = envelope[ch];
				if(ch == audioInputsCount - 1)
					idx += drive;
			}
		}
		
		float idx, drive;
		array<double> envelope(2);
	}
}