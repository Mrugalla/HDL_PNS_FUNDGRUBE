namespace Utils{
	double mix(double& a, double& b, float& p){ return a + p * (b - a); }
}
namespace DSP{
	class Filter{
		void setCutoff(float c){ cutoff = 1.f - c; }
		
		void process(double& sample){
			envelope = Utils::mix(envelope, sample, cutoff);
			sample = envelope;
		}
		
		float cutoff;
		double envelope;
	}
}