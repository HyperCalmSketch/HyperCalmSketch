#include "basic_sketch/basic_sketch.h"

#include <assert.h>
#include <queue>
#include <immintrin.h>
#include <type_traits>
#include <vector>

class calm : public basic_sketch
{
#define DEP_PER_TIMERECORDER 16
#define CELL_PER_LRU 16
#define CELL_PER_SS 8
protected:
    const uint32_t dep_timerecorder = DEP_PER_TIMERECORDER, dep_lru = CELL_PER_LRU, dep_ss = CELL_PER_SS;
    const uint32_t count_threshold = 2;
    uint32_t n_timerecorder, n_lru, n_ss;
    double time_threshold, unit_time;
    struct TimeRecorder{
        float last_batch[DEP_PER_TIMERECORDER];
		float last_item[DEP_PER_TIMERECORDER];
        uint32_t head;
        uint16_t keys[DEP_PER_TIMERECORDER];
    }*timerecorder;
    struct Lru{
        uint32_t freqs;
        uint32_t head;
        uint16_t keys[CELL_PER_LRU];
    }*lru;
    struct SS{
        uint32_t keys[CELL_PER_SS];
        uint32_t freq[CELL_PER_SS];
        uint16_t delta[CELL_PER_SS];
    }*ss;
#undef DEP_PER_TIMERECORDER
#undef CELL_PER_LRU
#undef CELL_PER_SS 
public:
    using basic_sketch::operator new;
    using basic_sketch::operator new[];
    using basic_sketch::operator delete;
    using basic_sketch::operator delete[];
    calm(int argc, basic_sketch_string *argv):
            n_timerecorder(argv[0].to_int()),
            n_lru(argv[1].to_int()),
            n_ss(argv[2].to_int()),
            time_threshold(argv[3].to_double()), 
            unit_time(argv[4].to_double()){
        timerecorder = new TimeRecorder[n_timerecorder];
        lru = new Lru[n_lru];
        ss = new SS[n_ss];
        memset(timerecorder, 0, sizeof(TimeRecorder) * n_timerecorder);
        memset(lru, 0, sizeof(Lru) * n_lru);
        memset(ss, 0, sizeof(SS) * n_ss);
    }
    calm(const basic_sketch_string &s)
    {
        size_t tmp = 0;
        const char *S = s.c_str();

        memcpy(&n_timerecorder, S + tmp, sizeof(uint32_t));
        tmp += sizeof(uint32_t);

        memcpy(&n_lru, S + tmp, sizeof(uint32_t));
        tmp += sizeof(uint32_t);

        memcpy(&n_ss, S + tmp, sizeof(uint32_t));
        tmp += sizeof(uint32_t);

        memcpy(&time_threshold, S + tmp, sizeof(double));
        tmp += sizeof(double);
        
        memcpy(&unit_time, S + tmp, sizeof(double));
        tmp += sizeof(double);

        timerecorder = new TimeRecorder[n_timerecorder];
        lru = new Lru[n_lru];
        ss = new SS[n_ss];
        
        memcpy(timerecorder, S + tmp, n_timerecorder * sizeof(TimeRecorder));
        tmp += n_timerecorder * sizeof(TimeRecorder);

        memcpy(lru, S + tmp, n_lru * sizeof(Lru));
        tmp += n_lru * sizeof(Lru);

        memcpy(ss, S + tmp, n_ss * sizeof(SS));
    }
    ~calm()
    {
        delete [] timerecorder;
        delete [] lru;
        delete [] ss;
    }

    const uint32_t Mod(const uint32_t x, const uint32_t mod){
        return (x%mod+mod)%mod;
    }
    uint32_t value(uint32_t val, int i){
        return (val>>i)&1;
    }
    void change(uint32_t &val, int i, int j){
        val ^= (value(val, i) ^ j) << i;
    }
    const uint32_t zero_cnt(const uint32_t x){
        static const uint32_t MultiplyDeBruijnBitPosition[32] ={
            0, 1, 28, 2, 29, 14, 24, 3, 30, 22, 20, 15, 25, 17, 4, 8,
            31, 27, 13, 23, 21, 19, 16, 7, 26, 12, 18, 6, 11, 5, 10, 9
            };
        return MultiplyDeBruijnBitPosition[((uint32_t)((x & -x) * 0x077CB531U)) >> 27];
    }

