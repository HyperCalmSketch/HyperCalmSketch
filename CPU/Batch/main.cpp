#include "params.h"

using namespace std;

#include <vector>
#include <cstdio>
#include <cassert>
#include <ctime>
#include <string>

#include "../HyperCalm/HyperBloomFilter.h"
#include "../ComparedAlgorithms/ClockSketch.h"
#include "../ComparedAlgorithms/SWAMP.h"
#include "../ComparedAlgorithms/TOBF.h"
#include "../ComparedAlgorithms/groundtruth.h"

using namespace groundtruth::type_info;

extern void ParseArgs(int argc, char** argv);
extern vector<pair<uint32_t, float>> load_data(const string& fileName);

template <typename Sketch>
vector<Index> insert_result(Sketch&& sketch, const vector<Record>& input) {
    vector<int> res;
    for (int i = 0; i < input.size(); ++i) {
        auto& [key, time] = input[i];
        if (sketch.insert(key, time)) {
            res.push_back(i);
        }
    }
	return res;
}

tuple<int, int> single_hit_test(
    const vector<Index>& results,
    const vector<Index>& objects,
    const vector<Index>& batches
) {
    int object_count = 0, correct_count = 0;
    int j = 0, k = 0;
    for (int i : results) {
        while (j + 1 < int(batches.size()) && batches[j] < i)
            ++j;
        if (j < int(batches.size()) && batches[j] == i) {
            ++correct_count;
        }
        while (k + 1 < int(objects.size()) && objects[k] < i)
            ++k;
        if (k < int(objects.size()) && objects[k] == i) {
            ++object_count;
        }
    }
    return make_tuple(object_count, correct_count);
}

void hit_test(const vector<Record>& input) {
    constexpr bool use_counter = false;
    groundtruth::adjust_params(input, BATCH_TIME, UNIT_TIME);
    groundtruth::item_count(input);
    auto [objects, batches] = groundtruth::batch(input, BATCH_TIME, BATCH_SIZE_LIMIT);
    printf("---------------------------------------------\n");
    printName(sketchName, checkName);
    uint64_t time_ns = 0;
    int object_count = 0, correct_count = 0, tot_our_size = 0;
    for (int t = 0; t < repeat_time; ++t) {
        timespec start_time, end_time;
        clock_gettime(CLOCK_MONOTONIC, &start_time);
        vector<Index> res;
        if (sketchName == 1)
            res = insert_result(HyperBloomFilter(memory, BATCH_TIME, t), input);
        else if (sketchName == 2)
            res = insert_result(ClockSketch<use_counter>(memory, BATCH_TIME, t), input);
        else if (sketchName == 3)
            res = insert_result(TOBF<use_counter>(memory, BATCH_TIME, 4, t), input);
        else if (sketchName == 4)
            res = insert_result(SWAMP<int, float, use_counter>(memory, BATCH_TIME), input);
        clock_gettime(CLOCK_MONOTONIC, &end_time);
        time_ns += (end_time.tv_sec - start_time.tv_sec) * uint64_t(1e9);
        time_ns += (end_time.tv_nsec - start_time.tv_nsec);
        tuple<int, int> test_result = single_hit_test(res, objects, batches);
        object_count += get<0>(test_result);
        correct_count += get<1>(test_result);
        tot_our_size += res.size();
    }
    printf("---------------------------------------------\n");
    auto recall = 1.0 * object_count / objects.size() / repeat_time;
    auto precision = 1.0 * correct_count / tot_our_size / repeat_time;
    printf("Hit test:\n");
    printf("Throughput:\t %f M/s\n", 1e3 * input.size() * repeat_time / time_ns);
    printf("Recall Rate:\t %f\n", recall);
    printf("Precision Rate:\t %f\n", precision);
    auto f1 = recall || precision ? 2 * recall * precision / (recall + precision) : 0.;
    printf("F1 Score:\t %f\n", f1);
}

//--------------------------------------------------------------------------------------------------------------------------

template <typename Sketch>
tuple<int, int> single_test(
    Sketch&& sketch,
    const vector<Record>& input,
    const map<ItemKey, vector<BatchTimeRange>>& item_batches
) {
    int correct_count = 0, report_count = 0;
    map<ItemKey, int> last_batch;
    for (int i = 0; i < input.size(); ++i) {
        auto& [key, time] = input[i];
        if (sketch.insert_cnt(key, time) + 1 != BATCH_SIZE_LIMIT)
            continue;
        ++report_count;
        if (!item_batches.count(key))
            continue;
        auto& batches = item_batches.at(key);
        int batch_id = last_batch[key];
        while (batch_id < int(batches.size()) && batches[batch_id].second < i)
            ++batch_id;
        last_batch[key] = batch_id;
        if (batch_id < int(batches.size()) && batches[batch_id].first <= i) {
            ++correct_count;
            ++last_batch[key]; // avoid duplicate
        }
    }
    return make_tuple(correct_count, report_count);
}

