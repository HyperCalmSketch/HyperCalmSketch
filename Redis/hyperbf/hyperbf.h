#include "basic_sketch/basic_sketch.h"

#include <assert.h>
#include <immintrin.h>
#include <random>
#include <type_traits>

#define MIN(x, y) ((x) < (y) ? (x) : (y))

static constexpr uint64_t ODD_BIT_MASK = 0x5555555555555555;
enum Bits : uint64_t {
    _00 = 0x0000000000000000,
    _01 = 0x5555555555555555,
    _10 = 0xaaaaaaaaaaaaaaaa,
    _11 = 0xffffffffffffffff,
};
static constexpr uint64_t STATE_MASKS[] = { Bits::_01, Bits::_10, Bits::_11 };
static constexpr uint64_t MASK[] = { ~Bits::_01, ~Bits::_10, ~Bits::_11 };

class basic_hb : public basic_sketch
{
protected:
    static constexpr size_t TableNum = 8;
	static constexpr size_t CellPerBucket = 32;
	static constexpr size_t CellBits = 2;
	static constexpr size_t CellMask = (1 << CellBits) - 1;

public:
	double time_threshold;
	uint32_t bucket_num;
	uint32_t seeds[TableNum + 1];
    uint64_t* buckets;

    using basic_sketch::operator new;
    using basic_sketch::operator new[];
    using basic_sketch::operator delete;
    using basic_sketch::operator delete[];
    basic_hb(int argc, basic_sketch_string *argv)
    {
        uint32_t memory = argv[0].to_int();
        double time_threshold_ = argv[1].to_double();
        int seed = 123;
        if (argc != 2)
            seed = argv[2].to_int();
        bucket_num = memory / TableNum / sizeof(uint64_t) * TableNum;
		time_threshold = time_threshold_;
        buckets = (uint64_t *)CALLOC(bucket_num, sizeof(uint64_t));
		memset(buckets, 0, bucket_num * sizeof(*buckets));
        std::mt19937 rng(seed);
        for (int i = 0; i <= TableNum; ++i) {
			seeds[i] = rng();
		}
    }
    basic_hb(const basic_sketch_string &s)
    {
        size_t tmp = 0;
        const char *ss = s.c_str();

        memcpy(&time_threshold, ss + tmp, sizeof(double));
        tmp += sizeof(double);

        memcpy(&bucket_num, ss + tmp, sizeof(uint32_t));
        tmp += sizeof(uint32_t);

        memcpy(seeds, ss + tmp, sizeof(uint32_t) * (TableNum + 1));
        tmp += sizeof(uint32_t) * (TableNum + 1);

        buckets = new uint64_t[bucket_num];
        memcpy(buckets, ss + tmp, sizeof(uint64_t) * bucket_num);
    }
    ~basic_hb()
    {
        FREE(buckets);
    }
    basic_sketch_string *to_string()
    {
        size_t string_size = sizeof(double) + sizeof(uint32_t) * (TableNum + 2) + sizeof(uint64_t) * bucket_num;
        char *s = (char *)CALLOC(string_size, sizeof(char));
        size_t tmp = 0;

        memcpy(s + tmp, &time_threshold, sizeof(double));
        tmp += sizeof(double);

        memcpy(s + tmp, &bucket_num, sizeof(uint32_t));
        tmp += sizeof(uint32_t);

        memcpy(s + tmp, seeds, sizeof(uint32_t) * (TableNum + 1));
        tmp += sizeof(uint32_t) * (TableNum + 1);

        buckets = (uint64_t *)CALLOC(bucket_num, sizeof(uint64_t));
        memcpy(s + tmp, buckets, sizeof(uint64_t) * bucket_num);
        tmp += sizeof(uint64_t) * bucket_num;

        basic_sketch_string *bs = new basic_sketch_string(s, tmp);
        delete s;

        return bs;
    }
    basic_sketch_reply *insert(const int &argc, const basic_sketch_string *argv)
    {
        basic_sketch_reply *result = new basic_sketch_reply;
        for (int c = 0; c < argc/2; ++c){
             
            long long ans = 0;
            const uint32_t key = argv[c<<1].to_int();
            const double time = argv[c<<1|1].to_double();
            const int first_bucket_pos = CalculatePos(key, TableNum) % bucket_num & ~(TableNum - 1);
            
            for (int i = 0; i < TableNum; ++i) {
				int pos = CalculatePos(key, i) % 32;
				int bucket_pos = (first_bucket_pos + i);

				int now_tag = int(time / time_threshold + 1.0 * i / TableNum) % 3 + 1;
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
				x ^= uint64_t(now_tag^old_tag) << (2 * pos);
			}
            result->push_back(ans);
        }
        
        return result;
    }

    static basic_sketch_reply *Insert(void *o, const int &argc, const basic_sketch_string *argv)
    {
        return ((basic_hb *)o)->insert(argc, argv);
    }

    static int command_num() { return 1; }
    static basic_sketch_string command_name(int index)
    {
        basic_sketch_string tmp[] = {"insert"};
        return tmp[index];
    }
    static basic_sketch_func command(int index)
    {
        basic_sketch_func tmp[] = {(basic_hb::Insert)};
        return tmp[index];
    }
    static basic_sketch_string class_name() { return "basic_hb"; }
    static int command_type(int index)
    {
        int tmp[] = {0};
        return tmp[index];
    }
    static char *type_name() { return "BASIC_HBS"; }
private:
	inline uint32_t CalculatePos(uint32_t key, int i) {
		return (key * seeds[i]) >> 15;
	}
};
