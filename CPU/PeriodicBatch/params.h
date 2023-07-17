#ifndef _PERIODICBATCH_PARAMS_H_
#define _PERIODICBATCH_PARAMS_H_

#include <string>

inline std::string fileName;
inline int sketchName;
inline double BATCH_TIME, UNIT_TIME;
inline bool verbose = false;
inline int repeat_time = 1, TOPK_THRESHOLD = 200, BATCH_SIZE_LIMIT = 1, memory = 5e5;

#include <iostream>

static void printName(int sketchName) {
    if (sketchName == 1) {
        std::cout << "Test HyperCalm\n";
    } else if (sketchName == 2) {
        std::cout << "Test HyperCalm_Old\n";
    } else if (sketchName == 3) {
        std::cout << "Test Clock+USS\n";
    }
}

#endif
