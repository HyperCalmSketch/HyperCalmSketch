#include <string.h>
#include <cstdint>
#include <cstdlib>
#include <vector>
using namespace std;

#include <boost/program_options.hpp>

#include "../datasets/trace.h"
#include "basic/CalmSpaceSavingTopK.h"
#include "basic/UnbiasedSpaceSavingTopK.h"
#include "basic/SpaceSavingTopK.h"
#include "basic/CFSpaceSavingTopK.h"
#include "bucketized/CalmSpaceSavingTopK_Buckets.h"
#include "bucketized/SpaceSavingTopK_Buckets.h"
#include "groundtruthTopK.h"

using namespace boost::program_options;

string fileName;
int sketchName;
int repeat_time = 1, TOPK_THERSHOLD = 200, memory = 1e5, use_simd = 0;

void ParseArgs(int argc, char** argv) {
	options_description opts("Options");
	opts.add_options()
		("fileName,f", value<string>()->required(), "file name")
		("sketchName,s", value<int>()->required(), "sketch name")
		("time,t", value<int>()->required(), "repeat time")
		("topk,k", value<int>()->required(), "topk")
		("memory,m", value<int>()->required(), "memory")
		("use_simd,u", value<int>()->required(), "use_simd");
	variables_map vm;

	store(parse_command_line(argc, argv, opts), vm);
	if (vm.count("fileName")) {
		fileName = vm["fileName"].as<string>();
	} else {
		printf("please use -f to specify the path of dataset.\n");
		exit(0);
	}
	if (vm.count("sketchName")) {
		sketchName = vm["sketchName"].as<int>();
		if (sketchName < 1 || sketchName > 5) {
			printf("sketchName < 1 || sketchName > 5\n");
			exit(0);
		}
	} else {
		printf("please use -s to specify the name of sketch.\n");
		exit(0);
	}
	if (vm.count("time"))
		repeat_time = vm["time"].as<int>();
	if (vm.count("topk"))
		TOPK_THERSHOLD = vm["topk"].as<int>();
	if (vm.count("memory"))
		memory = vm["memory"].as<int>();
	if (vm.count("use_simd"))
		use_simd = vm["use_simd"].as<int>();
}

int main(int argc, char** argv) {
	ParseArgs(argc, argv);
	printf("---------------------------------------------\n");
	vector<pair<uint32_t, float>> input;
	if (fileName.back() == 't')
		input = loadCAIDA(fileName.c_str());
	else
		input = loadCRITEO(fileName.c_str());
	auto ans = groundtruth(input, TOPK_THERSHOLD);
	sort(ans.begin(), ans.end());
	printf("---------------------------------------------\n");
	if (sketchName == 1) {
		puts("Test Bucketized CalmSS");
	} else if (sketchName == 2) {
		puts("Test Bucketized SS");
	} else if (sketchName == 3){
		puts("Test Basic CALMSS");
	} else if (sketchName == 4){
		puts("Test SS");
	} else if (sketchName == 5){
		puts("Test USS");
	} else if (sketchName == 6){
		puts("Test CF+SS");
	}
	int corret_count = 0;
	double sae = 0, sre = 0;
	auto check_time = [&](auto sketch) {
		for (int i = 0; i < (int)input.size(); ++i) {
			auto pr = input[i];
			auto tkey = pr.first;
			auto ttime = pr.second;
			sketch.insert(tkey, 1);
		}
	};
	auto check = [&](auto sketch) {
		for (int i = 0; i < (int)input.size(); ++i) {
			auto pr = input[i];
			auto tkey = pr.first;
			auto ttime = pr.second;
			sketch.insert(tkey, 1);
		}
		auto our = sketch.get_top_k(TOPK_THERSHOLD);
		sort(our.begin(), our.end());
		int j = 0;
		for (auto x: our) {
			while (j + 1 < int(ans.size()) && ans[j].first < x.first)
				++j;
			if (j < int(ans.size()) && ans[j].first == x.first) {
				++corret_count;
				sae += abs(ans[j].second - x.second);
				sre += abs(ans[j].second - x.second) / double(ans[j].second);
			}
		}
	};
	timespec start_time, end_time;
	clock_gettime(CLOCK_MONOTONIC, &start_time);
	for (int t = 0; t < repeat_time; ++t) {
		if (sketchName == 1)
			check_time(CalmSpaceSavingTopK_Buckets(memory, memory / 11, use_simd));
		else if (sketchName == 2)
			check_time(SpaceSavingTopK_Buckets(memory, use_simd));
		else if (sketchName == 3)
			check_time(CalmSpaceSavingTopK(memory, 3, memory / 1000));
		else if (sketchName == 4)
			check_time(SpaceSavingTopK(memory));
		else if (sketchName == 5)
			check_time(UnbiasedSpaceSavingTopK(memory));
		else if (sketchName == 6)
			check_time(CFSpaceSavingTopK(memory, memory / 7));
	}
	clock_gettime(CLOCK_MONOTONIC, &end_time);
	uint64_t time_ns = uint64_t(end_time.tv_sec - start_time.tv_sec) * 1000000000 + (end_time.tv_nsec - start_time.tv_nsec);
	for (int t = 0; t < repeat_time; ++t) {
		if (sketchName == 1)
			check(CalmSpaceSavingTopK_Buckets(memory, memory / 11, use_simd));
		else if (sketchName == 2)
			check(SpaceSavingTopK_Buckets(memory, use_simd));
		else if (sketchName == 3)
			check(CalmSpaceSavingTopK(memory, 3, memory / 1000));
		else if (sketchName == 4)
			check(SpaceSavingTopK(memory));
		else if (sketchName == 5)
			check(UnbiasedSpaceSavingTopK(memory));
		else if (sketchName == 6)
			check(CFSpaceSavingTopK(memory, memory / 7));
	}
	printf("---------------------------------------------\n");
	printf("Results:\n");
	printf("Average Speed:\t %f M/s\n", 1e3 * input.size() * repeat_time / time_ns);
	printf("Recall Rate:\t %f\n", 1.0 * corret_count / ans.size() / repeat_time);
	printf("AAE:\t\t %f\n", sae / corret_count);
	printf("ARE:\t\t %f\n", sre / corret_count);
	printf("---------------------------------------------\n");
}