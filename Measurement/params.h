#ifndef _PERIODICBATCH_PARAMS_H_
#define _PERIODICBATCH_PARAMS_H_

#include <string>

inline std::string fileName;
inline int checkType, sketchName;
inline double BATCH_TIME, UNIT_TIME, loss_rate = 0.001;
inline int repeat_time = 1, TOPK_THRESHOLD = 200, BATCH_SIZE_LIMIT = 4, memory = 5e5;

#include <iostream>

static void printName(int sketchName, int checkType) {
    if (sketchName == 1) {
        std::cout << "Test Basic HyperCalm\n";
    } else if (sketchName == 2) {
        std::cout << "Test Bucketized HyperCalm\n";
    } 
    if (checkType == 1)
        printf("Check Delay.\n");
    else if(checkType == 2)
        printf("Check Loss.\n");
}

#endif
