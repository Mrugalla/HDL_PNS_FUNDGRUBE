namespace convolution {
	class ImpulseResponse {
		ImpulseResponse(array<double> ir) { data = ir; size = ir.length; }
		array<double> data;
		int size;
	}
	
	class Processor {
		Processor() {}
		Processor(int irSize) {
			buffer.length = irSize;
			wIdx = 0;
			size = irSize;
		}
		void process(array<double>& data, ImpulseResponse& ir) {
			for(uint s = 0; s < data.length; ++s) {
				++wIdx;
				if(wIdx == size) wIdx = 0;
				buffer[wIdx] = data[s];
				int rIdx = wIdx;
				double y = buffer[rIdx] * ir.data[0];
				for(int i = 1; i < size; ++i) {
					--rIdx;
					if(rIdx == -1) rIdx = size - 1;
					y += buffer[rIdx] * ir.data[i];
				}
				data[s] = y;
			}
		}
		
		array<double> buffer;
		int wIdx, size;
	}
	
	namespace IRTest {
		// a namesapce that returns specific IRs made with https://www.earlevel.com/main/2010/12/05/building-a-windowed-sinc-filter/
		// i wanna learn how this page's algorithm works at some point too but for now i just wanna find out what it can do.
		
		array<double> make0() {
			// factor = .5, length = 199, rejection = 90, gain = 1
			return {-0.000003055192744486332,0,0.0000061701090803117584,0,-0.000010643939298569722,0,0.00001682462081495755,0,-0.000025113634674665028,0,0.00003596987707798252,0,-0.000049913399161288374,0,0.00006752899380647781,0,-0.0000894696196541565,0,0.00011645966633212502,0,-0.00014929808224601864,0,0.00018886140904096721,0,-0.00023610679134593367,0,0.00029207506293415445,0,-0.0003578940497195854,0,0.0004347822739094624,0,-0.0005240533023573434,0,0.0006271210504813982,0,-0.0007455064368163751,0,0.0008808458968472592,0,-0.001034902391408623,0,0.00120957972825272,0,-0.0014069412381212442,0,0.0016292341481756144,0,-0.0018789213967819803,0,0.0021587231796921006,0,-0.0024716712540583633,0,0.0028211801034585546,0,-0.003211140492392964,0,0.0036460431095941743,0,-0.004131143071956945,0,0.0046726806762556385,0,-0.0052781807604445705,0,0.005956863818275,0,-0.006720219055958438,0,0.007582817208415843,0,-0.008563486986580616,0,0.009687058247814974,0,-0.010987016219297286,0,0.012509673341860275,0,-0.014320976267770022,0,0.016518116943726336,0,-0.019250430650365736,0,0.02275958098750915,0,-0.027463577393639915,0,0.03415280746865259,0,-0.04452211172185513,0,0.06297981704464543,0,-0.1056928660303178,0,0.3181733751122296,0.5000008986001705,0.3181733751122296,0,-0.1056928660303178,0,0.06297981704464543,0,-0.04452211172185513,0,0.03415280746865259,0,-0.027463577393639915,0,0.02275958098750915,0,-0.019250430650365736,0,0.016518116943726336,0,-0.014320976267770022,0,0.012509673341860275,0,-0.010987016219297286,0,0.009687058247814974,0,-0.008563486986580616,0,0.007582817208415843,0,-0.006720219055958438,0,0.005956863818275,0,-0.0052781807604445705,0,0.0046726806762556385,0,-0.004131143071956945,0,0.0036460431095941743,0,-0.003211140492392964,0,0.0028211801034585546,0,-0.0024716712540583633,0,0.0021587231796921006,0,-0.0018789213967819803,0,0.0016292341481756144,0,-0.0014069412381212442,0,0.00120957972825272,0,-0.001034902391408623,0,0.0008808458968472592,0,-0.0007455064368163751,0,0.0006271210504813982,0,-0.0005240533023573434,0,0.0004347822739094624,0,-0.0003578940497195854,0,0.00029207506293415445,0,-0.00023610679134593367,0,0.00018886140904096721,0,-0.00014929808224601864,0,0.00011645966633212502,0,-0.0000894696196541565,0,0.00006752899380647781,0,-0.000049913399161288374,0,0.00003596987707798252,0,-0.000025113634674665028,0,0.00001682462081495755,0,-0.000010643939298569722,0,0.0000061701090803117584,0,-0.000003055192744486332};
		}
		
		array<double> make1() {
			// factor = .2, length = 199, rejection = 90, gain = 1
			return {-0.0000017958013329629392,-0.000004244146293789394,-0.000005868135827791082,-0.000004829620627224069,0,0.000007933433203193437,0.000016001201746706175,0.000019666272418457612,0.000014761457752771285,0,-0.000021142611483261753,-0.000040439716137378155,-0.000047470571766681586,-0.00003422270709081759,0,0.00004579745989325527,0.00008509085880939846,0.00009727933667975703,0.00006845343044659998,0,-0.00008775541104241866,-0.00015995022407666937,-0.00017961828331638003,-0.00012429381710443675,0,0.0001545479964404763,0.0002777805252547731,0.00030783794483799884,0.00021036532399005902,0,-0.0002555591912963088,-0.00045440061331751346,-0.0004984054445727851,-0.00033725438732877256,0,0.000402222120842694,0.0007090203714048427,0.0007712608858983274,0.0005177494082977136,0,-0.0006083017503010663,-0.0010647468033007158,-0.0011503813056690617,-0.0007672314627030939,0,0.0008903925455355188,0.0015494972864056608,0.0016648187772294748,0.001104404805543951,0,-0.0012688685421190658,-0.002197754405412634,-0.0023507044124620813,-0.0015527116704284528,0,0.001769730764168804,0.00305398305381819,0.003255155997304318,0.002143095255795543,0,-0.0024282305097344335,-0.004179352395182462,-0.004443993539053824,-0.0029194799546829464,0,0.003296141237604943,0.00566532706931101,0.006017472498842669,0.003950054660306627,0,-0.0044570782891071295,-0.007662686933554121,-0.008144378671865767,-0.005352125763130234,0,0.006061524638772063,0.010449297196820782,0.011143508052010462,0.007353018267926311,0,-0.008417677842880654,-0.014611633534411483,-0.015709698578007282,-0.010465607322941285,0,0.012277325204595438,0.021645697163520296,0.02370531755706955,0.016142722576320448,0,-0.020074562329109397,-0.03681098927798099,-0.042343141024458915,-0.030702876454896947,0,0.04645327647610439,0.10052011817319324,0.1511053644182286,0.18701804400754127,0.20000081548824558,0.18701804400754127,0.1511053644182286,0.10052011817319324,0.04645327647610439,0,-0.030702876454896947,-0.042343141024458915,-0.03681098927798099,-0.020074562329109397,0,0.016142722576320448,0.02370531755706955,0.021645697163520296,0.012277325204595438,0,-0.010465607322941285,-0.015709698578007282,-0.014611633534411483,-0.008417677842880654,0,0.007353018267926311,0.011143508052010462,0.010449297196820782,0.006061524638772063,0,-0.005352125763130234,-0.008144378671865767,-0.007662686933554121,-0.0044570782891071295,0,0.003950054660306627,0.006017472498842669,0.00566532706931101,0.003296141237604943,0,-0.0029194799546829464,-0.004443993539053824,-0.004179352395182462,-0.0024282305097344335,0,0.002143095255795543,0.003255155997304318,0.00305398305381819,0.001769730764168804,0,-0.0015527116704284528,-0.0023507044124620813,-0.002197754405412634,-0.0012688685421190658,0,0.001104404805543951,0.0016648187772294748,0.0015494972864056608,0.0008903925455355188,0,-0.0007672314627030939,-0.0011503813056690617,-0.0010647468033007158,-0.0006083017503010663,0,0.0005177494082977136,0.0007712608858983274,0.0007090203714048427,0.000402222120842694,0,-0.00033725438732877256,-0.0004984054445727851,-0.00045440061331751346,-0.0002555591912963088,0,0.00021036532399005902,0.00030783794483799884,0.0002777805252547731,0.0001545479964404763,0,-0.00012429381710443675,-0.00017961828331638003,-0.00015995022407666937,-0.00008775541104241866,0,0.00006845343044659998,0.00009727933667975703,0.00008509085880939846,0.00004579745989325527,0,-0.00003422270709081759,-0.000047470571766681586,-0.000040439716137378155,-0.000021142611483261753,0,0.000014761457752771285,0.000019666272418457612,0.000016001201746706175,0.000007933433203193437,0,-0.000004829620627224069,-0.000005868135827791082,-0.000004244146293789394,-0.0000017958013329629392};
		}
		
		array<double> make2() {
			// factor = .5, length = 99, rejection = 90, gain = 1
			return { 0.00001234543596327421,0,-0.00004344568526742124,0,0.00010305331559244446,0,-0.0002055964523723566,0,0.00036962573694295365,0,-0.0006182899507672315,0,0.000979772007210882,0,-0.0014877060996681165,0,0.002181627673144961,0,-0.0031075534884872163,0,0.00431885727620488,0,-0.00587771220221287,0,0.007857542930432825,0,-0.010347224247287477,0,0.01345829709701244,0,-0.01733750028099672,0,0.022189021401160518,0,-0.02831549545865312,0,0.036197807124857956,0,-0.046662832680990834,0,0.06127551587389479,0,-0.08340334512301664,0,0.12183505280220708,0,-0.20887175107240838,0,0.6355005242483182,0.9999988196383697,0.6355005242483182,0,-0.20887175107240838,0,0.12183505280220708,0,-0.08340334512301664,0,0.06127551587389479,0,-0.046662832680990834,0,0.036197807124857956,0,-0.02831549545865312,0,0.022189021401160518,0,-0.01733750028099672,0,0.01345829709701244,0,-0.010347224247287477,0,0.007857542930432825,0,-0.00587771220221287,0,0.00431885727620488,0,-0.0031075534884872163,0,0.002181627673144961,0,-0.0014877060996681165,0,0.000979772007210882,0,-0.0006182899507672315,0,0.00036962573694295365,0,-0.0002055964523723566,0,0.00010305331559244446,0,-0.00004344568526742124,0,0.00001234543596327421 };
		}
		
		array<double> make3() {
			// factor = .448, length = 25, rejection = 22, gain = 2
			// goal for small length (upsample)
			return {-0.04448880857299171,0.011973607781237787,0.05954715415701679,0.006748659436729747,-0.07395847241830694,-0.03668498597983041,0.08661266131977638,0.0863521628992693,-0.09649054509896655,-0.18766916549603102,0.10277254110628156,0.6332885835399858,0.903993214651659,0.6332885835399858,0.10277254110628156,-0.18766916549603102,-0.09649054509896655,0.0863521628992693,0.08661266131977638,-0.03668498597983041,-0.07395847241830694,0.006748659436729747,0.05954715415701679,0.011973607781237787,-0.04448880857299171};
		}
		
		array<double> make4() {
			// factor = .448, length = 25, rejection = 22, gain = 1
			// goal for small length (downsample)
			return {-0.022244404286495854,0.0059868038906188935,0.029773577078508397,0.0033743297183648734,-0.03697923620915347,-0.018342492989915204,0.04330633065988819,0.04317608144963465,-0.04824527254948328,-0.09383458274801551,0.05138627055314078,0.3166442917699929,0.4519966073258295,0.3166442917699929,0.05138627055314078,-0.09383458274801551,-0.04824527254948328,0.04317608144963465,0.04330633065988819,-0.018342492989915204,-0.03697923620915347,0.0033743297183648734,0.029773577078508397,0.0059868038906188935,-0.022244404286495854};
		}
	}
}