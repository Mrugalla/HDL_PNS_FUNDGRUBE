namespace hdl {
	double pi = 3.14159265359;
	double tau = pi * 2;
	
	string dbg(array<int>& arg) {
		string s = "" + arg[0];
		for(uint i = 1; i < arg.length; ++i)
			s += ", " + arg[i];
		return s;
	}
	string dbg(array<double>& arg) {
		string s = "" + arg[0];
		for(uint i = 1; i < arg.length; ++i)
			s += ", " + arg[i];
		return s;
	}
	int limit(int v, int up, int low = 0) {
		while(v < low) v += up;
		while(v >= up) v -= up;
		return v;
	}
	
	class Wavetable {
		Wavetable() { }
		void resize(int s) {
			size = s;
			max = size - 1;
			data.length = s + 1;
			sizeTimesFsInv = double(size) / sampleRate;
		}
		void makeSine() {
			for(uint i = 0; i < data.length; ++i)
				data[i] = sin(tau * i / size);
		}
		void setWT(array<double>& d) {
			resize(d.length);
			data = d;
		}
		
		array<double> data;
		double sizeTimesFsInv;
		int size, max;
	}
	
	class Osc {
		Osc(int s) {
			wt.resize(s);
			wt.makeSine();
		}
		void setFreq(double f) { inc = f * wt.sizeTimesFsInv; }
		void setWT(array<double>& d) { wt.setWT(d); }
		
		double process() { incIdx(); return getSample(); }
		double getSample() { return wt.data[int(idx)]; }
		
		Wavetable wt;
		double idx, inc;
		
		private void incIdx() {
			idx += inc;
			if(idx >= wt.size)
				idx -= wt.size;
		}
	}
	
	class OscLinearInterpolation : Osc {
		OscLinearInterpolation(int s) { super(s); }
		
		double getSample() {
			int iFloor = int(idx);
			int iCeil = iFloor + 1;
			if(iCeil > wt.max)
				iCeil = 0;
			double iFrac = idx - iFloor;
			
			return wt.data[iFloor] + iFrac * (wt.data[iCeil] - wt.data[iFloor]);
		}
	}
	
	class OscSplineInterpolation : Osc {
		// https://www.desmos.com/calculator/iguotuanyf
		// https://youtu.be/9_aJGUTePYo?t=354 Catmull-Rom Spline
		OscSplineInterpolation(int wtSize, int spline) {
			super(32);
			splineSize = spline;
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
			int i1 = int(idx);
			int i0 = i1 - 1;
			int i2 = i1 + 1;
			int i3 = i1 + 2;
			
			if(i0 < 0)
				i0 = wt.max;
			if(i2 > wt.max)
				i2 = 0;
			if(i3 > wt.max)
				i3 -= wt.size;
			
			double mix = idx - i1;
			int splIdx = int(mix * splineSize);
			
			double y = 0;
			y += wt.data[i0] * y1[splIdx];
			y += wt.data[i1] * y2[splIdx];
			y += wt.data[i2] * y3[splIdx];
			y += wt.data[i3] * y4[splIdx];
			
			return y;
		}
		
		array<double> y1, y2, y3, y4;
		int splineSize;
	}
	
	class OscLagRangeInterpolationSimple : Osc {
		OscLagRangeInterpolationSimple(int s, int o) {
			super(s);
			order = o;
			y.length = o;
			x.length = o;
		}
		
		double getSample() {
			loadX();
			loadY();
			
			double yp = 0;
			for(int i = 0; i < order; ++i) {
				double p = y[i];
				for(int j = 0; j < order; ++j)
					if(j != i)
						p *= (idx - (x[0] + j)) / (i - j);
				yp += p;
			}
			return yp;
		}
		
		int order;
		array<double> y(order);
		array<int> x(order);
		
		private void loadX() {
			x[0] = int(idx);
			for(int i = 1; i < order; ++i)
				x[i] = x[0] + i;
		}
		private void loadY() { for(int i = 0; i < order; ++i) y[i] = wt.data[limit(x[i], wt.size)]; }
	}

	class OscLagRangeInterpolation : Osc {
		OscLagRangeInterpolation(int s, int o) {
			super(s);
			order = o;
			subIJInv.length = order;
			for(int i = 0; i < order; ++i) {
				subIJInv[i].length = order;
				for(int j = 0; j < order; ++j)
					subIJInv[i][j] = 1. / double(i - j);
			}
			addFloorJ.length = wt.size;
			for(int w = 0; w < wt.size; ++w) {
				addFloorJ[w].length = order;
				for(int j = 0; j < order; ++j)
					addFloorJ[w][j] = w + j;
			}
			iX.length = wt.size;
			for(int w = 0; w < wt.size; ++w) {
				iX[w].length = order;
				for(int i = 0; i < order; ++i) {
					iX[w][i] = w + i;
					if(iX[w][i] > wt.max)
						iX[w][i] -= wt.size;
				}
			}
		}
		
		double getSample() {
			int iFloor = int(idx);
			double p = wt.data[iFloor];
			for(int j = 1; j < order; ++j)
				p *= (idx - addFloorJ[iFloor][j]) * subIJInv[0][j];
			double yp = p;
			for(int i = 1; i < order; ++i) {
				p = wt.data[iX[iFloor][i]];
				for(int j = 0; j < order; ++j)
					if(j != i)
						p *= (idx - addFloorJ[iFloor][j]) * subIJInv[i][j];
				yp += p;
			}
			return yp;
		}
		
		array<array<double>> subIJInv;
		array<array<int>> addFloorJ, iX;
		int order;
	}
	
}