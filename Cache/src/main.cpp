#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <vector>
#include <set>
#include <list>
using namespace std;

static double BATCH_TIME_THRESHOLD = 0.727 / 5e4;
static double UNIT_TIME = BATCH_TIME_THRESHOLD * 10;
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
		exit(-1);
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
void check_css(char* filename, int sz, int cachesize) {
	vector<pair<uint32_t, float>> input = loaddata(filename);
	groundtruth::adjust_params(input, BATCH_TIME_THRESHOLD, UNIT_TIME);
	puts("(* Each item is a memory access request.)");
	//    puts("read over");
	printf("Finish reading %s\n", filename);
	printf("---------------------------------------------\n");
	puts("Parameters of the candidate algorithms:");
	int memory2 = sz;

	int memory1 = min(int(memory2 / 20), int(5e4));
	HyperBloomFilter bf(memory1, BATCH_TIME_THRESHOLD);
	int c_size = max(1, min(memory2 / 1500, 2000)),
		q_size = max(1, min(memory2 / 1000, 1000));
	auto ss = new CalmSpaceSavingCache(BATCH_TIME_THRESHOLD, UNIT_TIME, memory2 - memory1, 7, q_size, c_size);
	auto base = new ClockSketch(memory2, BATCH_TIME_THRESHOLD / 30);
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
	int tot = 1;
	for (int i = 0; i < (int)input.size(); ++i) {
		auto pr = input[i];
		auto tkey = pr.first;
		auto ttime = pr.second;
		bool b = bf.insert(tkey, ttime);
		float nxt = ss->insert(tkey, ttime, b);
		if (nxt > 0) {
			auto now = make_pair(nxt, tkey);
			if (!pref.count(now))
				pref.insert(now);
		}
		auto now = *pref.begin();
		while (pref.size() > 0 && now.first - addpre <= ttime) {
			if (ss->make_sure(now.second, now.first)) {
				string id = to_string(now.second);
				lrupre.insert(id, ++tot);
				lfupre.insert(id, 0, ttime);
			}
			pref.erase(now);
			now = *pref.begin();
		}
		string id = to_string(tkey);

		if (lru.query(id) != -1) lruHitCnt++;
		lru.insert(id, ++tot);
		if (lfu.query(id) != -1) lfuHitCnt++;
		lfu.insert(id);

		if (lrupre.query(id) != -1) lrupreHitCnt++;
		lrupre.insert(id, ++tot);

		if (lfupre.query(id) != -1) lfupreHitCnt++;
		if (b == 0)
			lfupre.insert(id, 1, ttime, ttime + BATCH_TIME_THRESHOLD * 2);
		else
			lfupre.insert(id, 1, ttime);

		if (lfubase.query(id) != -1) lfubaseHitCnt++;
		b = base->insert(tkey, ttime);
		if (b == 0)
			lfubase.insert(id, 1, ttime, ttime + BATCH_TIME_THRESHOLD * 2);
		else
			lfubase.insert(id, 1, ttime);
	}

	printf("---------------------------------------------\n");
	printf("Results:\n");
	printf("Cache Size = %d\n", cachesize);
	printf("Memory = %d\n", memory2);
	printf("Hit Rate of Basic LRU: \t\t %f\n", 1.0 * lruHitCnt / input.size() );
	printf("Hit Rate of LRU+HyperCalm: \t %f\n", 1.0 * lrupreHitCnt / input.size() );
	printf("Hit Rate of Basic LFU: \t\t %f\n", 1.0 * lfuHitCnt / input.size() );
	printf("Hit Rate of LFU+Clock: \t\t %f\n", 1.0 * lfubaseHitCnt / input.size() );
	printf("Hit Rate of LFU+HyperCalm: \t %f\n", 1.0 * lfupreHitCnt / input.size() );
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
	ParseArg(argc, argv, filename, sz, cache);
	printf("---------------------------------------------\n");
	check_css(filename, sz, cache);
	printf("---------------------------------------------\n");
	return 0;
}
