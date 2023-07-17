#ifndef _CALMSPACESAVING_H_
#define _CALMSPACESAVING_H_

#include "Lru.h"

using namespace std;

#define DEP_PER_DEQUE 32

#define Mod2(x) (x%circular_array_size + circular_array_size)%circular_array_size

class CalmSpaceSaving : public LRU
{
protected:
    struct Deques{
    public:
        uint16_t keys[DEP_PER_DEQUE];
        float last_batch[DEP_PER_DEQUE];
		float last_item[DEP_PER_DEQUE];
    }*deques;
    const int circular_array_size, dep = DEP_PER_DEQUE;
    const double time_threshold, unit_time;
	char *head;
	void array_push_key(uint32_t key, float time){
		const int key_mod_n = Mod2(key) ,Head = head[key_mod_n];
        const uint16_t key_fingerprint = (key/circular_array_size) % 65535 + 1; 
		deques[key_mod_n].keys[Head] = key_fingerprint;
		deques[key_mod_n].last_item[Head] = time;
		deques[key_mod_n].last_batch[Head] = time;
		head[key_mod_n] = (head[key_mod_n] + 1)%dep;
	}
	void array_replace_key(uint32_t key, float time, int index){
		const int key_mod_n = Mod2(key) ,Head = head[key_mod_n];
        const uint16_t key_fingerprint = (key/circular_array_size) % 65535 + 1; 
		deques[key_mod_n].last_item[index] = deques[key_mod_n].last_item[Head];
		deques[key_mod_n].last_batch[index] = deques[key_mod_n].last_batch[Head];
		deques[key_mod_n].keys[index] = deques[key_mod_n].keys[Head];
		deques[key_mod_n].keys[Head] = key_fingerprint;
		deques[key_mod_n].last_item[Head] = time;
		deques[key_mod_n].last_batch[Head] = time;
		head[key_mod_n] = (head[key_mod_n] + 1)%dep;
	}
public:
	CalmSpaceSaving(double _time_threshold, double _unit_time,
		int memory, int _count_threshold, int _q_mem,
		int _circular_array_mem):
                    circular_array_size(_circular_array_mem/(sizeof(Deques)+sizeof(char))),
                    time_threshold(_time_threshold),
                    unit_time(_unit_time),
                    LRU(memory - _circular_array_mem, _count_threshold, _q_mem){
		printf("c = %d\t (Number of the TimeRecorder queue)\n",circular_array_size);
        printf("w = %d\t (Number of the LRU queue in CalmSS)\n",q_size);
        printf("w = %d\t (Number of the SS in CalmSS)\n",capacity);
        deques = new(std::align_val_t{64}) Deques[circular_array_size];
		head = new char[circular_array_size];
        memset(deques, 0, sizeof(Deques)*circular_array_size);
		memset(head, 0, sizeof(char)*circular_array_size);
	}
	~CalmSpaceSaving() {
		delete[] deques;
		delete[] head;
	}
	bool insert(uint32_t key, float time, bool bf_new, int freq = 1) {
		const int key_mod_n = Mod2(key);
        const uint16_t key_fingerprint = (key/circular_array_size) % 65535 + 1; 
		if constexpr(use_simd) {
			const __m512i item = _mm512_set1_epi16(key_fingerprint);                                                                                                                                                                                             
			__m512i *keys_p = (__m512i*)(deques[key_mod_n].keys);
			uint64_t matched = matched = _mm512_cmpeq_epi16_mask(item, *keys_p);
			if(matched != 0){
				int index = __tzcnt_u64(matched);
				//printf("%d %d\n",key_fingerprint,deques[key_mod_n].keys[index]);
				if(time - deques[key_mod_n].last_item[index] < time_threshold){
					deques[key_mod_n].last_item[index] = time;
					return 0;
				}
				if(!bf_new) return 0;
				int16_t delta = (time - deques[key_mod_n].last_batch[index]) / unit_time;
				insert_calm(key, delta, 1);
				array_replace_key(key, time, index);
				return 1;
			}
			if(!bf_new) return 0;
			array_push_key(key, time);
			return 1;
		}
		else{if(!bf_new) return 0;
			for(int i=0; i<dep; i++)
				if(deques[key_mod_n].keys[i] == key_fingerprint){
					if(time - deques[key_mod_n].last_item[i] < time_threshold){
						deques[key_mod_n].last_item[i] = time;
						return 0;
					}
					
					int16_t delta = (time - deques[key_mod_n].last_batch[i]) / unit_time;
					insert_calm(key, delta, 1);
					array_replace_key(key, time, i);
					return 1;
				}
			if(!bf_new) return 0;
			array_push_key(key, time);
			return 1;
		}
	}
};

#endif // _CALMSPACESAVING_H_