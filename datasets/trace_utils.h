#ifndef _TRACE_UTILS_H_
#define _TRACE_UTILS_H_

using namespace std;

#include "trace.h"

using Record = pair<uint32_t, float>;

vector<Record> load_large_data(const string& fileName);

vector<Record> load_data(const string& fileName) {
    if (fileName.substr(0, 2) == "[L") {
        return load_large_data(fileName);
    }
    if (fileName.back() == 'w')
        return load_raw_data(fileName.c_str());
    else if (fileName.back() == 't')
        return loadCAIDA(fileName.c_str());
    else
        return loadCRITEO(fileName.c_str());
}

vector<Record> load_large_data(const string& fileName) {
    vector<Record> input;
    int total_num = 60;
    if (fileName.find(']') == string::npos) {
        throw runtime_error("invalid file name");
    }
    if (auto nums = fileName.substr(2, fileName.find(']') - 2); !nums.empty()) {
        total_num = stoi(nums);
    }
    string fname = fileName.substr(fileName.find(']') + 1);
    char buf[100];
    double last_time = 0;
    for (int i = 0; i < total_num; ++i) {
        snprintf(buf, 100, fname.c_str(), i);
        auto single_data = load_data(buf);
        for (auto& [key, time] : single_data) {
            input.emplace_back(key, time + last_time);
        }
        last_time += single_data.back().second;
    }
    return input;
}

#endif // _TRACE_UTILS_H_
