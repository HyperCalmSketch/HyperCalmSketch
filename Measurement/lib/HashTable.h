#ifndef _HASHTABLE_H_
#define _HASHTABLE_H_

#include <cassert>

int operator % (pair<uint32_t, uint16_t> A, int B){
    return (A.first + A.second) % B;
}

#define Mod(x) ((x%n+n)%n)
template<typename key_t, typename val_t>
class Hash_table{
    int n;
    int *head;
    int *pool,pool_n;
public:
    struct Node{
        int next;
        key_t first;
        val_t second;
    };
    Node *nodes;
    Hash_table(int _n = 30000000):n(_n),pool_n(_n){
        head = new int [n];
        nodes = new Node [n];
        pool = new int [n];
        for(int i=0;i<n;++i){
            head[i] = -1;
            pool[i] = i;
        }
    }
    ~Hash_table(){
        delete [] head;
        delete [] pool;
        delete [] nodes;
    }
    bool count(key_t key){
        for(int i = head[Mod(key)]; i!=-1; i=nodes[i].next)
        if(nodes[i].first==key)return 1;
        return 0;
    }
    val_t& operator [](key_t key){
        int key_mod_n = Mod(key);
        for(int i = head[key_mod_n]; i != -1; i = nodes[i].next)
        if(nodes[i].first == key)
            return nodes[i].second;
        assert(pool_n > 0);
        --pool_n;
        int i = pool[pool_n];
        nodes[i].next = head[key_mod_n];
        nodes[i].first = key;
        nodes[i].second = val_t();
        head[key_mod_n] = i;
        return nodes[i].second;
    }
    Node* find(key_t key){
        for(int i = head[Mod(key)]; i != -1; i = nodes[i].next)
        if(nodes[i].first == key)
            return nodes + i;
        return nodes + n;
    }
    Node* end(){
        return nodes + n;
    }
    void erase(Node *p){
        int p_i = p - nodes;
        pool[pool_n++] = p_i;
        int key_mod_n = Mod(p->first);
        int i = head[key_mod_n];
        if(i == p_i)
            head[key_mod_n] = p->next;
        else{
            while(nodes[i].next != p_i) i = nodes[i].next;
            nodes[i].next = p->next;
        }
    }
    void erase(key_t key){
        Node *p = find(key);
        if(p != end())
            erase(p);
    }
};
#undef Mod
#endif // _HASHTABLE_H_
