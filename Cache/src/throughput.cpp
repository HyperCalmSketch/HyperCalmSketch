#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <ctime>
#include <cassert>
#include <vector>
#include <set>

using namespace std;

const int TOPK_THRESHOLD = 100;
const double BATCH_TIME_THRESHOLD = 0.727 / 5e4;
const double UNIT_TIME = BATCH_TIME_THRESHOLD * 10;
const double addpre = UNIT_TIME * 30;

#include "../../CPU/ComparedAlgorithms/ClockSketch.h"
#include "../../CPU/ComparedAlgorithms/groundtruth.h"
#include "../../CPU/HyperCalm/HyperBloomFilter.h"
#include "CalmSpaceSavingCache.h"
#include "lfu.h"
#include "lru.h"

set<pair<float, uint32_t>> pref;
vector<pair<uint32_t, float>> loaddata(char* filename) {
	FILE* pf = fopen(filename, "rb");
	if (!pf) {
		printf("%s not found!\n", filename);
		// exit(-1);
	}
	double ftime = -1;
	vector<pair<uint32_t, float>> vec;
	uint32_t id;
	while (fscanf(pf, "%d", &id) != EOF) {
		uint32_t tkey = id;
		double ttime;
		fscanf(pf, "%lf", &ttime);
		if (ftime < 0) ftime = ttime;
		vec.push_back(pair<uint32_t, float>(tkey, ttime - ftime));
	}
	fclose(pf);
	return vec;
}
int Id = 0;
void check_css(char* filename, int sz, int cachesize) {
	vector<pair<uint32_t, float>> input = loaddata(filename);

	printf("Finish reading %s\n", filename);
	if(Id==2 || Id==5 || Id==4){
		printf("---------------------------------------------\n");
		puts("Parameters of the candidate algorithms:");
	}

	timespec time1, time2;
	int memory2 = sz;
	int memory1 = min(int(memory2 / 20), int(5e4));
	// HyperBloomFilter bf(memory1, BATCH_TIME_THRESHOLD);
	auto bf = (Id==2 || Id==5) ? new HyperBloomFilter(memory1, BATCH_TIME_THRESHOLD) : NULL; 
	int c_size = max(1, min(memory2 / 1500, 2000)),
		q_size = max(1, min(memory2 / 1000, 1000));
	auto ss = (Id==2 || Id==5) ? new CalmSpaceSavingCache(BATCH_TIME_THRESHOLD, UNIT_TIME, memory2 - memory1, 7, q_size, c_size) : NULL;
	auto base = (Id==4) ? new ClockSketch(memory2, BATCH_TIME_THRESHOLD) : NULL;
	LRU lru(cachesize);
	LRU lrupre(cachesize);
	LFU lfu(cachesize);
	LFU lfupre(cachesize);
	LFU lfubase(cachesize);

	int lruHitCnt = 0;
	int lfuHitCnt = 0;
	int lrupreHitCnt = 0;
	int lfupreHitCnt = 0;
	int lfubaseHitCnt = 0;
	// printf("%d\n", (int)input.size());
	int tot = 1;
	clock_gettime(CLOCK_MONOTONIC, &time1);
	for (int i = 0; i < (int)input.size(); ++i) {
		auto pr = input[i];
		auto tkey = pr.first;
		auto ttime = pr.second;
		bool b;
		if (Id == 2 || Id == 5) {
			b = bf->insert(tkey, ttime);
			float nxt = ss->insert(tkey, ttime, b);
			if (nxt > 0) {
				auto now = make_pair(nxt, tkey);
				if (!pref.count(now))
					pref.insert(now);
				// printf("insert %d %f\n",now.second,now.first);
			}
			auto now = *pref.begin();
			while (pref.size() > 0 && now.first - addpre <= ttime) {
				if (ss->make_sure(now.second, now.first)) {
					string id = to_string(now.second);
					if (Id == 2) lrupre.insert(id, ++tot);
					if (Id == 5) lfupre.insert(id, 0, ttime);
					// puts("alb");
				}
				pref.erase(now);
				now = *pref.begin();
			}
		}
		string id = to_string(tkey);

		if (Id == 1) {
			if (lru.query(id) != -1) lruHitCnt++;
			lru.insert(id, ++tot);
		}
		if (Id == 2) {
			if (lrupre.query(id) != -1) lrupreHitCnt++;
			lrupre.insert(id, ++tot);
		}
		if (Id == 3) {
			if (lfu.query(id) != -1) lfuHitCnt++;
			lfu.insert(id);
		}
		if (Id == 4) {
			if (lfubase.query(id) != -1) lfubaseHitCnt++;
			b = base->insert(tkey, ttime);
			if (b == 0)
				lfubase.insert(id, 1, ttime, ttime + BATCH_TIME_THRESHOLD * 2);
			else
				lfubase.insert(id, 1, ttime);
		}
		if (Id == 5) {
			if (lfupre.query(id) != -1) lfupreHitCnt++;
			if (b == 0)
				lfupre.insert(id, 1, ttime, ttime + BATCH_TIME_THRESHOLD * 2);
			else
				lfupre.insert(id, 1, ttime);
		}
	}
	clock_gettime(CLOCK_MONOTONIC, &time2);
	uint64_t resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
	double insertionMops = (double)1000.0 * (input.size()) / resns;
	printf("---------------------------------------------\n");
	printf("Results:\n");
	printf("# Requests: %lu, Throughput: %lf M/s\n", input.size(), insertionMops);
}
#include <boost/program_options.hpp>
using namespace boost::program_options;
void ParseArg(int argc, char* argv[], char* filename, int& sz, int& cachesize) {
	options_description opts("Cache Simulator Options");

	opts.add_options()("memory,m", value<int>()->required(), "memory size")("cachesize,c", value<int>()->required(), "lines of the cache")("version,v", value<int>()->required(), "version")("filename,f", value<string>()->required(), "trace file")("help,h", "print help info");
	boost::program_options::variables_map vm;

	store(parse_command_line(argc, argv, opts), vm);

	if (vm.count("help")) {
		cout << opts << endl;
		return;
	}

	if (vm.count("memory")) {
		sz = vm["memory"].as<int>();
	} else {
		printf("please use -m to specify the memory size.\n");
		exit(0);
	}

	if (vm.count("cachesize")) {
		cachesize = vm["cachesize"].as<int>();
	} else {
		printf("please use -c to specify the lines of the cache.\n");
		exit(0);
	}
	if (vm.count("version")) {
		Id = vm["version"].as<int>();
	} else {
		printf("please use -v to specify version.\n");
		exit(0);
	}
	//--filename tmp.txt
	if (vm.count("filename")) {
		strcpy(filename, vm["filename"].as<string>().c_str());
	} else {
		printf("please use -f to specify the trace file.\n");
		exit(0);
	}
}
int main(int argc, char* argv[]) {
	int sz, cache;
	char filename[100];
	// sscanf(argv[1],"%s",filename);
	// sscanf(argv[2],"%d",&sz);
	// sscanf(argv[3],"%d",&cache);
	// sscanf(argv[4],"%d",&Id);
	ParseArg(argc, argv, filename, sz, cache);
	printf("---------------------------------------------\n");
	check_css(filename, sz, cache);
	printf("---------------------------------------------\n");
	return 0;
}
