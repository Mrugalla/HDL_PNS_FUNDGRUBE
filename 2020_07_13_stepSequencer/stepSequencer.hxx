namespace hdl {
	
	class StepSequencer {
		StepSequencer(){ minuteInv = 1. / (60. * sampleRate); }
		
		void setRate(int r){ rate = pow(2., r); }
		
		void setTransportInfo(const TransportInfo& transport){
			isPlaying = transport.get_isPlaying();
			double bpm = transport.get_bpm();
			double position = transport.get_positionInQuarterNotes();
			double speed = bpm * minuteInv;
			
			phase = rate * position;
			phase -= int(phase);
			inc = rate * speed;
		}
		
		bool process() {
			if(!isPlaying)
				return false;
			bool trigger = false;
			if(phase >= 1.) {
				phase -= 1.;
				trigger = true;
			}
			phase += inc;
			return trigger;
		}
		
		double getPhase(){
			double wave = phase;
			phase += inc;
			if(phase >= 1.)
				phase -= 1.;
			return wave;
		}
		
		double minuteInv, rate, inc, phase;
		bool isPlaying;
	}
	
}