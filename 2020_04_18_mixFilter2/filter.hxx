namespace Utils{
	double mix(double& a, double& b, float& p){ return a + p * (b - a); }
}
namespace DSP{
	enum FilterTypes{
		LP = 0, HP = 1
	}
	
	class Filter{
		Filter(){
			for(uint ch = 0; ch < envelope.length; ++ch)
				envelope[ch].length = 14; // max order
		}
		void setCutoff(float c){
			if(type == FilterTypes::LP)
				cutoff = 1.f - c;
			else
				cutoff = c;
			}
		void setOrder(int o){ order = o; }
		void setType(int t){ type = t; }
		
		void process(double& sample, uint ch){
			if(cutoff == 0)
				sample = 0.f;
			else{
				envelope[ch][0] = Utils::mix(envelope[ch][0], sample, cutoff);
				for(int o = 1; o < order; ++o)
					envelope[ch][o] = Utils::mix(envelope[ch][o], envelope[ch][o - 1], cutoff);
			
				if(type == FilterTypes::LP)
					sample = envelope[ch][order - 1];
				else
					sample -= envelope[ch][order - 1];
			}
		}
		
		float cutoff;
		int order, type;
		array<array<double>> envelope(2);
	}
}