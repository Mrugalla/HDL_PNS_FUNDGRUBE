namespace hdl{

	class WaveShaper {
		// https://www.desmos.com/calculator/8vohoeljpg
		void setShape(double s) {
			shape = abs(s);
			positive = s >= 0;
		}
		
		double process(double phase) {
			if(!positive)
				return phase + shape * (pow(phase, 4.) - phase);
			return phase + shape * ((1. - pow(phase - 1., 4.)) - phase);
		}
		
		double shape;
		bool positive;
	}
	
	enum EnvelopeState{ Attack, Release }

	class EnvelopeGenerator {
		void setDecayInSeconds(double d){ decay = 1. / (d * sampleRate); }
		void setDecayInMS(double d){ decay = 1000. / (d * sampleRate); }
		void setAttackInMS(double a){ attack = 1000. / (a * sampleRate); }
		void setShape(double s){ shaper.setShape(s); }
		
		double process(bool trigger){
			if(trigger){
				state = EnvelopeState::Attack;
			}
			else {
				if(state == EnvelopeState::Attack)
					if(phase < 1.)
						phase += attack;
					else {
						phase = 1.;
						state = EnvelopeState::Release;
					}
				else
					if(phase > 0.)
						phase -= decay;
					else
						phase = 0.;
			}
			return shaper.process(phase);
		}
		
		WaveShaper shaper;
		double attack = 0., decay = 0., phase = 0., shape = -2.;
		EnvelopeState state = EnvelopeState::Release;
	}
}