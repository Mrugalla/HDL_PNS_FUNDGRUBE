namespace hdl {
	double pi = 3.14159265359;
	double tau = pi * 2;
	
	class Wavetable {
		Wavetable(int s){
			data.length = s;
			makeSine();
		}
		int size() { return data.length; }
		void makeSine() {
			for(int i = 0; i < size(); ++i) {
				double x = double(i) / size();
				data[i] = sin(x * tau);
			}
		}
		
		array<double> data;
	}
	
	class Osc {
		void setFreq(double f){ inc = f * wt.size() * FsInv; }
		
		double process() {
			double wave = getSample();
			idx += inc;
			if(idx >= wt.size())
				idx -= wt.size();
			return wave;
		}
		
		double getSample(){ return wt.data[int(idx)]; }
		
		Wavetable wt(32);
		double FsInv = 1. / sampleRate;
		double idx, inc;
	}
	
	class OscLinearInterpolation : Osc {
		double getSample() {
			int iFloor = int(idx);
			int iCeil = iFloor + 1;
			if(iCeil >= wt.size())
				iCeil = 0;
			double iFrac = idx - iFloor;
			
			return wt.data[iFloor] + iFrac * (wt.data[iCeil] - wt.data[iFloor]);
		}
	}
	
	class OscSplineInterpolation : Osc {
		// https://www.desmos.com/calculator/iguotuanyf
		// https://youtu.be/9_aJGUTePYo?t=354 Catmull-Rom Spline
		OscSplineInterpolation(int s = 512) {
			splineSize = s;
			y1.length = splineSize;
			y2.length = splineSize;
			y3.length = splineSize;
			y4.length = splineSize;
			
			for(int i = 0; i < splineSize; ++i) {
				double x = double(i) / splineSize;
				
				y1[i] = (-pow(x, 3) + 2 * pow(x, 2) - x) / 2;
				y2[i] = (3 * pow(x, 3) - 5 * pow(x, 2) + 2) / 2;
				y3[i] = (-3 * pow(x, 3) + 4 * pow(x, 2) + x) / 2;
				y4[i] = (pow(x, 3) - pow(x, 2)) / 2;
			}
		}
		
		double getSample() {
			int i1 = int(idx) - 1;
			int i2 = i1 + 1;
			int i3 = i1 + 2;
			int i4 = i1 + 3;
			
			if(i1 < 0)
				i1 = wt.size() - 1;
			if(i3 >= wt.size())
				i3 = 0;
			if(i4 >= wt.size())
				i4 -= wt.size();
			
			double iFrac = idx - i2;
			
			double y = 0.;
			y += wt.data[i1] * y1[int(splineSize * iFrac)];
			y += wt.data[i2] * y2[int(splineSize * iFrac)];
			y += wt.data[i3] * y3[int(splineSize * iFrac)];
			y += wt.data[i4] * y4[int(splineSize * iFrac)];
			
			return y;
		}
		
		array<double> y1, y2, y3, y4;
		int splineSize;
	}
}