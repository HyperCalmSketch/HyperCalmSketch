#ifndef _CALMSPACESAVINGBASIC_H_
#define _CALMSPACESAVINGBASIC_H_

#include <cstdint>
#include <cstdlib>
#include <vector>
#include "CalmSpaceSavingTopK.h"

#include "../../lib/HashTable.h"

using namespace std;

class CalmSpaceSaving_Basic : public CalmSpaceSavingTopK
{
    struct Last_Time{
        float last_batch, last_item;
        int count;
        uint16_t delta;
        Last_Time(float _last_batch = 0, float _last_item = 0, int _count = 0, int _delta = 0):
                last_batch(_last_batch), last_item(_last_item), count(_count), delta(_delta){}
    };
protected:
	const double unit_time, time_threshold;
    const int circular_array_size;
    int circular_head;
    uint32_t *circular_array;
    Hash_table<uint32_t, Last_Time> hash_table_circular;

    void array_push(uint32_t new_key) {
		uint32_t old_key = circular_array[circular_head];
		if (old_key) {
			auto itr = hash_table_circular.find(old_key);
			if (!--(itr->second.count)) {
				hash_table_circular.erase(itr);
			}
		}
		circular_array[circular_head] = new_key;
		(++circular_head) %= circular_array_size;
	}

public:
	CalmSpaceSaving_Basic(double _time_threshold, double _unit_time,
		int memory, int count_threshold, int q_size,
		int _circular_array_size, int _time_or_size = 0): time_threshold(_time_threshold),
                                   unit_time(_unit_time),
                                   circular_array_size(_circular_array_size),
                                   CalmSpaceSavingTopK(memory - _circular_array_size*sizeof(int)
                                                              - _circular_array_size*sizeof(Hash_table<uint32_t, Last_Time>::Node),
                                                               count_threshold, q_size, _time_or_size),
                                   hash_table_circular(_circular_array_size){
        circular_array = new uint32_t [circular_array_size]{};
        circular_head = 0;
    }
	~CalmSpaceSaving_Basic() {
        delete [] circular_array;
	}

	bool insert(uint32_t key, float time, int bf_new) {
		auto itr = hash_table_circular.find(key);
		if (itr == hash_table_circular.end()) {
            if(!bf_new)
                return 0;
            array_push(key);
            hash_table_circular[key] = Last_Time(time, time, 1);
            return 1;
        }
        else{
            if(time - itr->second.last_item < time_threshold){
                if(itr->second.delta){
                    if(!time_or_size)
                        insert_calm({key, itr->second.delta-1}, {0, 100000.0L*time - 100000.0L*itr->second.last_item});
                    else  
                        insert_calm({key, itr->second.delta-1}, {0, 1});
                }
                itr->second.last_item = time;
                return 0;
            }
            itr->second.count ++;
            array_push(key);
            int16_t delta = (time - itr->second.last_batch) / unit_time;
            itr->second = Last_Time(time, time, itr->second.count, delta+1);
            if(!time_or_size)
                insert_calm({key, delta}, {1, 0});
            else 
                insert_calm({key, delta}, {1, 1});
        }
        return 1;
	}

};

#endif // _CALMSPACESAVING_H_