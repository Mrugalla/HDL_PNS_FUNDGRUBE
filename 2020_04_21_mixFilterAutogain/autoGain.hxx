namespace DSP{
	class AutoGainModule{
		array<array<array<double>>> autogainFactor; // type, order, cutoff
		double curValue; // current value from autogainFactor
		int cutoffGrains = 3; // sets amount of granularity in cutoff-autogain.
		
		// parameters from the filter
		int type, order;
		float cutoff;
		
		/*
		*	in the constructor we prepare everything that is needed
		*	in order to get the autogainFactors for each filter setting.
		*/
		AutoGainModule(){
			array<double> buffer = getNoiseBuffer();
			double size = double(buffer.length);
			
			calculateAutoGainFactorForFilter(2, 10, cutoffGrains, buffer, size);
		}
		
		/*
		*	when parameters are set from main.cxx
		*	curValue is being calculated from the
		*	autogainFactor-array.
		*/
		void setParameters(int t, int o, float c){
			if(c == 1.f || c == 0.f) // we don't need a value from the array if the filter is
				curValue = 1.f; // off or filtering everything.
			else if(type != t || order != o || cutoff != c){
				type = t;
				order = o;
				cutoff = c;
				
				int orderS = order - 1;
				// we perform linear interpolation on the cutoff index because it's float:
				float cutoffS = cutoff * (cutoffGrains - 1);
				int cutoffF = int(cutoffS);
				int cutoffC = (cutoffF + 1) % cutoffGrains;
				float cutoffFrac = cutoffS - cutoffF;
				
				curValue = Utils::mix(
					autogainFactor[type][orderS][cutoffF],
					autogainFactor[type][orderS][cutoffC],
					cutoffFrac);
			}
		}
		
		/*
		*	when we process the input samples everything else has been settled already.
		*	maximum performance!
		*/
		void process(double& sample){ sample *= curValue; }
		
		private void calculateAutoGainFactorForFilter(int types, int orders, int cutoffs, array<double>& buffer, double& size){
			prepareAutogainFactor(types, orders, cutoffs);
			double rms = getRMSOf(buffer, size);
			
			/*
			*	calculate rms ratio for each filter setting.
			*	the rms ratio between 2 signals determines the factor by which
			*	a signal has to be multiplicated to reach a similiar gain level.
			*/
			for(uint t = 0; t < autogainFactor.length; ++t)
				for(uint o = 0; o < autogainFactor[t].length; ++o)
					for(uint c = 0; c < autogainFactor[t][o].length; ++c){
						double cutoff = double(c) / double(autogainFactor[t][o].length);
						autogainFactor[t][o][c] = rms / getRMSFor(cutoff, o + 1, t, buffer, size);
					}
		}
		
		private void prepareAutogainFactor(int types, int orders, int cutoffs){
			/*
			*	this method prepares the multidimensional array
			*	to have the correct size.
			*/
			autogainFactor.length = types;
			for(uint type = 0; type < autogainFactor.length; ++type){
				autogainFactor[type].length = orders;
				for(uint order = 0; order < autogainFactor[type].length; ++order)
					autogainFactor[type][order].length = cutoffs;
			}
		}
		
		private double getAutogainFactor(double rms, double rmsFilter){ return rms / rmsFilter; }
		
		private double getRMSFor(float cutoff, int order, int type, array<double> buffer, double& size){
			/*
			*	we apply a filter with certain settings on the buffer and get
			*	the resulting rms value.
			*/
			Filter filter(type, order, cutoff);
			double rms = getRMSOf(buffer, size, filter);
			return rms;
		}
		
		private double getRMSOf(array<double>& buffer, double& size){
			double rms = 0.;
			for(int i = 0; i < size; ++i)
				rms += pow(buffer[i], 2);
			rms /= size;
			return sqrt(rms);
		}
		
		private double getRMSOf(array<double> buffer, double& size, Filter& filter){
			/*
			*	we get the RMS-value of the original buffer (dry)
			*/
			double rms = 0.;
			for(int i = 0; i < size; ++i){
				filter.process(buffer[i], 0);
				rms += pow(buffer[i], 2);
			}	
			rms /= size;
			return sqrt(rms);
		}
		
		private array<double> getNoiseBuffer(){
			/*
			*	we import the wav-sample with the pink noise.
			*	notice that this is a 44.1khz sample. if you planned
			*	to make your filter work for more sampleRates
			*	you'd have to recalculate everything with a different sample
			*	if the sampleRate has changed. but i wanted
			*	to keep things simple here.
			*/
			string path = scriptFilePath;
			WaveFileData waveFileData;
			path = path.substr(0, path.findFirst("main.cxx", 0)) + "PinkNoise44100.wav";
			convertToUnix(path);
			if(waveFileData.loadFile(path))
				return waveFileData.interleavedSamples;
			return array<double>();
		}
		
		bool convertToUnix(string& path){
			/*
			*	a helper method for some edge cases of the path.
			*	don't ask me why xD
			*/
			bool isUNC = (path.findFirst("\\\\")==0);
			if(!isUNC){
				int index = path.findFirst("\\");
				while(index >= 0){
					path[index] = '/';
					index = path.findFirst("\\",index+1);
				}
			}
		return !isUNC;
}
	}
}