#include "sketch.h"

using namespace std;

const uint32_t flow_number = 100000;
const char *path = "../../../data/test-4s.dat";

vector<data_t> traces;
map<five_tuple, map<uint8_t, uint32_t>> ground_truth_all;
map<five_tuple, map<uint8_t, uint32_t>> ground_truth_large;
map<five_tuple, uint32_t> ground_truth_size;
map<uint32_t, vector<five_tuple>> sort_ground_truth_size;

sketch<uint32_t, H_BUCKET_NUM, SLOT_NUM, CM_DEPTH_HIST, CM_WIDTH_HIST, BLOOM_SIZE, BLOOM_HASH_NUM> hist;

int main() {
	readTraces(path, traces);

	// insert data
	uint16_t max_length = 0, min_length = 0xffff, packet_cnt = 0;
	double total_insert_time = 0;
	for (auto it = traces.begin(); it != traces.end(); it++) {
		uint8_t bid = it->length / BUCKET_WIDTH;
		new_data_t tmp_key((Key_t)it->key.str, bid);
		auto t_a = std::chrono::high_resolution_clock::now();
		hist.insert((Key_t)it->key.str, bid, tmp_key);
		auto t_b = std::chrono::high_resolution_clock::now();
		total_insert_time += std::chrono::duration_cast<std::chrono::microseconds>(t_b - t_a).count();

		//decode the ground truth
		ground_truth_all[it->key][bid]++;
		ground_truth_size[it->key]++;
		max_length = max(max_length, it->length);
		min_length = min(min_length, it->length);
		packet_cnt++;
		// if (ground_truth_size.size() == flow_number) {
		// 	break;
		// }
	}

	cout << packet_cnt / total_insert_time << "MPPS" << endl;

	ground_truth_large = ground_truth_all;

	for (auto it = ground_truth_size.begin(); it != ground_truth_size.end(); it++) {
		sort_ground_truth_size[it->second].push_back(it->first);
	}

	for (auto it = sort_ground_truth_size.begin(); it != sort_ground_truth_size.end(); it++) {
		if (it->first < 1000) {
			for (auto j = it->second.begin(); j != it->second.end(); ++j) {
				ground_truth_large.erase(*j);
			}
		}
	}

	printf("Flow number:%d\nLarge flow number:%d\nmax length in the data:%d\nmin length in the data:%d\n",
		(int)ground_truth_all.size(), (int)ground_truth_large.size(), max_length, min_length);

	//point query of large flows
	double are = 0;
	uint32_t bucket_large = 0;
	for (auto it = ground_truth_large.begin(); it != ground_truth_large.end(); ++it) {
		for (auto it2 = it->second.begin(); it2 != it->second.end(); ++it2) {			// true histogram
			uint32_t result = hist.pointQuery((Key_t)it->first.str, it2->first);
			are = abs((double)result - it2->second) / it2->second;
			if (are < POINT_ARETHRESHOLD) {
				hist.statistics.pointLarge++;
			}
			else {
				// cout << result << " " << it2->second << endl;
			}
			bucket_large++;
		}
	}

	// point query of all flows
	uint32_t bucket_all = 0;
	for (auto it = ground_truth_all.begin(); it != ground_truth_all.end(); ++it) {
		for (auto it2 = it->second.begin(); it2 != it->second.end(); ++it2) {			// true histogram
			// if (compareKeyHist((Key_t)it->first.str, it2->first))
			// 	cout << "hi\n";
			uint32_t result = hist.pointQuery((Key_t)it->first.str, it2->first);
			are = abs((double)result - it2->second) / it2->second;
			if (are < POINT_ARETHRESHOLD) {
				hist.statistics.pointAll++;
			}
			else {
				// cout << "Wrong answer: " << result << " " << it2->second << endl;
				// for (int i = 0; i < CHARKEY_LEN; ++i) {
				// 	cout << (uint32_t)(uint8_t)it->first.str[i] << " ";
				// }
				// cout << (uint32_t)(uint8_t)it2->first << endl;
			}
			bucket_all++;
		}
	}

	//histogram query of large flows
	for (auto it = ground_truth_large.begin(); it != ground_truth_large.end(); ++it) {
		double totalerr = 0;
		uint32_t totalcnt = 0;
		Hist_t result = hist.histogramQuery((Key_t)it->first.str);
		for (int i = 0; i  < BUCKET_NUM; ++i) {
			if (it->second.find(i) != it->second.end()) {
				totalerr += fabs((double)result.at(i) - it->second[i]);
				totalcnt += it->second[i];
			}
			else {
				totalerr += fabs(result.at(i));
			}
		}
		are = totalerr / totalcnt;
		if (are < HISTOGRAM_ARETHRESHOLD) {
			hist.statistics.histogramLarge++;
		}
		else {
			// std::cout << totalerr << " " << totalcnt << std::endl;
		}
	}

	// histogram query of all flows
	for (auto it = ground_truth_all.begin(); it != ground_truth_all.end(); ++it) {
		double totalerr = 0;
		uint32_t totalcnt = 0;
		Hist_t result = hist.histogramQuery((Key_t)it->first.str);
		for (int i = 0; i  < BUCKET_NUM; ++i) {
			if (it->second.find(i) != it->second.end()) {
				totalerr += fabs((double)result.at(i) - it->second[i]);
				totalcnt += it->second[i];
			}
			else {
				totalerr += fabs(result.at(i));
			}
		}
		are = totalerr / totalcnt;
		if (are < HISTOGRAM_ARETHRESHOLD) {
			hist.statistics.histogramAll++;
		}
		else {
			// std::cout << totalerr << " " << totalcnt << std::endl;
		}
	}

	// cardinality
	uint32_t cardinality_truth = get_cardinality(ground_truth_all);
	uint32_t cardinality_estimate = hist.get_cardinality();
	hist.statistics.cardinalityRE = fabs((double)cardinality_truth - cardinality_estimate) / cardinality_truth;
	cout << "Cardinality(estimate, truth):" << cardinality_estimate << " " << cardinality_truth << endl;

	// entropy
	double entropy_truth = get_entropy(ground_truth_all);
	double entropy_estimate = hist.get_entropy();
	hist.statistics.entropyRE = fabs((double)entropy_truth - entropy_estimate) / entropy_truth;
	cout << "Entropy(estimate, truth):" << entropy_estimate << " " << entropy_truth << endl;

	
	cout << "Point query for large flows (proportion of buckets): " <<
		(double)hist.statistics.pointLarge / bucket_large << endl;
	cout << "Point query for all flows (proportion of buckets): " <<
		(double)hist.statistics.pointAll / bucket_all << endl;
	cout << "Histogram query for large flows (proportion of flows): " <<
		(double)hist.statistics.histogramLarge / ground_truth_large.size() << endl;
	cout << "Histogram query for all flows (proportion of flows): " <<
		(double)hist.statistics.histogramAll / ground_truth_all.size() << endl;
	cout << "Cardinality relative error: " << hist.statistics.cardinalityRE << endl;
	cout << "Entropy relative error: " << hist.statistics.entropyRE << endl;
	cout << "Memory usage: " << (double)hist.get_memory_usage() / 1024 / 1024 << " MB" << endl;
	
	return 0;
}




