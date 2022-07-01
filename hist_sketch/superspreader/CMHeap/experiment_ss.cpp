#include "CMHeap.h"

using namespace std;

const uint32_t flow_number = 100000;
const char *path = "../../../data/test-4s.dat";

vector<int_data_t> traces;
map<Key_t, set<uint32_t>> ground_truth_all;
CMHeap<CMH_DEPTH, CMH_WIDTH> cmh(CMH_B, CMH_BB, CMH_C);

int main() {
	readTraces(path, traces);

	// insert data
	uint32_t max_dst = 0, min_dst = numeric_limits<uint32_t>::max();
	for (auto it = traces.begin(); it != traces.end(); it++) {
		uint16_t bid;
		if (it->dst < 1300000000)
			bid = 0;
		else {
			bid = (it->dst - 1100000000) / (48752 * 256 + 1);
		}
		cmh.update(it->key, bid);
		//decode the ground truth
		ground_truth_all[it->key].insert(bid);
		max_dst = max(max_dst, (uint32_t)bid);
		min_dst = min(min_dst, (uint32_t)bid);
		if (ground_truth_all.size() == flow_number) {
			break;
		}
	}

	printf("Flow number:%d\nmax ip in the data:%d\nmin ip in the data:%d\n",
		(int)ground_truth_all.size(), max_dst, min_dst);

    cmh.get_superspreaders();

	uint32_t fp = 0, fn = 0, tp = 0, tn = 0;
	for (auto iter = ground_truth_all.begin(); iter != ground_truth_all.end(); ++iter) {
		if (iter->second.size() > SS_TRUE_THRESHOLD) {
            if (cmh.result.find(iter->first) != cmh.result.end()) {
                tp++;
            }
            else {
                fn++;
            }
		}
		else {
            if (cmh.result.find(iter->first) == cmh.result.end()) {
                tn++;
            }
            else {
                fp++;
            }
		}
	}


	cout << "False positive: " << fp << endl;
	cout << "False negative: " << fn << endl;
	cout << "True positive: " << tp << endl;
	cout << "True negative: " << tn << endl;
	cout << "Precision: " << (double)tp / (tp + fp) << endl;
	cout << "Recall: " << (double)tp / (tp + fn) << endl;
	cout << "Memory usage: " << (double)cmh.get_memory_usage() / 1024 / 1024 << " MB" << endl;
    return 0;
}