#ifndef _CLOCKUSS_H_
#define _CLOCKUSS_H_

#include "UnbiasedSpaceSaving.h"
#include "clockSketch.h"

class ClockUSS {
private:
	UnbiasedSpaceSaving uss;
	clockSketch cs;

public:
#define memory2 min(memory / 2, 50000)
#define sz max(1, memory / 1000)
	ClockUSS(double time_threshold, double unit_time,
		int memory, int seed): uss(time_threshold, unit_time, memory - memory2, sz, seed), cs(memory2, time_threshold, seed) {
	}
#undef sz
#undef memory2
	void insert(int key, double time) {
		bool b = cs.insert(key, time);
		uss.insert(key, time, b);
	}
	vector<pair<pair<int, int16_t>, int>> get_top_k(int k) {
		return uss.get_top_k(k);
	}
};


#endif // _CLOCKUSS_H_