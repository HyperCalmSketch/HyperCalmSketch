#ifndef _SPACESAVINGUTILSTOPK_H_
#define _SPACESAVINGUTILSTOPK_H_

#include <cstdint>

struct Node
{
    uint32_t key;
    uint32_t val;
    Node * prev;
    Node * next;
    Node * parent;
    Node * next1;
};

/*
struct HashNode {
    uint32_t key;
    Node * p_node;
    HashNode * next;
    HashNode * prev;
};*/


#endif // _SPACESAVINGUTILSTOPK_H_