    void array_push_key(uint32_t key, float time){
		const int key_mod_n = Mod(key, n_timerecorder) ,Head = timerecorder[key_mod_n].head;
        const uint16_t key_fingerprint = (key / n_timerecorder)%65535 + 1; 
		timerecorder[key_mod_n].keys[Head] = key_fingerprint;
		timerecorder[key_mod_n].last_item[Head] = time;
		timerecorder[key_mod_n].last_batch[Head] = time;
		timerecorder[key_mod_n].head = (Head + 1)%dep_timerecorder;
	}
	void array_replace_key(uint32_t key, float time, int index){
		const int key_mod_n = Mod(key, n_timerecorder) ,Head = timerecorder[key_mod_n].head;
        const uint16_t key_fingerprint = (key / n_timerecorder)%65535 + 1; 
		timerecorder[key_mod_n].last_item[index] = timerecorder[key_mod_n].last_item[Head];
		timerecorder[key_mod_n].last_batch[index] = timerecorder[key_mod_n].last_batch[Head];
		timerecorder[key_mod_n].keys[index] = timerecorder[key_mod_n].keys[Head];
		timerecorder[key_mod_n].keys[Head] = key_fingerprint;
		timerecorder[key_mod_n].last_item[Head] = time;
		timerecorder[key_mod_n].last_batch[Head] = time;
		timerecorder[key_mod_n].head = (Head + 1)%dep_timerecorder;
	}

    void append_new_key(uint32_t key, uint16_t time, int freq = 1) {
        const int key_mod_n = Mod(key + time, n_lru);
        uint16_t key_finger = (key + time) % 65535 + 1;
        uint32_t matched = 0;
        for(int i=0; i<dep_lru; ++i)
            if(lru[key_mod_n].keys[i] == key_finger){
                SS_update(key, time, count_threshold);
                lru[key_mod_n].keys[i] = 0;
                change(lru[key_mod_n].freqs, i, 0);
                return ;
            }
        matched = (~lru[key_mod_n].freqs) & 65535;
        if(matched != 0){
            int index = zero_cnt(matched);
            change(lru[key_mod_n].freqs, index, freq);
            lru[key_mod_n].keys[index] = key_finger;
            return ;
        }
        lru[key_mod_n].keys[lru[key_mod_n].head] = key_finger;
        lru[key_mod_n].head = (lru[key_mod_n].head+ 1)%dep_lru;
    }
    void insert_lru(uint32_t key, uint16_t time, int freq = 1){
        int i = find(key, time);
        if (i == 0) {
            append_new_key(key, time, freq);
        } else {
            add_counter(key, time, i-1, freq);
        }
    }

    int find(uint32_t key, uint16_t time){
        const int key_mod_n = Mod(key+time, n_ss);
        for(int i=0; i<dep_ss; ++i)
            if(ss[key_mod_n].keys[i] == key && ss[key_mod_n].delta[i] == time)
                return i + 1;
        return 0;
    }
    void add_counter(uint32_t key, uint16_t time, int i, int freq = 1){
        const int key_mod_n = Mod(key+time, n_ss);
        ss[key_mod_n].freq[i] += freq;
    }
    void SS_update(uint32_t key, uint16_t time, int freq = 1){
        const int key_mod_n = Mod(key+time, n_ss);
        int index = 0;
        for(int i=1; i<dep_ss && ss[key_mod_n].freq[index]; i++)
            if(ss[key_mod_n].freq[i] < ss[key_mod_n].freq[index])
                index = i;
        // printf("%d %d\n",key_mod_n, index);/;
        ss[key_mod_n].freq[index] += freq;
        ss[key_mod_n].delta[index] = time;
        ss[key_mod_n].keys[index] = key;
    }

