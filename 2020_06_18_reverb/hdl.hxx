namespace hdl{

	float pi = 3.14159265359f;
	float mix(float a, float b, float p){ return a + p * (b - a); }
	
	// our reverb unit. just a filter and 7 fb-delays.
	class Reverb{
		Reverb(int s, float fb = .1f){
			resize(s);
			setFeedback(fb);
		}
		
		void resize(int s){
			dly0.resize(s);
			dly1.resize(s);
			dly2.resize(s);
			dly3.resize(s);
			dly4.resize(s);
			dly5.resize(s);
			dly6.resize(s);
			dly7.resize(s);
		}
		void setFilterCutoff(float cutoff){
			filterL.setCutoff(cutoff);
			filterR.setCutoff(cutoff);
		}
		void setFilterType(int t){
			filterL.setType(t);
			filterR.setType(t);
		}
		void setRoomSize(float d){
			dly0.setDelay(d, d);
			dly1.setDelay(d / 2, d / 3);
			dly2.setDelay(d / 2, d / 3);
			dly3.setDelay(d / 5, d / 8);
			dly4.setDelay(d / 5, d / 8);
			dly5.setDelay(d / 13, d / 21);
			dly6.setDelay(d / 13, d / 21);
			dly7.setDelay(d / 34, d / 34);
		}
		void setFeedback(float fb){
			dly0.setFeedback(fb / 34, fb / -34);
			dly1.setFeedback(fb / -21, fb / 21);
			dly2.setFeedback(fb / 13, fb / -13);
			dly3.setFeedback(fb / -8, fb / 8);
			dly4.setFeedback(fb / 5, fb / -5);
			dly5.setFeedback(fb / -3, fb / 3);
			dly6.setFeedback(fb / 2, fb / -2);
			dly7.setFeedback(fb, fb);
		}
		double process(double sample, uint ch){
			switch(ch){
				case 0: sample = filterL.process(sample); break;
				case 1: sample = filterR.process(sample); break;
			}
			
			return (
				dly0.process(sample, ch) + 
				dly1.process(sample, ch) +
				dly2.process(sample, ch) +
				dly3.process(sample, ch) +
				dly4.process(sample, ch) + 
				dly5.process(sample, ch) +
				dly6.process(sample, ch) +
				dly7.process(sample, ch)) / 8.;
		}
		
		private Filter filterL, filterR;
		private FBDelayStereo dly0, dly1, dly2, dly3, dly4, dly5, dly6, dly7;
	}
	
	
	// a simple filter that can be low- and highpass.
	enum FilterType{ LP = 0, HP }
	class Filter{
		Filter(){ cutoff = .15f; }
		double process(double sample){
			switch(type){
				case FilterType::LP:
					envelope = mix(envelope, sample, 1.f - tanh(2 * cutoff));
					return envelope;
				case FilterType::HP:
					envelope = mix(envelope, sample, pow(cutoff, 2));
					return sample - envelope;
			}
			return 0.;
		}
		void setCutoff(float co){ cutoff = co; }
		void setType(int t){ type = t; }
		
		private double envelope, cutoff;
		private int type = FilterType::LP;
	}
	
	
	// smoothens out delay-parameter for smooth transitions between rates
	class SmoothParameter{
		SmoothParameter(){
			destination = param = 0.f;
			retuneSpeed = .0001f;
		}
		void setRetuneSpeed(float rts){ retuneSpeed = rts; }
		void setDestination(float dest){ destination = dest; }
		
		float process(){
			param = mix(param, destination, retuneSpeed);
			return param;
		}
		
		private float param, destination, retuneSpeed;
	}


	// feedback delay (used for delay, flanger and reverb)
	class FBDelayStereo{
		void resize(int s){
			delayL.resize(s);
			delayR.resize(s);
		}
		int size(){ return delayL.size(); }
		void setDelay(float dL, float dR){
			paramL.setDestination(dL);
			paramR.setDestination(dR);
		}
		void setFeedback(float fbL, float fbR){
			delayL.setFeedback(fbL);
			delayR.setFeedback(fbR);
		}
		
		double process(double sample, uint ch){
			switch(ch){
				case 0:
					delayL.setDelay(paramL.process());
					return delayL.process(sample);
				case 1:
					delayR.setDelay(paramR.process());
					return delayR.process(sample);
			}
			return 0.;
		}
		
		private FBDelay delayL, delayR;
		
		private SmoothParameter paramL, paramR;
	}

	class FBDelay{
		FBDelay(){
			wIdx = 0;
			setDelay(0.f);
			setFeedback(.8f);
		}
		void resize(int s){ buffer.length = s; }
		void setDelay(float d){ delay = d; }
		void setFeedback(float fb){ feedback = fb; }
		
		double process(double sample){
			inc();
			buffer[wIdx] = tanh(1.5 * buffer[wIdx] * feedback + sample); //the waveshaper is just used to give it some flavour, but actually not needed.
			return getSampleInterpolatedLinearly();
		}
		
		private double getSampleInterpolatedLinearly(){
			int rFloor = int(rIdx);
			int rCeil = (rFloor + 1) % buffer.length;
			float rFrac = rIdx - rFloor;
			
			return mix(buffer[rFloor], buffer[rCeil], rFrac);
		}

		private void inc(){
			++wIdx;
			wIdx %= size();
			rIdx = wIdx - delay;
			if(rIdx < 0.f)
				rIdx += size();
			rIdx %= size(); // yes, this line seems useless but it's an angelscript-thing!
		}
		
		int size(){ return buffer.length; }
		
		private array<double> buffer;
		private float delay, rIdx, feedback;
		private int wIdx;
	}


	// feedforward delay (used for predelay- and lookahead functionality)
	class FFDelayStereo{
		FFDelayStereo(int s = 2){ resize(s); }
		void resize(int s){
			delayL.resize(s);
			delayR.resize(s);
		}
		int size(){ return delayL.size(); }
		void setDelay(float dL, float dR){
			paramL.setDestination(dL);
			paramR.setDestination(dR);
		}
		
		double process(double sample, uint ch){
			switch(ch){
				case 0:
					delayL.setDelay(paramL.process());
					return delayL.process(sample);
				case 1:
					delayR.setDelay(paramR.process());
					return delayR.process(sample);
			}
			return 0.;
		}
		
		private FFDelay delayL, delayR;
		private SmoothParameter paramL, paramR;
	}

	class FFDelay{
		FFDelay(int s = 2){
			wIdx = 0;
			resize(s);
			setDelay(0.f);
		}
		void resize(int s){ buffer.length = s; }
		void setDelay(float d){ delay = d; }
		
		double process(double sample){
			inc();
			buffer[wIdx] = sample;
			return getSampleInterpolatedLinearly();
		}
		
		private double getSampleInterpolatedLinearly(){
			int rFloor = int(rIdx);
			int rCeil = (rFloor + 1) % buffer.length;
			float rFrac = rIdx - rFloor;
			
			return mix(buffer[rFloor], buffer[rCeil], rFrac);
		}

		private void inc(){
			++wIdx;
			wIdx %= buffer.length;
			rIdx = wIdx - delay;
			if(rIdx < 0)
				rIdx += size();
		}
		
		int size(){ return buffer.length; }
		
		private array<double> buffer;
		private float delay, rIdx;
		private int wIdx;
	}
}