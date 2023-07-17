#ifndef _LFU_H_
#define _LFU_H_

class LFU {
public:
    typedef typename multimap<int, string>::iterator lfuIterator;
    unordered_map<string,double>wait;
    LFU() {}

    LFU(int _K) : K(_K) {}

    ~LFU() {}

    void insert(const string &key,int tr=1,double ti=-1,double bao=-1) {
        auto elem = lfuStorage.find(key);
        if (elem != lfuStorage.end()) {
            auto updated_elem = make_pair(elem->second->first + tr, key);
            frequencyStorage.erase(elem->second);
            lfuStorage[key] = frequencyStorage.emplace_hint(
                    frequencyStorage.cend(), move(updated_elem));
        } else {
            if (lfuStorage.size() >= K) {
                auto ele=frequencyStorage.begin();
                if(ti>0)
                while(wait.count(ele->second)&&(ti<wait[ele->second])){
                    ele++;
                }
                frequencyStorage.erase(ele);
                lfuStorage.erase(ele->second);
                if(wait.count(ele->second))wait.erase(ele->second);
            }
            auto updated_elem = make_pair(1, key);
            lfuStorage[key] = frequencyStorage.emplace_hint(
                    frequencyStorage.cend(), move(updated_elem));
        }
        if(bao>0){
            wait[key]=bao;
        }
    }

    int query(const string &key) {
        auto elem = lfuStorage.find(key);
        if (elem != lfuStorage.end()) {
            return elem->second->first;
        }
        return -1;
    }

    void init(int _K) {
        K = _K;
    }

    int size() {
        return lfuStorage.size();
    }

private:
    multimap<int, string> frequencyStorage;
    unordered_map<string, lfuIterator> lfuStorage;
    int K;
};

#endif //_LFU_H_
