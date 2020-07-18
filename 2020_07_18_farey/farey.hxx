namespace hdl {
	class Farey {
		Farey(int maxOrder){
			sequence.length = maxOrder;
			double x = 1. / 2.;
			sequence[0] = x;
			for(int i = 1; i < maxOrder; ++i){
				x /= 2.;
				sequence[i] = x;
			}
		}
		
		void setOrder(int o) { order = o; }
		
		double process(double sample) {
			double y = 0.;
			for(int o = 0; o < order; ++o)
				if(y < sample)
					y += sequence[o];
				else
					y -= sequence[o];
			return y;
		}
		
		private array<double> sequence;
		private int order;
	}
}