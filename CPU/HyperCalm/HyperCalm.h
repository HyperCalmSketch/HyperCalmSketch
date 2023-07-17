#ifndef _HYPERCALM_H_
#define _HYPERCALM_H_

#include "CalmSpaceSaving.h"
#include "HyperBloomFilter.h"

class HyperCalm {
private:
	CalmSpaceSaving css;
	HyperBloomFilter hbf;

public:
#define memory2 min(memory / 2, 50000)
#define sz max(1, memory / 1000)
	HyperCalm(double time_threshold, double unit_time,
		int memory, int seed): css(time_threshold, unit_time, memory - memory2, 3, sz, sz), hbf(memory2, time_threshold, seed) {
	}
#undef sz
#undef memory2
	void insert(int key, double time) {
		bool b = hbf.insert(key, time);
		css.insert(key, time, b);
	}
	vector<pair<pair<int, int16_t>, int>> get_top_k(int k) {
		return css.get_top_k(k);
	}
};


#endif // _HYPERCALM_H_