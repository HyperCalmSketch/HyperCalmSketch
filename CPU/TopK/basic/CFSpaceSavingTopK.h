#ifndef _CFSPACESAVINGTOPK_H_
#define _CFSPACESAVINGTOPK_H_

#include "SpaceSavingTopK.h"
#include <random>

#define tail_node nodes[0].prev
#define HASHNUM 4
#define Modxy(x, y) (((x)%(y)+(y))%(y))

class CFSpaceSavingTopK : public SpaceSavingTopK
{
    const int d1 = HASHNUM, d2 = HASHNUM;
    const int L1 = 15, L2 = 255, L = L1 + L2;
    int seed_low[HASHNUM];
    int seed_high[HASHNUM];
    uint8_t *Low;
    uint8_t *High;
    const int n_low, n_high;
    void append_new_key(uint32_t key, int freq) {
        uint8_t min_low = L1;
        for(int i=0; i<d1; i++){
            int h = Modxy(1ll*key*(i+1) + seed_low[i], n_low);
            uint8_t count = h&1 ? (Low[h/2]&L1) : ((Low[h/2]>>4)&L1);
            min_low = min(min_low, count);
        }
        if(min_low != L1){
            for(int i=0; i<d1; i++){
                int h = Modxy(1ll*key*(i+1) + seed_low[i], n_low);
                if((h&1) && (Low[h/2]&L1) == min_low)
                    Low[h/2] ++;
                if(!(h&1) && ((Low[h/2]>>4)&L1) == min_low)
                    Low[h/2] += 16;
            }
        }
        else{
            uint8_t min_high = L2;
            for(int i=0; i<d2; i++){
                int h = Modxy(1ll*key*(i+1) + seed_high[i], n_high);
                uint8_t count = High[h];
                min_high = min(min_high, count);
            }
            if(min_high != L2)
                for(int i=0; i<d2; i++){
                    int h = Modxy(1ll*key*(i+1) + seed_high[i], n_high);
                    if(High[h] == min_high)
                        High[h] ++;
                }
            else {
                if (now_element < capacity) 
                    append_new_key1(key, freq);
                else{
                    
                    uint32_t old_key = tail_node->key;
                    hash_table.erase(old_key);
                    tail_node->key = key;
                    hash_table[key] = tail_node;
                    add_counter(tail_node, freq);
                }
            }
        }
        return;
    }

public:
    CFSpaceSavingTopK(int memory, int memory2) : 
        SpaceSavingTopK(memory - memory2), n_low(memory2/2/sizeof(uint8_t)*2), n_high(memory2/2/sizeof(uint8_t)){
        Low = new uint8_t[n_low/2];
        High = new uint8_t[n_high];
        mt19937 rng(123);
		for (int i = 0; i <= d1; ++i) {
			seed_low[i] = rng();
            seed_high[i] = rng();
		}
    }
    ~CFSpaceSavingTopK(){
        delete [] Low;
        delete [] High;
    }

    void insert(uint32_t key, int freq = 1)
    {
        auto itr = hash_table.find(key);
        if (itr == hash_table.end()) {
            // key not found
            append_new_key(key, freq);
        } else {
            // key found
            add_counter(itr->second, freq);
        }
    }

    vector<pair<int,int>> get_top_k(int k) {
        vector<pair<int,int>> ans;

        Node * idx = nodes[0].next;
        for(int i = 0; i < k && i < capacity && i < now_element; ++i){
            ans.push_back( {idx->key, idx->val + L} );
            idx = idx->next; 
        }
        return ans; 
    }
};
#undef tail_node

#endif //_UNBIASEDSPACESAVINGTOPK_H_