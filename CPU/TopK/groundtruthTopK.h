#ifndef _GROUNDTRUTHTOPK_H_
#define _GROUNDTRUTHTOPK_H_


vector<pair<int, int>> groundtruth(const vector<pair<uint32_t, float>>& input, int TOPK_THRESHOLD) {
	map<int, int> cnt;
	for (int i = 0; i < (int)input.size(); ++i) {
		auto [tkey, ttime] = input[i];
		++cnt[tkey];
	}
	vector<pair<int, int>> ans;
	for (auto [key, c]: cnt)
		ans.push_back({ -c, key });
	assert(TOPK_THRESHOLD <= (int)ans.size());
	nth_element(ans.begin(), ans.begin() + TOPK_THRESHOLD, ans.end());
	vector<pair<int, int>> ans2(TOPK_THRESHOLD);
	for (int i = 0; i < TOPK_THRESHOLD; ++i) ans2[i] = { ans[i].second, -ans[i].first };
	return ans2;
}

#endif // _GROUNDTRUTHTOPK_H_