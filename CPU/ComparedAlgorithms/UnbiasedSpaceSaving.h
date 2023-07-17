#ifndef _UNBIASEDSPACESAVING_H_
#define _UNBIASEDSPACESAVING_H_

#include <cstdint>
#include <cstdlib>
#include <vector>
#include <random>

#include "../lib/HashTable.h"

using namespace std;

#define tail_node SS_nodes[0].val_prev

class UnbiasedSpaceSaving {
	struct SS_Node {
		uint32_t key;
		int16_t delta;
		uint32_t val;
		SS_Node* val_prev;
		SS_Node* val_next;
		SS_Node* val_parent;
		SS_Node* key_next;
	};
	struct Info {
		float last_batch_time, last_item_time;
		int count; //count appear in circular array or SS
		SS_Node* first_SS_node;
		Info(float _last_batch_time = 0, float _last_item_time = 0, int _count = 1,
			SS_Node* _first_SS_node = nullptr): last_batch_time(_last_batch_time), last_item_time(_last_item_time), count(_count), first_SS_node(_first_SS_node) {}
	};

	int now_element, capacity;
	SS_Node* SS_nodes;
	Hash_table<uint32_t, Info> hash_table;
	mt19937 rng;

	uint32_t* circular_array;
	int circular_array_head;
	const int circular_array_size;

	double BATCH_TIME_THRESHOLD, UNIT_TIME;

	void array_push(uint32_t new_key) {
		uint32_t old_key = circular_array[circular_array_head];
		if (old_key) {
			auto itr = hash_table.find(old_key);
			if (!--(itr->second.count)) {
				hash_table.erase(itr);
			}
		}
		circular_array[circular_array_head] = new_key;
		(++circular_array_head) %= circular_array_size;
	}

	void append_new_key(uint32_t key, int16_t delta, float time, int freq, Info& info) {
		if (now_element < capacity) {
			info.count++;
			int idx = ++now_element; // we use 0 to represent header
			auto np = SS_nodes + idx;
			np->key = key;
			np->delta = delta;
			np->val = 0;
			np->key_next = info.first_SS_node;
			info.first_SS_node = np;

			// append to tail
			np->val_prev = tail_node;
			tail_node->val_next = np;
			np->val_next = SS_nodes;
			np->val_parent = np;
			tail_node = np;
			add_counter(tail_node, freq);
		} else {
			if (rng() % (tail_node->val + 1) == 0) {
				info.count++;
				replace_new_key(key, delta, time);
				tail_node->key_next = info.first_SS_node;
				info.first_SS_node = tail_node;
			}
			add_counter(tail_node, freq);
		}
	}

	void replace_new_key(uint32_t key, int16_t delta, float time) {
		uint32_t old_key = tail_node->key;
		auto it = hash_table.find(old_key);
		if (it == hash_table.end()) {
			fprintf(stderr, "LINE %d: hash_table.find(old_key) == hash_table.end()\n",
				__LINE__);
			exit(0);
		}
		if (!--(it->second.count)) {
			hash_table.erase(it);
		} else {
			SS_Node* p = it->second.first_SS_node;
			if (p == tail_node) {
				it->second.first_SS_node = tail_node->key_next;
			} else {
				while (p->key_next != tail_node) p = p->key_next;
				p->key_next = tail_node->key_next;
			}
		}
		tail_node->key = key;
		tail_node->delta = delta;
	}

	void add_counter(SS_Node* my, int freq) {
		if (my->val_parent == my && my->val_next->val == my->val) {
			SS_Node *p = my->val_next, *nt = my->val_next;
			while (p && p->val == my->val) {
				p->val_parent = nt;
				p = p->val_next;
			}
		}

		my->val += freq;
		SS_Node* prev_node = my->val_prev;

		if (prev_node->val > my->val) {
			return;
		}

		SS_Node* next_node = my->val_next;

		// make next and prev connect
		prev_node->val_next = my->val_next;
		next_node->val_prev = my->val_prev;

		while (prev_node->val < my->val) {
			prev_node = prev_node->val_parent->val_prev;
		}

		next_node = prev_node->val_next;

		my->val_next = prev_node->val_next;
		prev_node->val_next = my;

		my->val_prev = next_node->val_prev;
		next_node->val_prev = my;

		my->val_parent = (prev_node->val == my->val) ? prev_node->val_parent : my;
	}

public:
	UnbiasedSpaceSaving(double batch_time, double unit_time,
		int memory, int _circular_array_size, int seed): now_element(0),
														 capacity((memory - _circular_array_size * (sizeof(uint32_t) + sizeof(Hash_table<uint32_t, Info>::Node))) / (sizeof(SS_Node) + sizeof(int) * 2 + sizeof(Hash_table<uint32_t, Info>::Node)) - 1),
														 hash_table(capacity + _circular_array_size + 5),
														 rng(seed),
														 circular_array_size(_circular_array_size),
														 BATCH_TIME_THRESHOLD(batch_time),
														 UNIT_TIME(unit_time) {
		SS_nodes = new SS_Node[capacity + 1];
		memset(SS_nodes, 0, (capacity + 1) * sizeof(SS_Node));
		now_element = 0;
		SS_nodes[0].val = -1;
		SS_nodes[0].val_parent = SS_nodes;
		tail_node = SS_nodes;

		circular_array = new uint32_t[circular_array_size];
		memset(circular_array, 0, circular_array_size * sizeof(uint32_t));
		circular_array_head = 0;
	}
	~UnbiasedSpaceSaving() {
		delete[] SS_nodes;
		delete[] circular_array;
	}

	bool insert(uint32_t key, float time, bool bf_new, int freq = 1) {
		auto itr = hash_table.find(key);
		if (itr == hash_table.end()) {
			// key not found
			if (!bf_new) return 0;
			array_push(key);
			hash_table[key] = Info(time, time);
			if (now_element < capacity)
				append_new_key(key, -1, time, freq, hash_table[key]);
		} else {
			itr->second.count++; //array count++
			// key found
			array_push(key);
			if (time - itr->second.last_item_time < BATCH_TIME_THRESHOLD) {
				itr->second.last_item_time = time;
				return 0;
			}
			int16_t delta = (time - itr->second.last_batch_time) / UNIT_TIME;
			itr->second.last_item_time = itr->second.last_batch_time = time;
			for (SS_Node* p = itr->second.first_SS_node; p; p = p->key_next)
				if (p->delta == delta) {
					add_counter(p, freq);
					return 1;
				}
			append_new_key(key, delta, time, freq, itr->second);
		}
		return 1;
	}


	vector<pair<pair<int, int16_t>, int>> get_top_k(int k) {
		vector<pair<pair<int, int16_t>, int>> ans(k);

		SS_Node* idx = SS_nodes[0].val_next;
		int i;
		int mn_v = 1e9;
		for (i = 0; i < k && i < capacity && i < now_element; ++i) {
			ans[i] = { { idx->key, idx->delta }, idx->val };
			idx = idx->val_next;
			mn_v = min(mn_v, (int)idx->val);
		}
		for (; i < k; ++i)
			ans[i] = {};
		return ans;
	}
};
#undef tail_node

#endif //_UNBIASEDSPACESAVING_H_