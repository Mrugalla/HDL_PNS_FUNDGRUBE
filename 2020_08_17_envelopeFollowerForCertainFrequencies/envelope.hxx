namespace hdl {
	double pi = 3.14159265359;
	double tau = pi * 2.;
	
	namespace utils {
		double note2freq(double note, double xen = 12., double masterTune = 440.) {
			return pow(2., (note - 69.) / xen) * masterTune;
		}
		double freq2samples(double freq) { return sampleRate / freq; }
		double note2samples(double note, double xen = 12., double masterTune = 440.) {
			return freq2samples(note2freq(note, xen, masterTune));
		}
		double maximum(array<double>& data) {
			double max = 0;
			for(uint i = 0; i < data.length; ++i) {
				double x = abs(data[i]);
				if(max < x) {
					max = x;
				}
			}
			return max;
		}
		double maximum(array<double>& data, uint size) {
			double max = 0;
			for(uint i = 0; i < size; ++i) {
				double x = abs(data[i]);
				if(max < x)
					max = x;
			}
			return max;
		}
		double clamp(double x, double upperLimit, double lowerLimit = 0.) {
			if(x >= upperLimit) return upperLimit - 1.;
			else if(x < lowerLimit) return lowerLimit;
			else return x;
		}
	}
	
	class Range {
		Range(double minimum = 0, double maximum = 0) {
			min = minimum;
			max = maximum;
		}
		
		hdl::Range@ opAssign(const hdl::Range &in other) {
			min = other.min;
			max = other.min;
			return this;
		}	
		
		double min, max;
	}
	
	class EnvelopeFollower {
		uint size() { return fifo.length; }
		
		void setSpectralRange(Range range) {
			spectralRange = range;
			fifo.length = int(utils::freq2samples(range.min));
			setReloadFrequency(range.min);
		}
		
		void setReloadFrequency(double f) {
			length = uint(utils::freq2samples(f));
		}
		
		double process(double x) {
			if (idx >= length) {
				idx = 0;
				double max = utils::maximum(fifo, length);
				envelope = max;
			}
			fifo[idx] = x;
			++idx;
			
			return envelope;
		}
		
		array<double> fifo;
		Range spectralRange;
		double envelope = 0;
		uint idx = 0;
		uint length;
	}
	
	class EFLinear : EnvelopeFollower {
		void setReloadFrequency(double f) {
			EnvelopeFollower::setReloadFrequency(f);
			lengthInv = 1. / length;
		}
		
		double process(double x) {
			if (idx >= length) {
				idx = 0;
				double max = utils::maximum(fifo, length);
				inc = (max - envelope) * lengthInv;
			}
			fifo[idx] = x;
			++idx;
			
			envelope += inc;
			return envelope;
		}
		
		double inc = 0, lengthInv = 1;
	}
	
	class SplineInterpolator {
		SplineInterpolator(int lookupTableSize) {
			size = lookupTableSize;
			y1.length = size;
			y2.length = size;
			y3.length = size;
			y4.length = size;
			double sizeInv = 1. / size;
			for(int i = 0; i < size; ++i) {
				double x = i * sizeInv;
				y1[i] = (-pow(x, 3) + 2 * pow(x, 2) - x) / 2;
				y2[i] = (3 * pow(x, 3) - 5 * pow(x, 2) + 2) / 2;
				y3[i] = (-3 * pow(x, 3) + 4 * pow(x, 2) + x) / 2;
				y4[i] = (pow(x, 3) - pow(x, 2)) / 2;
			}
		}
		
		void writeSampleToBuffer(double x) {
			++wIdx;
			if(wIdx >= bufferLength) wIdx = 0;
			buffer[wIdx] = x;
		}
		
		double process(double phase) {
			int i4 = wIdx;
			int i3 = i4 - 1;
			int i2 = i4 - 2;
			int i1 = i4 - 3;
			
			if (i1 < 0) {
				i1 += bufferLength;
				if (i2 < 0) {
					i2 += bufferLength;
					if(i3 < 0)
						i3 += bufferLength;
				}
			}
			
			double y = 0.;
			int rIdx = int(utils::clamp(size * phase, size));
			y += buffer[i1] * y1[rIdx];
			y += buffer[i2] * y2[rIdx];
			y += buffer[i3] * y3[rIdx];
			y += buffer[i4] * y4[rIdx];
			return y;
		}
		
		array<double> y1, y2, y3, y4;
		array<double> buffer(5);
		int wIdx = 0, size, bufferLength = 4;
	}
	
	class EFSpline : EnvelopeFollower {
		void setReloadFrequency(double f) {
			EnvelopeFollower::setReloadFrequency(f);
			inc = 1. / length;
		}
		
		double process(double x) {
			if (idx >= length) {
				idx = 0;
				double max = utils::maximum(fifo, length);
				spline.writeSampleToBuffer(max);
				phase = 0.;
			}
			fifo[idx] = x;
			++idx;
			
			envelope = spline.process(phase);
			phase += inc;
			
			return envelope;
		}
		
		SplineInterpolator spline(1024);
		double phase = 0., inc = 0.;
	}
}