#ifndef _CLOCKUSS_H_
#define _CLOCKUSS_H_

#include "UnbiasedSpaceSaving.h"
#include "ClockSketch.h"

template <bool use_counter = false>
class ClockUSS {
private:
    using _ClockSketch = ClockSketch<use_counter>;
    UnbiasedSpaceSaving uss;
    _ClockSketch cs;

    inline int suggestMemory(int memory, double time_threshold) {
        int suggest_max;
        if (time_threshold > 0.001) {
            suggest_max = 50000;
        } else {
            suggest_max = 2000;
        }
        return min(memory / 2, suggest_max);
    }

public:
#define csmem suggestMemory(memory, time_threshold)
#define sz max(1, memory / 1000)
    ClockUSS(double time_threshold, double unit_time, int memory, int seed)
        : uss(time_threshold, unit_time, memory - csmem, sz, seed),
          cs(csmem, time_threshold, seed) {}
#undef sz
#undef csmem
    void insert(int key, double time) {
        bool b = cs.insert(key, time);
        uss.insert(key, time, b);
    }

    void insert_filter(int key, double time, size_t min_size) {
        int size = cs.insert_cnt(key, time) + 1;
        if (size >= min_size)
            uss.insert(key, time, size == min_size);
    }

    template <size_t min_size>
    void insert_filter(int key, double time) {
        static_assert(min_size <= _ClockSketch::MaxReportSize);
        int size = cs.insert_cnt(key, time) + 1;
        if (size >= min_size)
            uss.insert(key, time, size == min_size);
    }

    vector<pair<pair<int, int16_t>, int>> get_top_k(int k) const {
        return uss.get_top_k(k);
    }
};


#endif // _CLOCKUSS_H_
