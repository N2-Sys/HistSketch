#include "sketch.h"

using namespace std;

const uint32_t flow_number = 100000;
const char *path = "../../../data/test-4s-lat.dat";

vector<data_t> traces;
map<five_tuple, map<uint8_t, uint32_t>> ground_truth_all;
map<five_tuple, uint32_t> ground_truth_90_size;
map<five_tuple, uint32_t> ground_truth_95_size;

sketch<uint32_t, H_BUCKET_NUM, SLOT_NUM, CM_DEPTH_HIST, CM_WIDTH_HIST, BLOOM_SIZE, BLOOM_HASH_NUM> hist;

int main() {
	readTraces(path, traces);

	// insert data
	uint16_t max_length = 0, min_length = 0xffff;
	for (auto it = traces.begin(); it != traces.end(); it++) {
		uint8_t bid = (it->length - 90) / BUCKET_WIDTH;
		hist.insert((Key_t)it->key.str, bid);
		//decode the ground truth
		ground_truth_all[it->key][bid]++;
		if (bid <= 17)
			ground_truth_90_size[it->key]++;
		if (bid <= 18)
			ground_truth_95_size[it->key]++;
		max_length = max(max_length, it->length);
		min_length = min(min_length, it->length);
		// if (ground_truth_size.size() == flow_number) {
		// 	break;
		// }
	}

	printf("Max latency: %d.\nMin latency: %d.\n", max_length, min_length);

	double _90_avgRe = 0, _95_avgRe = 0;
	for (auto iter = ground_truth_all.begin(); iter != ground_truth_all.end(); iter++) {
		uint32_t _90_percentile = 0, _95_percentile = 0, _90_percentile_estimate = 0, _95_percentile_estimate = 0;

		if (ground_truth_90_size.find(iter->first) != ground_truth_90_size.end())
			_90_percentile = ground_truth_90_size[iter->first];

		if (ground_truth_95_size.find(iter->first) != ground_truth_95_size.end())
			_95_percentile = ground_truth_95_size[iter->first];

		Hist_t result = hist.histogramQuery((Key_t)iter->first.str);
		for (int i = 0; i  < BUCKET_NUM; ++i) {
			if (i <= 17)
				_90_percentile_estimate += result.at(i);
			if (i <= 18)
				_95_percentile_estimate += result.at(i);
		}

		// cout << _90_percentile << " " << _90_percentile_estimate << endl;

		if (_90_percentile)
			_90_avgRe += fabs((double)_90_percentile_estimate - (double)_90_percentile) / _90_percentile;
		if (_95_percentile)
			_95_avgRe += fabs((double)_95_percentile_estimate - (double)_95_percentile) / _95_percentile;
	}

	cout << "90% RE:" << _90_avgRe / ground_truth_all.size() << endl;
	cout << "95% RE:" << _95_avgRe / ground_truth_all.size() << endl;
	cout << "Memory usage:" << hist.get_memory_usage() / (double)1024 / 1024 << endl;
	return 0;
}




