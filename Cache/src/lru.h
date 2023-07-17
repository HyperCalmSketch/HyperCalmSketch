#ifndef _LRU_H_
#define _LRU_H_

class LRU {
public:
    typedef typename std::pair<string, int> key_value_pair_t;
    typedef typename std::list<key_value_pair_t>::iterator list_iterator_t;

    LRU() {}

    LRU(int _K) : K(_K) {}

    void insert(const string &key, const int &value) {
        auto it = _cache_items_map.find(key);
        _cache_items_list.push_front(key_value_pair_t(key, value));
        if (it != _cache_items_map.end()) {
            _cache_items_list.erase(it->second);
            _cache_items_map.erase(it);
        }
        _cache_items_map[key] = _cache_items_list.begin();

        if (_cache_items_map.size() > K) {
            auto last = _cache_items_list.end();
            last--;
            _cache_items_map.erase(last->first);
            _cache_items_list.pop_back();
        }
    }

    int query(const string &key) {
        auto it = _cache_items_map.find(key);
        if (it == _cache_items_map.end()) {
            return -1;
        } else {
            _cache_items_list.splice(_cache_items_list.begin(), _cache_items_list, it->second);
            return it->second->second;
        }
    }

    void init(int _K) {
        K = _K;
    }

    bool exists(const string &key) const {
        return _cache_items_map.find(key) != _cache_items_map.end();
    }

    int size() const {
        return _cache_items_map.size();
    }

private:
    std::list<key_value_pair_t> _cache_items_list;
    std::unordered_map<string, list_iterator_t> _cache_items_map;
    int K;
};

#endif //_LRU_H_
