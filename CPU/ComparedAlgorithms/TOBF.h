#ifndef _TOBF_H_
#define _TOBF_H_
#include "../lib/murmur3.h"
class TOBF {
	float* last_time;
	double time_threshold;
	int hash_num, tot, seed;

public:
	TOBF(uint32_t memory, double time_threshold_, int _hash_num, int _seed): time_threshold(time_threshold_), hash_num(_hash_num) {
		tot = memory / (sizeof(float));
		last_time = new float[tot];
	}
	~TOBF() {
		delete[] last_time;
	}
	bool insert(int key, float time) {
		bool is_new = 0;
		for (int i = 0; i < hash_num; ++i) {
			int pos = MurmurHash3_x86_32((const char*)(&key), sizeof(int), seed * hash_num + i) % tot;
			is_new |= time - last_time[pos] >= time_threshold;
			last_time[pos] = time;
		}
		return is_new;
	}
};
#endif //_TOBF_H_