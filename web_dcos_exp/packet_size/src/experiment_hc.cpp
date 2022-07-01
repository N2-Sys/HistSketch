#include "sketch.h"

using namespace std;

const uint32_t flow_number = 150000;
const char *path = "../../../data/webdocs.dat";

vector<data_t> traces;

map<five_tuple, map<uint8_t, uint32_t>> ground_truth_all[2];
map<five_tuple, uint32_t> ground_truth_size[2];
map<five_tuple, uint32_t> ground_truth_hc;
map<five_tuple, map<uint8_t, uint32_t>> ground_truth_hc_dist;

sketch<uint32_t, H_BUCKET_NUM, SLOT_NUM, CM_DEPTH_HIST, CM_WIDTH_HIST, BLOOM_SIZE, BLOOM_HASH_NUM> hist[2];

int main() {
	readTraces(path, traces);

	// insert data
	uint16_t max_length[2] = {0}, min_length[2] = {0xffff, 0xffff};
	uint32_t stop = 181630 / 2, cnt = 0;
	for (auto it = traces.begin(); it != traces.end(); it++) {
		uint32_t idx = cnt / stop;
		uint8_t bid = transform_bid((it->length - 2608) / 100);
		new_data_t tmp_key((Key_t)it->key.str, bid);
		hist[idx].insert((Key_t)it->key.str, bid, tmp_key);
		//decode the ground truth
		ground_truth_all[idx][it->key][bid]++;
		ground_truth_size[idx][it->key]++;
		max_length[idx] = max(max_length[idx], it->length);
		min_length[idx] = min(min_length[idx], it->length);
		cnt++;
		if (ground_truth_size[0].size() + ground_truth_size[1].size() == flow_number) {
			break;
		}
	}

	// cout <<  "hi\n";

	for (int i = 0; i < 2; ++i) {
		printf("**Part%d**\nFlow number:%d\nmax length in the data:%d\nmin length in the data:%d\n",
			i+1, (int)ground_truth_all[i].size(), max_length[i], min_length[i]);
	}

	// calculate changes
	for (auto iter = ground_truth_size[0].begin(); iter != ground_truth_size[0].end(); iter++) {
		if (ground_truth_size[1].find(iter->first) != ground_truth_size[1].end()) {
			ground_truth_hc.emplace(iter->first, abs((int)iter->second - (int)ground_truth_size[1][iter->first]));
		}
		else {
			ground_truth_hc.emplace(iter->first, iter->second);
		}
	}
	for (auto iter = ground_truth_size[1].begin(); iter != ground_truth_size[1].end(); iter++) {
		if (ground_truth_hc.find(iter->first) == ground_truth_hc.end()) {
			ground_truth_hc.emplace(iter->first, iter->second);
		}
	}

	// calculate heavy changers
	map<uint8_t, uint32_t> changers;
	for (auto iter = ground_truth_hc.begin(); iter != ground_truth_hc.end(); iter++) {
		changers.clear();
		if (iter->second > HC_SIZE) {
			for (int i = 0; i < BUCKET_NUM; ++i) {
				uint32_t result[2] = {0};
				for (int j = 0; j < 2; ++j) {
					if (ground_truth_all[j].find(iter->first) != ground_truth_all[j].end()
						&& ground_truth_all[j][iter->first].find(i) != ground_truth_all[j][iter->first].end())
					{
						result[j] = ground_truth_all[j][iter->first][i];
					}
				}
				changers.emplace(i, abs((int64_t)result[0] - (int64_t)result[1]));
			}
			ground_truth_hc_dist.emplace(iter->first, changers);
		}
	}

	// cout << ground_truth_hc_dist.size() << endl;

	//point query of heavy changers
	double are = 0;
	uint32_t bucket_change = 0;
	for (auto it = ground_truth_hc_dist.begin(); it != ground_truth_hc_dist.end(); ++it) {
		for (auto it2 = it->second.begin(); it2 != it->second.end(); ++it2) {			// true histogram
			if (it2->second == 0)
				continue;
			// cout << "hi\n";
			uint32_t result1 = hist[0].pointQuery((Key_t)it->first.str, it2->first);
			uint32_t result2 = hist[1].pointQuery((Key_t)it->first.str, it2->first);
			uint32_t change = abs((int64_t)result1 - (int64_t)result2);
			are = abs((double)change - it2->second) / (double)it2->second;
			if (are < POINT_ARETHRESHOLD) {
				hist[0].statistics.pointChange++;
				// cout << change << " " << it2->second << endl;
			}
			else {
				// cout << change << " " << it2->second << endl;
			}
			bucket_change++;
		}
	}


	//histogram query of heavy changers
	for (auto it = ground_truth_hc_dist.begin(); it != ground_truth_hc_dist.end(); ++it) {
		double totalerr = 0;
		uint32_t totalcnt = 0;
		Hist_t result1 = hist[0].histogramQuery((Key_t)it->first.str);
		Hist_t result2 = hist[1].histogramQuery((Key_t)it->first.str);
		Hist_t result{};
		for (int j = 0; j < BUCKET_NUM; ++j) {
			result.at(j) = abs((int64_t)result1.at(j) - (int64_t)result2.at(j));
		}
		for (int j = 0; j  < BUCKET_NUM; ++j) {
			if (it->second.find(j) != it->second.end()) {
				totalerr += fabs((double)result.at(j) - it->second[j]);
				totalcnt += it->second[j];
			}
			else {
				totalerr += fabs(result.at(j));
			}
		}
		are = totalerr / totalcnt;
		if (are < HISTOGRAM_ARETHRESHOLD) {
			hist[0].statistics.histogramChange++;
		}
		else {
			// std::cout << totalerr << " " << totalcnt << std::endl;
		}
	}
	
	cout << "Point query for heavy changers (proportion of buckets): " <<
		(double)hist[0].statistics.pointChange / bucket_change << endl;
	cout << "Histogram query for heavy changers (proportion of flows): " <<
		(double)hist[0].statistics.histogramChange / ground_truth_hc_dist.size() << endl;
	cout << "Memory usage: " << (double)hist[0].get_memory_usage() / 1024 / 1024 << " MB" << endl;
	
	return 0;
}




