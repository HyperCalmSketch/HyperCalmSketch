#include "params.h"

using namespace std;

#include <vector>
#include <cstdio>
#include <cassert>
#include <ctime>
#include <string>
#include "params.h"

#include "../HyperCalm/HyperBloomFilterTest.h"
#include "../../ComparedAlgorithms/groundtruth.h"

using namespace groundtruth::type_info;

extern void ParseArgs(int argc, char** argv);
extern vector<pair<uint32_t, float>> load_data(const string& fileName);

template <typename Sketch>
pair<vector<Index>,uint64_t> insert_result(Sketch&& sketch, const vector<Record>& input, int veriable, int r) {
    vector<int> res;
    uint64_t time_ns = 0;
    timespec start_time, end_time;
    
    for (int i = 0; i < input.size(); ++i) {
        auto& [key, time] = input[i];
        if (sketch.insert(key, time)) {
            res.push_back(i);
        }
        if(i==input.size()/10){
            clock_gettime(CLOCK_MONOTONIC, &start_time); 
            if(veriable==1) 
                while(r--)
                    sketch.compress();
            else
                while(r--)
                    sketch.extend();
            clock_gettime(CLOCK_MONOTONIC, &end_time);
        }
    }
   
    time_ns += (end_time.tv_sec - start_time.tv_sec) * uint64_t(1e9);
    time_ns += (end_time.tv_nsec - start_time.tv_nsec);
	return make_pair(res, time_ns);
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
    uint64_t time_ns = 0;
    int object_count = 0, correct_count = 0, tot_our_size = 0;
    for (int t = 0; t < repeat_time; ++t) {
        
        pair<vector<Index>,uint64_t> res;
        res = insert_result(HyperBF::TestModeHyperBF(memory, BATCH_TIME, t), input, veriable, R);
        time_ns += res.second;
        tuple<int, int> test_result = single_hit_test(res.first, objects, batches);
        object_count += get<0>(test_result);
        correct_count += get<1>(test_result);
        tot_our_size += res.first.size();
    }
    printf("---------------------------------------------\n");
    auto recall = 1.0 * object_count / objects.size() / repeat_time;
    auto precision = 1.0 * correct_count / tot_our_size / repeat_time;
    printf("Hit test:\n");
    printf("Throughput:\t %f ms\n", time_ns/1e6/repeat_time);
    printf("Recall Rate:\t %f\n", recall);
    printf("Precision Rate:\t %f\n", precision);
    auto f1 = recall || precision ? 2 * recall * precision / (recall + precision) : 0.;
    printf("F1 Score:\t %f\n", f1);
}

//-------------------------------------------------------------------------------------------------------------------------
int main(int argc, char** argv) {
	ParseArgs(argc, argv);
	printf("---------------------------------------------\n");
	auto input = load_data(fileName);
	printf("---------------------------------------------\n");
	hit_test(input);
	printf("---------------------------------------------\n");
}
