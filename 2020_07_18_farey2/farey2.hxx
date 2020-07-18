/*

this version does not attempt to contain the original farey sequence, but a modified version that is more suitable for audio purposes as it gives the user more parameters.

*/

namespace hdl {
	class Farey2 {
		
		void setMaxStep(double s){ maxStep = s; }
		void setStepDrop(double d){ stepDrop = d; }
		void setOrder(int o) { order = o; }
		
		double process(double sample) {
			double y = 0.;
			double step = maxStep;
			for(int o = 0; o < order; ++o) {
				if(y < sample)
					y += step;
				else
					y -= step;
				step *= stepDrop;
			}
			return y;
		}
		
		private double maxStep, stepDrop;
		private int order;
	}
}