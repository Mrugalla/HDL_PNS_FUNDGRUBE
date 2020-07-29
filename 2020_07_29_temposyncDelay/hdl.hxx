namespace hdl {
	
	class RateHandler {
		bool process(double beatsPerMinute, double x, double y) {
			if(bpm != beatsPerMinute || x != xRate || y != yRate) {
				bpm = beatsPerMinute;
				xRate = x;
				yRate = y;
				double barsPerSec = bpm / 240.;
				double bar = sampleRate / barsPerSec;
				rate = bar * xRate / yRate;
				return true;
			}
			return false;
		}
		double getRate() { return rate; }
		
		double bpm, xRate, yRate, rate;
	}
	
	class FBDelay : FFDelay {
		FBDelay(int maxSize = int(sampleRate * 5)) {
			feedback = .6;
			super(maxSize);
		}
		
		void setFeedback(double f){ feedback = f; }
		
		double process(double x) {
			dIdx += retuneSpeed * (delay - dIdx);
			rIdx = wIdx - dIdx;
			if(rIdx < 0)
				rIdx += size();
			double y = x + getSample() * feedback;
			data[wIdx] = y;
			inc();
			return y;
		}
		
		double feedback;
	}
	
	class FFDelay {
		FFDelay(int maxSize = int(sampleRate * 5)) {
			data.length = maxSize;
			wIdx = 0;
			dIdx = rIdx = 0;
			retuneSpeed = 8. / sampleRate;
		}
		int size(){ return data.length; }
		void setDelayInSecounds(double d) { delay = clamp(d * sampleRate); }
		void setDelayInSamples(double d) { delay = clamp(d); }
		void setRetuneInSecounds(double r) { retuneSpeed = 1. / (sampleRate * r); }
		
		private double clamp(double d) {
			if(d < size()) return d;
			return size() - 1.;
		}
		
		double process(double x) {
			data[wIdx] = x;
			dIdx += retuneSpeed * (delay - dIdx);
			rIdx = wIdx - dIdx;
			if(rIdx < 0)
				rIdx += size();
			double y = getSample();
			inc();
			return y;
		}
		
		protected double getSample() { return data[int(rIdx)]; }
		protected void inc() {
			++wIdx;
			if(wIdx == size())
				wIdx = 0;
		}
		
		array<double> data;
		double delay, dIdx, rIdx, retuneSpeed;
		int wIdx;
	}
}