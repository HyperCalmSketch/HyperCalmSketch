#include <string.h>
#include <cstdint>
#include <cstdlib>
#include <assert.h>
#include <vector>
#include <map>
#include <unordered_map>
#include <list>
#include <string.h>
using namespace std;

#include <boost/program_options.hpp>

#include "../datasets/trace.h"
#include "../ComparedAlgorithms/ClockUSS.h"
#include "../HyperCalm/HyperCalm.h"
#include "../ComparedAlgorithms/groundtruth.h"

using namespace boost::program_options;

string fileName;
int sketchName;
double BATCH_TIME, UNIT_TIME;
int repeat_time = 1, TOPK_THERSHOLD = 200, memory = 5e5;

void ParseArgs(int argc, char** argv) {
	options_description opts("Options");
	opts.add_options()
		("fileName,f", value<string>()->required(), "file name")
		("sketchName,s", value<int>()->required(), "sketch name")
		("time,t", value<int>()->required(), "repeat time")
		("topk,k", value<int>()->required(), "topk")
		("memory,m", value<int>()->required(), "memory")
		("batch_time,b", value<double>()->required(), "batch time")
		("unit_time,u", value<double>()->required(),"unit time");
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
		if (sketchName < 1 || sketchName > 2) {
			printf("sketchName < 1 || sketchName > 2\n");
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
	if (vm.count("batch_time"))
		BATCH_TIME = vm["batch_time"].as<double>();
	if (vm.count("unit_time"))
		UNIT_TIME = vm["unit_time"].as<double>();
}

int main(int argc, char** argv) {
	ParseArgs(argc, argv);
	printf("---------------------------------------------\n");
	vector<pair<uint32_t, float>> input;
	if (fileName.back() == 't')
		input = loadCAIDA(fileName.c_str());
	else
		input = loadCRITEO(fileName.c_str());
	printf("---------------------------------------------\n");
	auto ans = groundtruth(input, BATCH_TIME, UNIT_TIME, TOPK_THERSHOLD).second;
	printf("BATCH_TIME = %f\n", BATCH_TIME);
	printf("---------------------------------------------\n");
	if (sketchName == 1) {
		puts("Test HyperCalm");
	} else {
		puts("Test Clock+USS");
	}
	sort(ans.begin(), ans.end());
	int corret_count = 0;
	double sae = 0, sre = 0;
	auto check = [&](auto sketch) {
		for (int i = 0; i < (int)input.size(); ++i) {
			auto pr = input[i];
			auto tkey = pr.first;
			auto ttime = pr.second;
			sketch.insert(tkey, ttime);
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
			check(HyperCalm(BATCH_TIME, UNIT_TIME, memory, t));
		else
			check(ClockUSS(BATCH_TIME, UNIT_TIME, memory, t));
	}
	clock_gettime(CLOCK_MONOTONIC, &end_time);
	uint64_t time_ns = uint64_t(end_time.tv_sec - start_time.tv_sec) * 1000000000 + (end_time.tv_nsec - start_time.tv_nsec);
	printf("---------------------------------------------\n");
	printf("Results:\n");
	printf("Average Speed:\t %f M/s\n", 1e3 * input.size() * repeat_time / time_ns);
	printf("Recall Rate:\t %f\n", 1.0 * corret_count / ans.size() / repeat_time);
	printf("AAE:\t\t %f\n", sae / corret_count);
	printf("ARE:\t\t %f\n", sre / corret_count);
	printf("---------------------------------------------\n");
}