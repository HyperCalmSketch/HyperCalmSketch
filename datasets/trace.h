#ifndef _TRACE_H_
#define _TRACE_H_

#include <cinttypes>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <vector>
#include <utility>

using Record = std::pair<uint32_t, float>;

std::vector<Record> loadCAIDA(const char *filename = "./CAIDA.dat") {
    printf("Open %s \n", filename);
    FILE *pf = fopen(filename, "rb");
    if (!pf) {
        printf("%s not found!\n", filename);
        exit(-1);
    }

    std::vector<Record> vec;
    double ftime = -1;
    char trace[30];
    while (fread(trace, 1, 21, pf)) {
        uint32_t tkey = *(uint32_t *)(trace);
        double ttime = *(double *)(trace + 13);
        if (ftime < 0)
            ftime = ttime;
        vec.emplace_back(tkey, ttime - ftime);
    }
    fclose(pf);
    return vec;
}

std::vector<Record> loadCRITEO(const char *filename = "./CRITEO.log") {
    printf("Open %s \n", filename);
    FILE *pf = fopen(filename, "rb");
    if (!pf) {
        printf("%s not found!\n", filename);
        exit(-1);
    }

    std::vector<Record> vec;
    char trace[40];
    int ttime = 0;
    while (fscanf(pf, "%s", trace) != EOF) {
        uint64_t tkey;
        sscanf(trace + 10, "%" SCNu64, &tkey);
        vec.push_back(std::pair<uint32_t, float>(tkey, ++ttime));
    }
    fclose(pf);
    return vec;
}

std::vector<Record> load_raw_data(const char* fileName) {
    using namespace std;
    cout << "loading raw data from " << fileName << endl;
    char buf[20];
	uint32_t key;
	float time;
	long long cnt = 0;
	fstream fin(fileName, ios::binary | ios::in);
    vector<Record> input;
	while (fin.read(buf, sizeof(uint32_t) + sizeof(float))) {
        key = *(uint32_t*)buf;
        time = *(float*)(buf + sizeof(uint32_t));
        input.emplace_back(key, time);
		if (++cnt % 100000000 == 0)
            cerr << cnt / 100000000 << "00M" << endl;
	}
    return input;
}
#endif // _TRACE_H_
