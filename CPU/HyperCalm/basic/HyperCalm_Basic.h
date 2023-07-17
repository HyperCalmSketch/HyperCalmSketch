#ifndef _HYPERCALMBASIC_H_
#define _HYPERCALMBASIC_H_

#include "CalmSpaceSaving_Basic.h"
#include "../HyperBloomFilter.h"

class HyperCalm_Basic {
private:
    using HBF = HyperBloomFilter<>;
    CalmSpaceSaving_Basic css;
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
#define sz1 max(1, memory / 1000)
#define sz2 max(1, memory / 100)
	HyperCalm_Basic(double time_threshold, double unit_time,
		int memory, int seed): css(time_threshold, unit_time, memory - memory2, 3, sz1, sz2), hbf(memory2, time_threshold, seed) {
	}
#undef sz1
#undef sz2
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
        //static_assert(min_size <= HBF::MaxReportSize);
        int size = hbf.insert_cnt(key, time) + 1;
        if (size >= min_size)
            css.insert(key, time, size == min_size);
    }

    vector<pair<pair<int, int16_t>, int>> get_top_k(int k) const {
        return css.get_top_k(k);
    }
};

#endif  // _HYPERCALM_H_