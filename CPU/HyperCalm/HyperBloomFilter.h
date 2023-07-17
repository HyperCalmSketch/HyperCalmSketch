#ifndef _HYPERBLOOMFILTER_H_
#define _HYPERBLOOMFILTER_H_

#include <immintrin.h>
#include <random>

#include "../lib/param.h"

static const uint64_t MASK[] = { 0xaaaaaaaaaaaaaaaa, 0x5555555555555555, 0 /*0xffffffffffffffff*/ };
#define CELL_PER_BUCKET 32
#define TABLE_NUM 8
class HyperBloomFilter {
public:
	uint64_t* buckets;
	double time_threshold;
	uint32_t bucket_num;
	uint32_t seeds[TABLE_NUM + 1];
	bool use_SIMD;

	HyperBloomFilter(uint32_t memory, double time_threshold_, int seed = 123) {
		bucket_num = memory / TABLE_NUM / sizeof(uint64_t) * TABLE_NUM;
		time_threshold = time_threshold_;
		buckets = new (align_val_t { 64 }) uint64_t[bucket_num];
		memset(buckets, 0, bucket_num * sizeof(*buckets));
		mt19937 rng(seed);
		for (int i = 0; i <= TABLE_NUM; ++i) {
			seeds[i] = rng();
		}
        printf("d = %d\t (Number of arrays in HyperBF)\n",bucket_num);
	}

	~HyperBloomFilter() {
		delete[] buckets;
	}

	bool insert(int key, double time) {
		int first_bucket_pos = CalculatePos(key, TABLE_NUM) % bucket_num & ~(TABLE_NUM - 1);
		bool ans = 0;
#ifdef SIMD
			__m512i* x = (__m512i*)(buckets + first_bucket_pos);
			uint64_t b[8];
			for (int i = 0; i < 8; ++i) {
				int now_tag = int(time / time_threshold + 1.0 * i / TABLE_NUM) % 3 + 1;
				int ban_tag = now_tag % 3;
				b[7 - i] = MASK[ban_tag];
			}
			__m512i ban = _mm512_set_epi64(b[0], b[1], b[2], b[3], b[4], b[5], b[6], b[7]);
			ban = _mm512_xor_epi64(*x, ban);
			ban = _mm512_and_epi64(_mm512_rol_epi64(ban, 63), ban);
			ban = _mm512_and_epi64(ban, _mm512_set1_epi64(MASK[1]));
			ban = _mm512_or_epi64(_mm512_rol_epi64(ban, 1), ban);
			*x = _mm512_andnot_epi64(ban, *x);
			for (int i = 0; i < TABLE_NUM; ++i) {
				int pos = CalculatePos(key, i) % 32;
				int bucket_pos = (first_bucket_pos + i);

				int now_tag = int(time / time_threshold + 1.0 * i / TABLE_NUM) % 3 + 1;

				uint64_t& x = buckets[bucket_pos];
				int old_tag = (x >> (2 * pos)) & 3;
				if (old_tag == 0)
					ans = 1;
				x += uint64_t(now_tag - old_tag) << (2 * pos);
			}
#else
			for (int i = 0; i < TABLE_NUM; ++i) {
				int pos = CalculatePos(key, i) % 32;
				int bucket_pos = (first_bucket_pos + i);

				int now_tag = int(time / time_threshold + 1.0 * i / TABLE_NUM) % 3 + 1;
				int ban_tag = now_tag % 3 + 1;

				uint64_t& x = buckets[bucket_pos];
				uint64_t ban_bits = x ^ MASK[ban_tag - 1];
				ban_bits &= ban_bits >> 1;
				ban_bits &= MASK[1];
				ban_bits |= ban_bits << 1;
				x &= ~ban_bits;

				int old_tag = (x >> (2 * pos)) & 3;
				if (old_tag == 0)
					ans = 1;
				x += uint64_t(now_tag - old_tag) << (2 * pos);
			}
#endif
		return ans;
	}

private:
	inline uint32_t CalculatePos(uint32_t key, int i) {
		return CalculateBucketPos(key, seeds[i]);
	}
};

#undef CELL_PER_BUCKET
#undef TABLE_NUM


#endif // _HYPERBLOOMFILTER_H_
