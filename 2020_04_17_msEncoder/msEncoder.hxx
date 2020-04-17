namespace MSProcessor{
	void encode(BlockData& data){
		for(uint s = 0; s < data.samplesToProcess; ++s){
			double T = data.samples[0][s];
			data.samples[0][s] = (data.samples[0][s] + data.samples[1][s]) * .5f;
			data.samples[1][s] = (T - data.samples[1][s]) * .5f;
		}
	}
	void decode(BlockData& data){
		for(uint s = 0; s < data.samplesToProcess; ++s){
			double T = data.samples[0][s];
			data.samples[0][s] = data.samples[0][s] + data.samples[1][s];
			data.samples[1][s] = T - data.samples[1][s];
		}
	}
}
namespace Utils{
	float mix(float& a, float& b, float& p){ return a + p * (b - a); }
}