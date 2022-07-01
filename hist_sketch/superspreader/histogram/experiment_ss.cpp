#include "sketch.h"

using namespace std;

const uint32_t flow_number = 100000;
const char *path = "../../../data/test-4s.dat";

vector<int_data_t> traces;
map<Key_t, map<uint8_t, uint32_t>> ground_truth_all;
map<Key_t, set<uint32_t>> ground_truth_set;

sketch<uint32_t, H_BUCKET_NUM, SLOT_NUM, CM_DEPTH_HIST, CM_WIDTH_HIST, BLOOM_SIZE, BLOOM_HASH_NUM> hist;

int main() {
	readTraces(path, traces);

	// insert data
	uint32_t max_dst = 0, min_dst = numeric_limits<uint32_t>::max();
	for (auto it = traces.begin(); it != traces.end(); it++) {
		// TODO: calculate bid
		uint16_t bid;
		if (it->dst < 1300000000)
			bid = 0;
		else {
			bid = (it->dst - 1100000000) / (48752 * 256 + 1);
			assert(bid < BUCKET_NUM);
		}
		hist.insert(it->key, bid);
		//decode the ground truth
		ground_truth_all[it->key][bid]++;
		ground_truth_set[it->key].insert(bid);
		max_dst = max(max_dst, (uint32_t)bid);
		min_dst = min(min_dst, (uint32_t)bid);
		if (ground_truth_all.size() == flow_number) {
			break;
		}
	}
	
	uint32_t total_buckets = 0;
	for (auto it = ground_truth_all.begin(); it != ground_truth_all.end(); ++it) {
		total_buckets += it->second.size();
	}

	printf("Flow number:%d\nBucket number:%d\nmax ip in the data:%u\nmin ip in the data:%u\n",
		(int)ground_truth_all.size(), total_buckets, max_dst, min_dst);

	// uint32_t max_spread = 0, min_spread = numeric_limits<uint32_t>::max();
	// map<uint32_t, uint32_t> spread_distr;
	// for (auto iter = ground_truth_set.begin(); iter != ground_truth_set.end(); ++iter) {
	// 	uint32_t size = iter->second.size();
	// 	max_spread = max(max_spread, size);
	// 	min_spread = min(min_spread, size);
	// 	spread_distr[size]++;
	// }

	// cout << "Spread: " << max_spread << " " << min_spread << endl;
	// cout << "Distribution:\n";
	// for (auto iter = spread_distr.begin(); iter != spread_distr.end(); ++iter) {
	// 	cout << iter->first << " " << iter->second << endl;
	// }

	uint32_t fp = 0, fn = 0, tp = 0, tn = 0;
	int cnt = 0;
	for (auto iter = ground_truth_set.begin(); iter != ground_truth_set.end(); ++iter) {
		Hist_t result = hist.histogramQuery(iter->first);
		int ss_cnt = hist_card;
		uint32_t total = 0;
		// for (int i = 0; i < BUCKET_NUM; ++i) {
		// 	// if (result.at(i))
		// 	// 	ss_cnt++;
		// 	total += result.at(i);
		// }
		bool isSS = ss_cnt > SS_ESTIMATE_THRESHOLD && hist_total > SS_TRUE_THRESHOLD;
		if (iter->second.size() > SS_TRUE_THRESHOLD) {
			if (isSS) {
				tp++;
			}
			else {
				cout << "FN: " << ss_cnt << " " << iter->second.size() << endl;
				fn++;
			}
		}
		else {
			if (!isSS) {
				// cout << "TN: " << ss_cnt << endl;
				tn++;
			}
			else {
				fp++;
				cout << "FP: " << ss_cnt << " " << iter->second.size() << " " << ground_truth_all[iter->first].size() << endl;
				// cout << "total: " << total << endl;
				// for (int i = 0; i < BUCKET_NUM; ++i) {
				// 	cout << result.at(i) << endl;
				// }
			}
		}
	}

	cout << "False positive: " << fp << endl;
	cout << "False negative: " << fn << endl;
	cout << "True positive: " << tp << endl;
	cout << "True negative: " << tn << endl;
	cout << "Precision: " << (double)tp / (tp + fp) << endl;
	cout << "Recall: " << (double)tp / (tp + fn) << endl;
	cout << "Memory usage: " << (double)hist.get_memory_usage() / 1024 / 1024 << " MB" << endl;
	
	return 0;
}




