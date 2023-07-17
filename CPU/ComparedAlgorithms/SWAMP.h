#ifndef _SWAMP_H_
#define _SWAMP_H_
#include "../lib/HashTable.h"

template <typename key_t, typename time_t, bool use_counter = false>
class SWAMP;

template<typename key_t, typename time_t>
class SWAMP<key_t, time_t, false> {
    using Table = Hash_table<key_t, pair<time_t, int>>;
    key_t* q;
    int head, tail, tot=0; //[head,tail] \pmod tot
    time_t time_threshold;
    Table last_time;

public:
    SWAMP(uint32_t memory, time_t time_threshold_): 
            tot(memory / (sizeof(key_t) + sizeof(typename Table::Node))),
            time_threshold(time_threshold_),
            last_time(tot + 5) {
        q = new key_t[tot];
        head = 1;
        tail = 0;
    }
    ~SWAMP() {
        delete[] q;
    }
    bool insert(key_t key, time_t time) {
        bool is_new = 0;
        if (!last_time.count(key) || time - last_time[key].first > time_threshold)
            is_new = 1;
        if (tail - head + 1 == tot) {
            if (!--last_time[q[head % tot]].second)
                last_time.erase(q[head % tot]);
            ++head;
        }
        ++tail;
        q[tail % tot] = key;
        last_time[key].first = time;
        last_time[key].second++;
        return is_new;
    }
};

template <typename key_t, typename time_t>
class SWAMP<key_t, time_t, true> {
    struct batch_info {
        time_t first;
        int second;
        int count;
    };

    using Table = Hash_table<key_t, batch_info>;
    key_t* q;
    int head = 1, tail = 0;
    int tot;
    time_t time_threshold;
    Table last_time;

public:
    SWAMP(uint32_t memory, time_t time_threshold)
        : tot(memory / (sizeof(key_t) + sizeof(typename Table::Node))),
          time_threshold(time_threshold), last_time(tot + 5) {
        q = new key_t[tot];
    }
    ~SWAMP() {
        delete[] q;
    }
    bool insert(key_t key, time_t time) {
        return !insert_cnt(key, time);
    }
    int insert_cnt(key_t key, time_t time) {
        bool is_new = 0;
        if (!last_time.count(key) || time - last_time[key].first > time_threshold)
            is_new = 1;
        if (tail - head + 1 == tot) {
            if (!--last_time[q[head % tot]].second)
                last_time.erase(q[head % tot]);
            ++head;
        }
        ++tail;
        q[tail % tot] = key;
        last_time[key].first = time;
        last_time[key].second++;
        if (!is_new)
            return ++last_time[key].count;
        return 0;
    }
};
#endif //_SWAMP_H_