void large_hit_test(const vector<Record>& input) {
    timespec start_time, end_time;
    uint64_t time_ns;
    groundtruth::adjust_params(input, BATCH_TIME, UNIT_TIME);
    groundtruth::item_count(input);
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    auto item_batches = groundtruth::item_batches(input, BATCH_TIME, BATCH_SIZE_LIMIT);
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    time_ns = (end_time.tv_sec - start_time.tv_sec) * 1e9;
    time_ns += end_time.tv_nsec - start_time.tv_nsec;
    printf("groundtruth time:\t %f s\n", time_ns / 1e9);
    int total_count = 0;
    for (auto& [key, batches] : item_batches)
        total_count += int(batches.size());
    printf("---------------------------------------------\n");
    printName(sketchName, checkName);
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    int correct_count = 0, report_count = 0;
    for (int t = 0; t < repeat_time; ++t) {
        tuple<int, int> res;
        if (sketchName == 1)
            res = single_test(HyperBloomFilter(memory, BATCH_TIME, t), input, item_batches);
        else if (sketchName == 2)
            res = single_test(ClockSketch<true>(memory, BATCH_TIME, t), input, item_batches);
        else if (sketchName == 3)
            res = single_test(TOBF<true>(memory, BATCH_TIME, 4, t), input, item_batches);
        else if (sketchName == 4)
            res = single_test(SWAMP<int, float, true>(memory, BATCH_TIME), input, item_batches);
        else assert(false);
        correct_count += get<0>(res);
        report_count += get<1>(res);
    }
    printf("---------------------------------------------\n");
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    printf("Large hit test:\n");
    time_ns = (end_time.tv_sec - start_time.tv_sec) * 1e9;
    time_ns += end_time.tv_nsec - start_time.tv_nsec;
    printf("Throughput:\t %f M/s\n", 1e3 * input.size() * repeat_time / time_ns);
    auto recall = 1.0 * correct_count / total_count / repeat_time;
    auto precision = 1.0 * correct_count / report_count / repeat_time;
    printf("Recall:\t %f\nPrecision:\t %f\n", recall, precision);
    printf("F1 Score: %f\n", recall || precision ? 2 * recall * precision / (recall + precision) : 0.);
}

//----------------------------------------------------------------------------------------------------------------------------


template <typename Algorithm>
pair<double, double> single_size_test(
    const vector<Record>& input,
    const vector<int>& realtime_sizes,
    Algorithm&& algo
) {
    static const bool can_overflow = true;
    static const int cellbits = 2;
    static const int overflow_limit = (1 << cellbits) + can_overflow;
    double ARE = 0, AAE = 0;
    int acc_cnt = 0, overflow_cnt = 0, overflow_acc = 0;
    for (int i = 0; i < input.size(); ++i) {
        auto &[key, time] = input[i];
        int raw_real_size = realtime_sizes[i];
        int size = min(algo.insert_cnt(key, time) + 1, overflow_limit);
        int real_size = min(raw_real_size, overflow_limit);
        int diff = abs(size - real_size);
        double relative_error = double(diff) / real_size;
        AAE += diff;
        ARE += relative_error;
        if (!diff) ++acc_cnt;
        if (raw_real_size > overflow_limit) {
            ++overflow_cnt;
            if (!diff) ++overflow_acc;
        }
    }
    AAE /= input.size();
    ARE /= input.size();
    return {AAE, ARE};
}

void size_test(const vector<Record>& input) {
    groundtruth::item_count(input);
    groundtruth::adjust_params(input, BATCH_TIME, UNIT_TIME);
    auto realtime_sizes = groundtruth::realtime_size(input, BATCH_TIME);
    printf("---------------------------------------------\n");
    printName(sketchName, checkName);
    int seed = 0;
    double AAE = 0, ARE = 0;
    pair<double, double> tmp;
    for (int t = 0; t < repeat_time; ++t) {
        if (sketchName == 1)
            tmp = single_size_test(input, realtime_sizes, HyperBloomFilter(memory, BATCH_TIME, seed));
        else if (sketchName == 2)
            tmp = single_size_test(input, realtime_sizes, ClockSketch<true>(memory, BATCH_TIME, seed));
        else if (sketchName == 3)
            tmp = single_size_test(input, realtime_sizes, TOBF<true>(memory, BATCH_TIME, 4, seed));
        else if (sketchName == 4)
            tmp = single_size_test(input, realtime_sizes, SWAMP<int, float, true>(memory, BATCH_TIME));
        AAE += tmp.first;
        ARE += tmp.second;
    }
    printf("---------------------------------------------\n");
    printf("Realtime size test\n");
    printf("AAE:\t %.5lf\nARE:\t %.3lf\n", AAE/repeat_time, ARE/repeat_time);
    printf("---------------------------------------------\n");
}

int main(int argc, char** argv) {
	ParseArgs(argc, argv);
	printf("---------------------------------------------\n");
	auto input = load_data(fileName);
	printf("---------------------------------------------\n");
	if(checkName == 1)
		size_test(input);
	if(checkName == 2)
		hit_test(input);
	if(checkName == 3)
		large_hit_test(input);
	printf("---------------------------------------------\n");
}
