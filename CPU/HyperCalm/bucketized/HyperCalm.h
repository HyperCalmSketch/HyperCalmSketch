#ifndef _HYPERCALM_H_
#define _HYPERCALM_H_

#include "CalmSpaceSaving.h"
#include "../HyperBloomFilter.h"

class HyperCalm {
private:
    using HBF = HyperBF::HyperBloomFilter<>;
    CalmSpaceSaving css;
    HBF hbf;

    inline int suggestHBFMemory(int memory, double time_threshold) {
        int suggest_max;
        if (time_threshold > 0.001) {
            suggest_max = 50000;
        } else {
            suggest_max = 3000;
        }
        return min(memory / 2, suggest_max);
    }

public:
#define memory2 suggestHBFMemory(memory, time_threshold)
#define mem1 min(15000, memory/15)
#define mem2 max(1, memory - mem1*7 - memory2)
	HyperCalm(double time_threshold, double unit_time,
		int memory, int seed): css(time_threshold, unit_time, memory - memory2, 2, mem1, mem2), 
			hbf(memory2, time_threshold, seed) {}
#undef mem1
#undef mem2
#undef memory2
    void insert(int key, double time) {
        bool b = hbf.insert(key, time);
        css.insert(key, time, b);
    }

    void insert_filter(int key, double time, size_t min_size) {
        int size = hbf.insert_cnt(key, time) + 1;
        if (size >= min_size)
            css.insert(key, time, size == min_size);
    }

    template <size_t min_size>
    void insert_filter(int key, double time) {
        static_assert(min_size <= hbf.MaxReportSize);
        int size = hbf.insert_cnt(key, time) + 1;
        if (size >= min_size)
            css.insert(key, time, size == min_size);
    }

    vector<pair<pair<int, int16_t>, int>> get_top_k(int k) const {
        return css.get_top_k(k);
    }
};

#endif  // _HYPERCALM_H_
