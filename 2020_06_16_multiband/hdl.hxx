namespace hdl{
	
	enum FilterType{ LP, HP }
	
	double mix(double a, double b, double c){ return a + c * (b - a); }
	
	class MBGain{
		void process(double& sampleLow, double& sampleHigh){
			gainLow.process(sampleLow);
			gainHigh.process(sampleHigh);
		}
		
		void setGainInDb(double x, double y){
			gainLow.setGainInDb(x);
			gainHigh.setGainInDb(y);
		}
		
		GainTool gainLow, gainHigh;
	}
	
	class GainTool{
		void process(double& sample){ sample *= gain; }
		
		double dbToAmp(double x){ return pow(10,x * twenInv); }
		double ampToDb(double x){ return 20. * log10(x); }
		
		void setGainInDb(double x){ gain = dbToAmp(x); }
		
		private double gain;
		private double twenInv = 1. / 20.;
	}
	
	class MBBitcrusher{
		void process(double& sampleLow, double& sampleHigh){
			crushLow.process(sampleLow);
			crushHigh.process(sampleHigh);
		}
		
		void setCrush(double cL, double cH){
			crushLow.setCrush(cL);
			crushHigh.setCrush(cH);
		}
		
		private Bitcrusher crushLow, crushHigh;
	}
	
	class Bitcrusher{
		void process(double& sample){
			double sampleFloor = int(crushFloor * sample) / crushFloor;
			double sampleCeil = int(crushCeil * sample) / crushCeil;
			sample = sampleFloor + crushFrac * (sampleCeil - sampleFloor);
		}
		void setCrush(double c){
			crush = 128. - 128. * tanh(2. * c);
			crushFloor = floor(crush);
			crushCeil = crushFloor + 1;
			crushFrac = crushCeil - crush;
		}
		
		private double crush, crushFloor, crushCeil, crushFrac;
	}
	
	class MBSaturation{
		void process(double& sampleLow, double& sampleHigh){
			satLow.process(sampleLow);
			satHigh.process(sampleHigh);
		}
		
		void setDrive(double driveLow, double driveHigh){
			satLow.drive = driveLow;
			satHigh.drive = driveHigh;
		}
		
		private Saturation satLow, satHigh;
	}
	
	class Saturation{
		void process(double& sample){ sample += drive * (getSat(sample) - sample); }
		private double getSat(double sample){ return tanh(7. * sample) * .25; }
		
		double drive;
	}
	
	class StereoFilter{
		double process(double sample, int ch){
			switch(ch){
				case 0: return filterL.process(sample);
				case 1: return filterR.process(sample);
			}
			return sample;
		}
		
		void setCutoff(double cutoff){
			cutoff = pow(cutoff, 2);
			
			filterL.cutoff = cutoff;
			filterR.cutoff = cutoff;
		}
		
		void setFilterType(int type){
			filterL.filterType = type;
			filterR.filterType = type;
		}
		
		private Filter filterL, filterR;
	}
	
	class Filter{
		double process(double sample){
			envelope += cutoff * (sample - envelope);
			switch(filterType){
				case FilterType::LP: return envelope;
				case FilterType::HP: return sample - envelope;
			}
			return sample;
		}
		private double envelope = 0;
		float cutoff = 0;
		int filterType = FilterType::LP;
	}
}