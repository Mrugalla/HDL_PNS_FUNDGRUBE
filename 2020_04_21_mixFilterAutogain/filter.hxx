namespace Utils{
	double mix(double& a, double& b, float& p){ return a + p * (b - a); }
	double mix(double& a, double b, double p){ return a + p * (b - a); }
}
namespace DSP{
	enum FilterTypes{ LP = 0, HP = 1 }
	
	class Filter{
		Filter(){
			for(uint ch = 0; ch < envelope.length; ++ch)
				envelope[ch].length = 10; // max order
		}
		Filter(int t, int o, float c){
			for(uint ch = 0; ch < envelope.length; ++ch)
				envelope[ch].length = 10; // max order
			setParameters(t, o, c);
		}
		void setParameters(int t, int o, float c){
			setType(t);
			setOrder(o);
			setCutoff(c);
		}
		private void setType(int t){ type = t; }
		private void setOrder(int o){ order = o; }
		private void setCutoff(float c){
			if(type == FilterTypes::LP)
				cutoff = 1.f - c;
			else
				cutoff = c;
			}
		
		void process(double& sample, uint ch){
			if(type == FilterTypes::LP && cutoff == 0.f){
				int maxOrder = order - 1;
				envelope[ch][maxOrder] = Utils::mix(envelope[ch][maxOrder], 0., .001);
				sample = envelope[ch][maxOrder];
			}
			else
				processFilter(sample, ch);
		}
		
		private void processFilter(double& sample, uint ch){
			envelope[ch][0] = Utils::mix(envelope[ch][0], sample, cutoff);
				for(int o = 1; o < order; ++o)
					envelope[ch][o] = Utils::mix(
						envelope[ch][o],
						envelope[ch][o - 1],
						cutoff);
						
				if(type == FilterTypes::LP)
					sample = envelope[ch][order - 1];
				else
					sample -= envelope[ch][order - 1];
		}
		
		int type, order;
		float cutoff;
		array<array<double>> envelope(2);
	}
}