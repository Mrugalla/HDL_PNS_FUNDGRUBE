namespace hdl {
	class Farey {
		void setOrder(int o) {
			order = o;
			sequence.length = o;
			for(int s = 0; s < order; ++s)
				sequence[s] = 1. / double(s + 2);
		}
		
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