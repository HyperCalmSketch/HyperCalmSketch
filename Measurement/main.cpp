#include <cmath>
#include <ctime>
#include <cstdlib>
#include <cassert>
#include <iostream>
#include <cstring>
#include <random>
#include <set>

#include "params.h"

using namespace std;

#include "HyperCalm/lossdelay_bucketized/HyperCalm.h"
#include "HyperCalm/lossdelay_basic/HyperCalm_Basic.h"
#include "../CPU/ComparedAlgorithms/groundtruth.h"

using namespace groundtruth::type_info;

constexpr size_t cellbits = 2;
constexpr size_t BatchSize = (1 << cellbits);
constexpr double eps = 0.000001;
map<PeriodicKey,int> topK;
map<PeriodicKey,double> cnt;

bool as(Record A, Record B){
    return A.second < B.second;
}


vector<Record> randomDrop(const vector<Record> &vec) {
    int n = vec.size();
    cnt.clear();
    vector<int> pos;
    for (int i = 0; i < n; i++)
        pos.push_back(i);
    std::random_shuffle(pos.begin(), pos.end());

    set<int> drop;
    int m = n * loss_rate;
    for (int i = 0; i < m; i++) 
        drop.insert(pos[i]);

    vector<pair<uint32_t, float> > ans;
    map<uint32_t, double> last_batch, last_item;
    map<uint32_t, int16_t> delta;
    for (int i = 0; i < n; i++){
        uint32_t key = vec[i].first;
        double time = vec[i].second;
		bool in = last_item.count(key);
		if (!in || time - last_item[key] > BATCH_TIME) {
			if(in) delta[key] = (time - last_batch[key]) / UNIT_TIME;
			last_batch[key] = time;
		}
        if (drop.count(i) == false)
            ans.push_back(vec[i]);
        else if(delta.count(key)){
            int16_t _delta = delta[key];
            if(topK.count({key, _delta}) && topK[{key, _delta}])
                cnt[{key, _delta}] ++;
            else ans.push_back(vec[i]);
        }
		last_item[key] = time;
	}
    return ans;
}

vector<Record> randomDelay(const vector<Record> &vec) {
    int n = vec.size();
    cnt.clear();
    vector<int> pos;
    for (int i = 0; i < n; i++)
        pos.push_back(i);
    std::random_shuffle(pos.begin(), pos.end());

    set<int> drop;
    int m = n * loss_rate;
    for (int i = 0; i < m; i++) 
        drop.insert(pos[i]);
    random_device rd;
    default_random_engine eng(rd());
    uniform_real_distribution<double> distr(0, eps);

    vector<Record> ans = vec;
    Hash_table<uint32_t, double> last_batch, last_item, delay;
    Hash_table<uint32_t, uint16_t> delta;
    for (int i = 0; i < n; i++){
        uint32_t key = ans[i].first;
        double time = ans[i].second;
		bool in = last_item.count(key), _begin = 1;

		if (!in || time - last_item[key] > BATCH_TIME) {
			if(in) delta[key] = (time - last_batch[key]) / UNIT_TIME;
			last_batch[key] = time;
            _begin = 0;
		}
        last_item[key] = time;

        if(drop.count(i) && delta.count(key) && topK.count({key, delta[key]}) && topK[{key, delta[key]}] && _begin){
            double extra_delay = distr(eng);
            if(!delay.count(key)) delay[key] = 0;
            cnt[{key, delta[key]}] += extra_delay;
            delay[key] += extra_delay;
        }
		if(delay.count(key)) ans[i].second += delay[key];
	}
    sort(ans.begin(), ans.end(), as);
    return ans;
}


pair<pair<double, double>, pair<double, double>> average_test_drop(
    const vector<Record>& input1,
    const vector<Record>& input2,
    int t
){
    auto check = [&] (auto sketch, auto input) {
        timespec start, end;
        for (auto &[key, time] : input) {
            sketch.insert(key, time);
        }
        auto our = sketch.get_top_k(TOPK_THRESHOLD);
        map<PeriodicKey,PeriodicVal> ans;
        for(auto [x,y] : our) ans[x] = y;
        return ans;
    };
    

    map<PeriodicKey,PeriodicVal> hypercalm[4] = {check(HyperCalm(BATCH_TIME, UNIT_TIME, memory, t, 1), input1), 
                                                 check(HyperCalm(BATCH_TIME, UNIT_TIME, memory, t, 1), input2),
                                                 check(HyperCalm_Basic(BATCH_TIME, UNIT_TIME, memory, t, 1), input1),
                                                 check(HyperCalm_Basic(BATCH_TIME, UNIT_TIME, memory, t, 1), input2)};

    double value10, value11, value20, value21;
    double sum10, sum11, sum20, sum21;
    uint32_t all = 0, hit1 = 0, size1 = 0;
    double rr1 = 0, pr1 = 0;
    uint32_t hit2 = 0, size2 = 0;
    double rr2 = 0, pr2 = 0;

    for(auto it = topK.begin(); it != topK.end(); ++it){
        auto key = it->first;
        //double loss = cnt[key];
        value10 = hypercalm[0][key].second;
        value11 = hypercalm[1][key].second;
        value20 = hypercalm[2][key].second;
        value21 = hypercalm[3][key].second;
        if(cnt.count(key)) all++;

        if(value10>0 && value11>0 && (value10 - value11 >= 1)) size1 ++;
        if(cnt.count(key) && value10>0 && value11>0 && (value10 - value11 >= 1)) hit1++;

        if(value20>0 && value21>0 && (value20 - value21 >= 1)) size2 ++;
        if(cnt.count(key) && value20>0 && value21>0 && (value20 - value21 >= 1)) hit2++;
    }

    rr1 = hit1 / (double)all;
    pr1 = hit1 / (double)size1;

    rr2 = hit2 / (double)all;
    pr2 = hit2 / (double)size2;
    return {{rr1, pr1}, {rr2, pr2}};
}


