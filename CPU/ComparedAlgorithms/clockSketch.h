#ifndef _CLOCKSKETCH_H_
#define _CLOCKSKETCH_H_
#include "../lib/param.h"
#include <random>

#define TABLE_NUM 4
#define S 4
#define C (64 / S)
class clockSketch {
public:
	uint64_t* buckets;
	double time_threshold;
	int64_t la_time;
	uint32_t bucket_num, la_pos;
	uint32_t seeds[TABLE_NUM + 1];

	clockSketch(uint32_t memory, double time_threshold_, int seed = 233) {
		bucket_num = memory / sizeof(uint64_t);
		time_threshold = time_threshold_;
		buckets = new uint64_t[bucket_num];
		memset(buckets, 0, bucket_num * sizeof(*buckets));
		la_time = la_pos = 0;
		mt19937 rng(seed);
		for (int i = 0; i <= TABLE_NUM; ++i) {
			seeds[i] = rng();
		}
        printf(" %d\t (Number of arrays in Clock-Sketch)\n",bucket_num);
	}

	~clockSketch() {
		delete[] buckets;
	}

	bool insert(int key, double time) {
		int64_t nt = time * ((1 << S) - 2) * (bucket_num * C) / time_threshold;
		if (la_time) {
			int d = nt - la_time;
			if (d >= (int)bucket_num * C * S) {
				memset(buckets, 0, bucket_num * sizeof(*buckets));
				d = 0;
			}
			for (; d > 0 && la_pos % C; --d) {
				uint64_t v = uint64_t(1) << (la_pos % C * S);
				if (buckets[la_pos / C] & v * ((1 << S) - 1))
					buckets[la_pos / C] -= v;
				(++la_pos) %= (bucket_num * C);
			}
			for (; d >= C; d -= C) {
				uint64_t x = buckets[la_pos / C];
				if (S == 8) {
					x |= x >> 4;
					x |= x >> 2;
					x |= x >> 1;
					buckets[la_pos / C] -= x & 0x0101010101010101;
				} else if (S == 4) {
					x |= x >> 2;
					x |= x >> 1;
					buckets[la_pos / C] -= x & 0x1111111111111111;
				}
				(la_pos += C) %= (bucket_num * C);
			}
			for (; d > 0; --d) {
				uint64_t v = uint64_t(1) << (la_pos % C * S);
				if (buckets[la_pos / C] & v * ((1 << S) - 1))
					buckets[la_pos / C] -= v;
				(++la_pos) %= (bucket_num * C);
			}
		}
		la_time = nt;
		bool ans = 0;
		for (int i = 0; i < TABLE_NUM; ++i) {
			int pos = CalculatePos(key, i) % (bucket_num * C);
			uint64_t v = uint64_t((1 << S) - 1) << (pos % C * S);
			if ((buckets[pos / C] & v) == 0)
				ans = 1;
			buckets[pos / C] |= v;
		}
		return ans;
	}

private:
	inline uint32_t CalculatePos(uint32_t key, int i) {
		return CalculateBucketPos(key, seeds[i]);
	}
};

#undef TABLE_NUM
#undef S
#undef C

#endif //_CLOCKSKETCH_H_