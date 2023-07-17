#ifndef _GROUNDTRUTH_H_
#define _GROUNDTRUTH_H_
#include <algorithm>
vector<pair<pair<int, int16_t>, int>> get_topk(const vector<pair<uint32_t, float>>& input, const vector<int>& batches, double UNIT_TIME, int TOPK_THRESHOLD) {
	map<pair<int, int16_t>, int> cnt;
	map<int, double> las;
	for (auto i: batches) {
		auto [v, t] = input[i];
		if (las.count(v)) {
			++cnt[{ v, (t - las[v]) / UNIT_TIME }];
		}
		las[v] = t;
	}

	vector<pair<int, pair<int, int16_t>>> q;
	for (auto [pr, c]: cnt)
		q.push_back({ -c, pr });
	if (q.size() < TOPK_THRESHOLD) exit(0);
	nth_element(q.begin(), q.begin() + TOPK_THRESHOLD, q.end());
	vector<pair<pair<int, int16_t>, int>> q1;
	int mn_c = 1e9;
	for (int i = 0; i < TOPK_THRESHOLD; ++i) {
		q1.push_back({ q[i].second, -q[i].first });
		mn_c = min(mn_c, -q[i].first);
	}
	return q1;
}
pair<vector<int>, vector<pair<pair<int, int16_t>, int>>> groundtruth(const vector<pair<uint32_t, float>>& input, double& BATCH_TIME_THRESHOLD, double& UNIT_TIME, int TOPK_THRESHOLD) {
	map<int, double> la;
	if (!BATCH_TIME_THRESHOLD) {
		double sum = 0;
		int cnt = 0;
		for (int i = 0; i < (int)input.size(); ++i) {
			auto [tkey, ttime] = input[i];
			if (la.count(tkey)) {
				sum += ttime - la[tkey];
				++cnt;
			}
			la[tkey] = ttime;
		}
		la.clear();
		BATCH_TIME_THRESHOLD = sum / cnt / 1/*0*/;
	}
	map<int, int> cnt;
	int mx_cnt = 0;
	for (int i = 0; i < (int)input.size(); ++i) {
		auto [tkey, ttime] = input[i];
		mx_cnt = max(mx_cnt, ++cnt[tkey]);
	}
    printf("# distinct items: %d\n",(int)cnt.size());


	if (!UNIT_TIME) {
		UNIT_TIME = BATCH_TIME_THRESHOLD * 10;
	}

	vector<int> batches;
	for (int i = 0; i < (int)input.size(); ++i) {
		auto [tkey, ttime] = input[i];
		if (la.count(tkey) && ttime - la[tkey] <= BATCH_TIME_THRESHOLD) {
		} else {
			batches.push_back(i);
		}
		la[tkey] = ttime;
	}

    printf("# items = %d\n",(int)input.size());
    printf("# item batches %d\n",int(batches.size()));
    printf("Freq. of the hottest item = %d\n",mx_cnt);

	return { batches, get_topk(input, batches, UNIT_TIME, TOPK_THRESHOLD) };
}
#endif //_GROUNDTRUTH_H_