    basic_sketch_string *to_string()
    {
        size_t string_size = sizeof(uint32_t)*3 + sizeof(double)*2 + 
                             n_timerecorder * sizeof(TimeRecorder) + n_lru * sizeof(Lru) + n_ss * sizeof(SS);
        char *S = (char *)CALLOC(string_size, sizeof(char));
        size_t tmp = 0;

        memcpy(S + tmp, &n_timerecorder, sizeof(uint32_t));
        tmp += sizeof(uint32_t);

        memcpy(S + tmp, &n_lru, sizeof(uint32_t));
        tmp += sizeof(uint32_t);

        memcpy(S + tmp, &n_ss, sizeof(uint32_t));
        tmp += sizeof(uint32_t);

        memcpy(S + tmp, &time_threshold, sizeof(double));
        tmp += sizeof(double);
        
        memcpy(S + tmp, &unit_time, sizeof(double));
        tmp += sizeof(double);
        
        memcpy(S + tmp, timerecorder, n_timerecorder * sizeof(TimeRecorder));
        tmp += n_timerecorder * sizeof(TimeRecorder);

        memcpy(S + tmp, lru, n_lru * sizeof(Lru));
        tmp += n_lru * sizeof(Lru);

        memcpy(S + tmp, ss, n_ss * sizeof(SS));
        tmp += n_ss * sizeof(SS);

        basic_sketch_string *bs = new basic_sketch_string(S, tmp);
        delete S;

        return bs;
    }
    basic_sketch_reply *insert(const int &argc, const basic_sketch_string *argv)
    {
        basic_sketch_reply *result = new basic_sketch_reply;
        long long ans = 0;
        for (int c = 0; c < argc; c+=3){
            const uint32_t key = argv[c].to_int();
            const double time = argv[c+1].to_double();
            const int bf_new = argv[c+2].to_long_long();
            const int key_mod_n = Mod(key, n_timerecorder);
            const uint16_t key_fingerprint = (key / n_timerecorder)%65535 + 1; 
            for(int i=0; i<dep_timerecorder; i++)
				if(timerecorder[key_mod_n].keys[i] == key_fingerprint){
					if(time - timerecorder[key_mod_n].last_item[i] < time_threshold){
						timerecorder[key_mod_n].last_item[i] = time;
                        goto Break;
					}
					int16_t delta = (time - timerecorder[key_mod_n].last_batch[i]) / unit_time;
					insert_lru(key, delta, 1);
					array_replace_key(key, time, i);
					goto Break;
				}
            if(!bf_new) {
                goto Break;
            }
			array_push_key(key, time);
            Break:
            ans++;
        }
        result->push_back(ans);
        return result;
    }
    basic_sketch_reply *topk(const int &argc, const basic_sketch_string *argv)
    {
        basic_sketch_reply *result = new basic_sketch_reply;
        for (int c = 0; c < argc; ++c){
            const int k = argv[c].to_int();
            std::priority_queue<std::pair<int, std::pair<int, int16_t> >, std::vector<std::pair<int, std::pair<int, int16_t> > >, std::greater<std::pair<int, std::pair<int, int16_t> > > > Q;
            for(int i=0; i<n_ss; ++i)
                for(int j=0; j<dep_ss; ++j){
                    if(ss[i].freq[j]){
                        Q.push(make_pair(ss[i].freq[j], std::make_pair(ss[i].keys[j],ss[i].delta[j])));   
                        if(Q.size() > k) Q.pop();
                     }
                }
            while(Q.size()){
                std::pair<int, int16_t> key = Q.top().second;
                int count = Q.top().first;
                result->push_back((long long)key.first);
                result->push_back((long long)key.second);
                result->push_back((long long)count);
                Q.pop();
            }
        }
        return result;
    }

    static basic_sketch_reply *Insert(void *o, const int &argc, const basic_sketch_string *argv)
    {
        return ((calm *)o)->insert(argc, argv);
    }
    static basic_sketch_reply *Topk(void *o, const int &argc, const basic_sketch_string *argv)
    {
        return ((calm *)o)->topk(argc, argv);
    }

    static int command_num() { return 2; }
    static basic_sketch_string command_name(int index)
    {
        basic_sketch_string tmp[] = {"insert", "topk"};
        return tmp[index];
    }
    static basic_sketch_func command(int index)
    {
        basic_sketch_func tmp[] = {(calm::Insert), (calm::Topk)};
        return tmp[index];
    }
    static basic_sketch_string class_name() { return "calm"; }
    static int command_type(int index)
    {
        int tmp[] = {0, 1};
        return tmp[index];
    }
    static char *type_name() { return "BASICCALM"; }
};