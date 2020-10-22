namespace Utils {
	double mix(double a, double b, float p){ return a + p * (b - a); }
}
namespace DSP {
	class Filter {
		void setCutoff(double c) { cutoff = 1 - c; }
		
		void process(double& sample) {
			envelope = Utils::mix(envelope, sample, cutoff);
			sample = envelope;
		}
		
		float cutoff, envelope;
	}
}