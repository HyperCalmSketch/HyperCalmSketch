#include <cstdlib>
#include <cassert>
#include <ctime>
#include <iostream>
#include <vector>

using namespace std;

#include "../HyperCalm/bucketized/HyperCalm.h"
#include "../../ComparedAlgorithms/groundtruth.h"

using namespace groundtruth::type_info;

#include "params.h"

extern void ParseArgs(int argc, char** argv);
extern vector<pair<uint32_t, float>> load_data(const string& fileName);

template <bool use_counter>
void periodic_size_test_compress(
    const vector<Record>& input,
    vector<pair<PeriodicKey, int>>& ans
) {
    printName();
    sort(ans.begin(), ans.end());
    vector<pair<PeriodicKey, int>> our;
    int corret_count = 0,cc=0,pc=0;
    double sae = 0, sre = 0;
    uint64_t time_ns = 0;
    auto check = [&] (auto sketch) {
        timespec start, end;
        int k=0,i=0,n=input.size(), compress_extend_limit = 0;
        for (auto &[key, time] : input) {
            sketch.template insert_filter(key, time, BATCH_SIZE_LIMIT);
            if((compress_extend_limit++)==n/10){
                clock_gettime(CLOCK_MONOTONIC, &start);
                sketch.compress_hypercalm(R);
                clock_gettime(CLOCK_MONOTONIC, &end);
            }
        }
        our = sketch.get_top_k(TOPK_THRESHOLD);
        time_ns += (end.tv_sec - start.tv_sec) * 1e9 + (end.tv_nsec - start.tv_nsec);
        sort(our.begin(), our.end());
        int j = 0;
        for (auto &[key, freq] : our) {
            while (j + 1 < ans.size() && ans[j].first < key)
                ++j;
            if (j < ans.size() && ans[j].first == key) {
                ++corret_count;
                auto diff = abs(ans[j].second - freq);
                //cerr<<freq<<" "<<ans[j].second<<endl;
                sae += diff;
                sre += diff / double(ans[j].second);
            }
        }
    };
    for (int t = 0; t < repeat_time; ++t) 
        check(HyperCalm(BATCH_TIME, UNIT_TIME, memory, t));
    cout << "---------------------------------------------" << endl;
    cout << "Results with compress:" << endl;
    cout << "Average Speed:\t " << time_ns / 1e6 << " ms" << endl;
    cout << "Recall Rate:\t " << 1.0 * corret_count / ans.size() / repeat_time << endl;
    cout << "AAE:\t\t " << sae / corret_count << endl;
    cout << "ARE:\t\t " << sre / corret_count << endl;
}

template <bool use_counter>
void periodic_size_test_extend(
    const vector<Record>& input,
    vector<pair<PeriodicKey, int>>& ans
) {
    printName();
    sort(ans.begin(), ans.end());
    vector<pair<PeriodicKey, int>> our;
    int corret_count = 0,cc=0,pc=0;
    double sae = 0, sre = 0;
    uint64_t time_ns = 0;
    auto check = [&] (auto sketch) {
        timespec start, end;
        int k=0,i=0,n=input.size(), compress_extend_limit = 0;
        for (auto &[key, time] : input) {
            sketch.template insert_filter(key, time, BATCH_SIZE_LIMIT);
            if((compress_extend_limit++)==n/10){
                clock_gettime(CLOCK_MONOTONIC, &start);
                sketch.extend_hypercalm(R);
                clock_gettime(CLOCK_MONOTONIC, &end);
            }
        }
        our = sketch.get_top_k(TOPK_THRESHOLD);
        time_ns += (end.tv_sec - start.tv_sec) * 1e9 + (end.tv_nsec - start.tv_nsec);
        sort(our.begin(), our.end());
        int j = 0;
        for (auto &[key, freq] : our) {
            while (j + 1 < ans.size() && ans[j].first < key)
                ++j;
            if (j < ans.size() && ans[j].first == key) {
                ++corret_count;
                auto diff = abs(ans[j].second - freq);
                //cerr<<freq<<" "<<ans[j].second<<endl;
                sae += diff;
                sre += diff / double(ans[j].second);
            }
        }
    };
    for (int t = 0; t < repeat_time; ++t) 
        check(HyperCalm(BATCH_TIME, UNIT_TIME, memory, t));
    cout << "---------------------------------------------" << endl;
    cout << "Results with compress:" << endl;
    cout << "Average Speed:\t " << time_ns / 1e6 << " ms" << endl;
    cout << "Recall Rate:\t " << 1.0 * corret_count / ans.size() / repeat_time << endl;
    cout << "AAE:\t\t " << sae / corret_count << endl;
    cout << "ARE:\t\t " << sre / corret_count << endl;
}

void size_test(const vector<pair<uint32_t, float>>& input) {
    groundtruth::item_count(input);
    groundtruth::adjust_params(input, BATCH_TIME, UNIT_TIME);
    vector<int> batche = groundtruth::batch(input, BATCH_TIME, BATCH_SIZE_LIMIT).first;
    auto ans = groundtruth::topk(input, batche, UNIT_TIME, TOPK_THRESHOLD);
    printf("BATCH_TIME = %f, UNIT_TIME = %f\n", BATCH_TIME, UNIT_TIME);
    if (memory > 1000)
        cout << "Total Memory: " << memory / 1000. << " KB";
    else
        cout << "Total Memory: " << memory << " B";
    cout << ", Top K: " << TOPK_THRESHOLD << '\n';
    cout << "---------------------------------------------" << '\n';
    if(variable == 1)
        periodic_size_test_compress<true>(input, ans);
    else 
        periodic_size_test_extend<true>(input, ans);
    // cout << "---------------------------------------------" << endl;
    // periodic_size_test<false>(input, ans);
    cout << "---------------------------------------------" << endl;
}


int main(int argc, char** argv) {
	ParseArgs(argc, argv);
	printf("---------------------------------------------\n");
	auto input = load_data(fileName);
	printf("---------------------------------------------\n");
	size_test(input);
	printf("---------------------------------------------\n");
}