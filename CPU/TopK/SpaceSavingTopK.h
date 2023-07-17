#ifndef _SPACESAVINGTOPK_H_
#define _SPACESAVINGTOPK_H_

#include "../lib/HashTable.h"
#include "SpaceSavingUtilsTopK.h"

#define tail_node nodes[0].prev

class SpaceSavingTopK
{
protected:
    int now_element, capacity;
    Node *nodes;
    Hash_table<uint32_t, Node *> hash_table;


    void append_new_key1(uint32_t key, int freq) {
        uint16_t idx = ++now_element; // we use 0 to represent header
        nodes[idx].key = key;
        nodes[idx].val = 0;
        hash_table[key] = nodes + idx;

        // append to tail
        nodes[idx].prev = tail_node;
        tail_node->next = nodes + idx;
        nodes[idx].next = nodes;
        nodes[idx].parent = nodes + idx;
        tail_node = &nodes[idx];
        add_counter(tail_node, freq);
    }
    void append_new_key2(uint32_t key, int freq) {
        uint32_t old_key = tail_node->key;
        hash_table.erase(old_key);
        tail_node->key = key;
        hash_table[key] = tail_node;
        add_counter(tail_node, freq);
    }
    void append_new_key(uint32_t key, int freq) {
        if (now_element < capacity) {
            append_new_key1(key, freq);
        } else {
            append_new_key2(key, freq);
        }
    }

    void add_counter(Node * my, int freq) {
        //if(now_element == capacity && my->val == tail_node->val)
        //    freq++;

        if (my->parent == my && my->next->val == my->val) {
            Node * p = my->next, * nt = my->next;
            while (p && p->val == my->val) {
                p->parent = nt;
                p = p->next;
            }
        }

        my->val += freq;
        int now_freq = my->val;
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

        my->parent = (prev_node->val == my->val) ? prev_node->parent : my;
    }
public:
    SpaceSavingTopK(int memory) : now_element(0),
            capacity(memory/(sizeof(Node)+sizeof(Hash_table<uint32_t, Node*>::Node))-1)
            ,hash_table(capacity+1){
        nodes = new Node [capacity+1];
        memset(nodes, 0, (capacity + 1) * sizeof(Node));
        now_element = 0;
        nodes[0].val = -1;
        nodes[0].parent = nodes;
        tail_node = nodes;
    }
    ~SpaceSavingTopK(){
        delete [] nodes;
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
            ans.push_back( {idx->key, idx->val} );
            idx = idx->next; 
        }
        return ans; 
    }
};
#undef tail_node

#endif //_SPACESAVINGTOPK_H_