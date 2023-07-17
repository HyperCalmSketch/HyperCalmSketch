#ifndef _SWAMP_H_
#define _SWAMP_H_
#include "../lib/HashTable.h"

template<typename key_t, typename time_t>
class SWAMP {
	key_t* q;
	int head, tail, tot=0; //[head,tail] \pmod tot
	time_t time_threshold;
	Hash_table<key_t, pair<time_t, int>> last_time;

public:
	SWAMP(uint32_t memory, time_t time_threshold_): 
			tot(memory / (sizeof(key_t) * 2 + sizeof(time_t) + sizeof(int) * 2)),
			time_threshold(time_threshold_),
			last_time(tot + 5) {
        fprintf(stderr, "SWAMP Size = %d\n", tot);
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
#endif //_SWAMP_H_