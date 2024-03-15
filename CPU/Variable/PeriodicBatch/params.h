#ifndef _PERIODICBATCH_PARAMS_H_
#define _PERIODICBATCH_PARAMS_H_

#include <string>

inline std::string fileName;
inline int variable;
inline double BATCH_TIME, UNIT_TIME;
inline bool verbose = false;
inline int repeat_time = 1, TOPK_THRESHOLD = 200, BATCH_SIZE_LIMIT = 1, memory = 5e5, R = 0;

#include <iostream>

static void printName() {
    std::cout << "Test HyperCalm_Variable\n";
}

#endif
