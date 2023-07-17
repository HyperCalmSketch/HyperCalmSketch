#ifndef _HYPERCALM_H_
#define _HYPERCALM_H_

#include "CalmSpaceSaving.h"
#include "../HyperBloomFilter.h"

class HyperCalm {
private:
    using HBF = HyperBloomFilter<>;
    CalmSpaceSaving css;
    HBF hbf;

    inline int suggestHBFMemory(int memory, double time_threshold) {
        int suggest_max;
        if (time_threshold > 0.001) {
            suggest_max = 50000;
        } else {
            suggest_max = 2000;
        }
        return min(memory / 2, suggest_max);
    }

public:
#define memory2 suggestHBFMemory(memory, time_threshold)
#define mem1 min(15000, memory/17)
#define mem2 max(1, memory - mem1*10 - memory2)
	HyperCalm(double time_threshold, double unit_time, int memory, int seed, int _time_or_num=0): 
            css(time_threshold, unit_time, memory - memory2, 2, mem1, mem2, _time_or_num), 
			hbf(memory2, time_threshold, seed) {}
#undef mem1
#undef mem2
#undef memory2
    void insert(int key, double time) {
        bool b = hbf.insert(key, time);
        css.insert(key, time, b);
    }
    vector<pair<pair<int, int16_t>, pair<int, double>>> get_top_k(int k) const {
        return css.get_top_k(k);
    }
};

#endif  // _HYPERCALM_H_