pair<pair<double, double>, pair<double, double>> average_test_delay(
    const vector<Record>& input1,
    const vector<Record>& input2,
    int t
){
    auto check = [&] (auto sketch, auto input) {
        timespec start, end;
        for (auto &[key, time] : input) {
            sketch.insert(key, time);
        }
        auto our = sketch.get_top_k(TOPK_THRESHOLD);
        map<PeriodicKey,PeriodicVal> ans;
        for(auto [x,y] : our) ans[x] = y;
        return ans;
    };
    

    map<PeriodicKey,PeriodicVal> hypercalm[4] = {check(HyperCalm(BATCH_TIME, UNIT_TIME, memory, t, 0), input1), 
                                                 check(HyperCalm(BATCH_TIME, UNIT_TIME, memory, t, 0), input2),
                                                 check(HyperCalm_Basic(BATCH_TIME, UNIT_TIME, memory, t, 0), input1),
                                                 check(HyperCalm_Basic(BATCH_TIME, UNIT_TIME, memory, t, 0), input2)};

    double value10, value11, value20, value21;
    double sum10, sum11, sum20, sum21;
    uint32_t all = 0, hit1 = 0, size1 = 0;
    double rr1 = 0, pr1 = 0;
    uint32_t hit2 = 0, size2 = 0;
    double rr2 = 0, pr2 = 0;

    for(auto it = topK.begin(); it != topK.end(); ++it){
        auto key = it->first;
        value10 = hypercalm[0][key].second;
        value11 = hypercalm[1][key].second;
        value20 = hypercalm[2][key].second;
        value21 = hypercalm[3][key].second;
        sum10 = hypercalm[0][key].first;
        sum11 = hypercalm[1][key].first;
        sum20 = hypercalm[2][key].first;
        sum21 = hypercalm[3][key].first;
        if(cnt.count(key)) all++;

        if(sum10>0 && sum11>0 && (value11/sum11 - value10/sum10 > 0)) size1 ++;
        if(cnt.count(key) && sum10>0 && sum11>0 && (value11/sum11 - value10/sum10 > 0)) hit1++;

        if(sum20>0 && sum21>0 && (value21/sum21 - value20/sum20 > 0)) size2 ++;
        if(cnt.count(key) && sum20>0 && sum21>0 && (value21/sum21 - value20/sum20 > 0)) hit2++;
    }

    rr1 = hit1 / (double)all;
    pr1 = hit1 / (double)size1;

    rr2 = hit2 / (double)all;
    pr2 = hit2 / (double)size2;
    return {{rr1, pr1}, {rr2, pr2}};
}

extern void ParseArgs(int argc, char** argv);
extern vector<Record> load_data(const string& fileName);

int main(int argc, char** argv) {
    ParseArgs(argc, argv);
    cout << "---------------------------------------------" << '\n';
    auto input1 = load_data(fileName);
    cout << "---------------------------------------------" << '\n';
    groundtruth::item_count(input1);
    groundtruth::adjust_params(input1, BATCH_TIME, UNIT_TIME);
    auto ans = groundtruth::topk_time(input1, BATCH_TIME, UNIT_TIME, TOPK_THRESHOLD);
    for(int i=0; i<TOPK_THRESHOLD; i++)
        if(i&1) topK[ans[i].first] = 0;
        else topK[ans[i].first] = 1;
    printName(sketchName, checkType);
    printf("BATCH_TIME = %f, UNIT_TIME = %f\n", BATCH_TIME, UNIT_TIME);
    if (memory > 1000)
        cout << "Total Memory: " << memory / 1000. << " KB";
    else
        cout << "Total Memory: " << memory << " B";
    cout << ", Top K: " << TOPK_THRESHOLD << '\n';
    cout << "---------------------------------------------" << '\n';
    double pr_basic = 0, rr_basic = 0, pr_bucketized = 0, rr_bucketized = 0;
    for (int t = 0; t < repeat_time; ++t) {
        auto input2 = checkType == 1 ? randomDelay(input1) : randomDrop(input1);
        pair<pair<double, double>, pair<double, double>> tmp = 
            checkType == 1 ? average_test_delay(input1, input2, t) : average_test_delay(input1, input2, t) ;
        rr_basic += tmp.first.first;
        pr_basic += tmp.first.second;

        rr_bucketized += tmp.second.first;
        pr_bucketized += tmp.second.second;
    }
    rr_basic /= repeat_time;
    pr_basic /= repeat_time;

    rr_bucketized /= repeat_time;
    pr_bucketized /= repeat_time;
    cout << "---------------------------------------------" << endl;
    if(sketchName == 1){
        printf("Recall Rate of Basic HyperCalm: %lf\n", rr_basic);
        printf("Precision Rate of Basic HyperCalm: %lf\n", pr_basic);
        printf("F1 Score of Basic HyperCalm: %lf\n", (2*rr_basic*pr_basic)/(rr_basic + pr_basic));
    }
    else{
        printf("Recall Rate of Bucketized HyperCalm: %lf\n", rr_bucketized);
        printf("Precision Rate of Bucketized HyperCalm: %lf\n", pr_bucketized);
        printf("F1 Score of Bucketized HyperCalm: %lf\n", (2*rr_bucketized*pr_bucketized)/(rr_bucketized + pr_bucketized));
    }
}