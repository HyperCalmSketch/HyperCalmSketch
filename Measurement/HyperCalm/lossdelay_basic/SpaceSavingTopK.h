#ifndef _SPACESAVINGTOPK_H_
#define _SPACESAVINGTOPK_H_

#include "../../lib/HashTable.h"

#define tail_node nodes[0].prev

template<class Ty1,class Ty2,class Ty3,class Ty4> 
inline const pair<Ty1,Ty2> operator+(const pair<Ty1, Ty2>&p1, const pair<Ty3, Ty4>&p2)
{
    pair<Ty1, Ty2> ret;
    ret.first = p1.first + p2.first;
    ret.second = p1.second + p2.second;
    return ret;
}

class SpaceSavingTopK
{
    struct Node
    {
        pair<uint32_t, uint16_t> key;
        pair<uint32_t, float> val;
        Node * prev;
        Node * next;
        Node * parent;
        Node * next1;
    };
protected:
    int now_element, capacity, time_or_size;
    Node *nodes;
    Hash_table<pair<uint32_t, uint16_t>, Node *> hash_table;


    void append_new_key1(pair<uint32_t, uint16_t> key, pair<char, float> freq) {
        uint16_t idx = ++now_element; // we use 0 to represent header
        nodes[idx].key = key;
        nodes[idx].val = {0,0};
        hash_table[key] = nodes + idx;

        // append to tail
        nodes[idx].prev = tail_node;
        tail_node->next = nodes + idx;
        nodes[idx].next = nodes;
        nodes[idx].parent = nodes + idx;
        tail_node = &nodes[idx];
        add_counter(tail_node, freq);
    }

    void add_counter(Node * my, pair<char, float> freq) {
        //if(now_element == capacity && my->val == tail_node->val)
        //    freq++;

        if (my->parent == my && my->next->val.first == my->val.first) {
            Node * p = my->next, * nt = my->next;
            while (p && p->val.first == my->val.first) {
                p->parent = nt;
                p = p->next;
            }
        }
        my->val = my->val + freq;
        pair<uint32_t, float> now_freq = my->val;
        Node * prev_node = my->prev;

        if (prev_node->val > now_freq) {
            return;
        }

        Node * next_node = my->next;

        // make next and prev connect
        prev_node->next = my->next;
        next_node->prev = my->prev;

        while (prev_node->val < now_freq) {
            prev_node = prev_node->parent->prev;
        }

        next_node = prev_node->next;

        my->next = prev_node->next;
        prev_node->next = my;

        my->prev = next_node->prev;
        next_node->prev = my;

        my->parent = (prev_node->val.first == my->val.first) ? prev_node->parent : my;
    }
public:
    SpaceSavingTopK(int memory, int _time_or_size) : 
            capacity(memory/(sizeof(Node)+sizeof(Hash_table<pair<uint32_t, uint16_t>, Node*>::Node))-1),
            now_element(0), hash_table(capacity+1), time_or_size(_time_or_size){
        nodes = new Node [capacity+1];
        memset(nodes, 0, (capacity + 1) * sizeof(Node));
        now_element = 0;
        nodes[0].val = {-1, 0};
        nodes[0].parent = nodes;
        tail_node = nodes;
    }
    ~SpaceSavingTopK(){
        delete [] nodes;
    }

    vector<pair<pair<int, int16_t>,pair<int, double>>> get_top_k(int k) const {
        vector<pair<pair<int, int16_t>,pair<int, double>>> ans;

        Node * idx = nodes[0].next;
        for(int i = 0; i < k && i < capacity && i < now_element; ++i){
            ans.push_back( {idx->key, idx->val} );
            idx = idx->next; 
        }
        return ans; 
    }
};
#undef tail_node

#endif //_SPACESAVINGTOPK_H_