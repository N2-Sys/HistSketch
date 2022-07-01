#include "CMSketch.h"
#include "CUSketch.h"
#include "IBLT.h"
#include "CountSketch.h"
#include "SuMaxSketch.h"
#include "ElasticSketch.h"
#include "CBF.h"
#include "NitroSketch.h"
#include "UnivMon.h"
#include "CoCoSketch.h"
#include "histogram.h"

using namespace std;

vector<data_t> traces;

const uint32_t flow_number = 150000;
const char *path = "../../../data/webdocs.dat";

vector<HistogramBase *> testVector[2];
map<five_tuple, map<uint8_t, uint32_t>> ground_truth_all[2];
map<five_tuple, uint32_t> ground_truth_size[2];
map<five_tuple, uint32_t> ground_truth_hc;
map<five_tuple, map<uint8_t, uint32_t>> ground_truth_hc_dist;

int main (int argc, char *argv[]) {
	readTraces(path, traces);

	// register sketch algorithms
	for (int i = 1; i < argc; ++i) {
		if (string(argv[i]) == "CM") {
			testVector[0].push_back(new Histogram<CMSketch<uint32_t, CM_DEPTH, CM_WIDTH>>("CM"));
			testVector[1].push_back(new Histogram<CMSketch<uint32_t, CM_DEPTH, CM_WIDTH>>("CM"));
		}
		else if (string(argv[i]) == "CU") {
			testVector[0].push_back(new Histogram<CUSketch<uint32_t, CU_DEPTH, CU_WIDTH>>("CU"));
			testVector[1].push_back(new Histogram<CUSketch<uint32_t, CU_DEPTH, CU_WIDTH>>("CU"));
		}
		else if (string(argv[i]) == "IBLT") {
			testVector[0].push_back(new Histogram<IBLT<uint32_t, BF_WIDTH, BF_HASHNUM, IBLT_WIDTH, IBLT_HASHNUM>>("IBLT"));
			testVector[1].push_back(new Histogram<IBLT<uint32_t, BF_WIDTH, BF_HASHNUM, IBLT_WIDTH, IBLT_HASHNUM>>("IBLT"));
		}
		else if (string(argv[i]) == "CS") {
			testVector[0].push_back(new Histogram<CountSketch<int32_t, CS_DEPTH, CS_WIDTH>>("CS"));
			testVector[1].push_back(new Histogram<CountSketch<int32_t, CS_DEPTH, CS_WIDTH>>("CS"));
		}
		else if (string(argv[i]) == "SuMax") {
			testVector[0].push_back(new Histogram<SuMaxSketch<uint32_t, SM_DEPTH, SM_WIDTH>>("SuMax"));
			testVector[1].push_back(new Histogram<SuMaxSketch<uint32_t, SM_DEPTH, SM_WIDTH>>("SuMax"));
		}
		else if (string(argv[i]) == "ES") {
			testVector[0].push_back(new Histogram<ElasticSketch<uint32_t, SLOT_NUM, LIGHT_DEPTH, LIGHT_WIDTH>>("ES"));
			testVector[1].push_back(new Histogram<ElasticSketch<uint32_t, SLOT_NUM, LIGHT_DEPTH, LIGHT_WIDTH>>("ES"));
		}
		else if (string(argv[i]) == "CBF") {
			testVector[0].push_back(new Histogram<CBF<uint32_t, CBF_WIDTH, CBF_HASHNUM>>("CBF"));
			testVector[1].push_back(new Histogram<CBF<uint32_t, CBF_WIDTH, CBF_HASHNUM>>("CBF"));
		}
		else if (string(argv[i]) == "NS") {
			testVector[0].push_back(new Histogram<NitroSketch<int32_t, NS_DEPTH, NS_WIDTH>>("NS"));
			testVector[1].push_back(new Histogram<NitroSketch<int32_t, NS_DEPTH, NS_WIDTH>>("NS"));
		}
		else if (string(argv[i]) == "UM") {
			testVector[0].push_back(new Histogram<UnivMon<int32_t, UM_LAYER, UM_DEPTH, UM_WIDTH>>("UM"));
			testVector[1].push_back(new Histogram<UnivMon<int32_t, UM_LAYER, UM_DEPTH, UM_WIDTH>>("UM"));
		}
		else if (string(argv[i]) == "CCS") {
			testVector[0].push_back(new Histogram<CoCoSketch<uint32_t, CCS_DEPTH, CCS_WIDTH>>("CCS"));
			testVector[1].push_back(new Histogram<CoCoSketch<uint32_t, CCS_DEPTH, CCS_WIDTH>>("CCS"));
		}
	}

	if (testVector[0].size() == 0) {
		for (int i = 0; i < 2; ++i) {
			testVector[i].push_back(new Histogram<CMSketch<uint32_t, CM_DEPTH, CM_WIDTH>>("CM"));
			testVector[i].push_back(new Histogram<CUSketch<uint32_t, CU_DEPTH, CU_WIDTH>>("CU"));
			testVector[i].push_back(new Histogram<IBLT<uint32_t, BF_WIDTH, BF_HASHNUM, IBLT_WIDTH, IBLT_HASHNUM>>("IBLT"));
			testVector[i].push_back(new Histogram<CountSketch<int32_t, CS_DEPTH, CS_WIDTH>>("CS"));
			testVector[i].push_back(new Histogram<SuMaxSketch<uint32_t, SM_DEPTH, SM_WIDTH>>("SuMax"));
			testVector[i].push_back(new Histogram<ElasticSketch<uint16_t, SLOT_NUM, LIGHT_DEPTH, LIGHT_WIDTH>>("ES"));
			testVector[i].push_back(new Histogram<CBF<uint32_t, CBF_WIDTH, CBF_HASHNUM>>("CBF"));
			testVector[i].push_back(new Histogram<NitroSketch<int32_t, NS_DEPTH, NS_WIDTH>>("NS"));
			testVector[i].push_back(new Histogram<UnivMon<int32_t, UM_LAYER, UM_DEPTH, UM_WIDTH>>("UM"));
			testVector[i].push_back(new Histogram<CoCoSketch<uint32_t, CCS_DEPTH, CCS_WIDTH>>("CCS"));
		}
	}

	// insert data
	uint16_t max_length[2] = {0}, min_length[2] = {0xffff, 0xffff};
	uint32_t stop = 181630 / 2, cnt = 0;
	for (auto it = traces.begin(); it != traces.end(); it++) {
		uint32_t idx = cnt / stop;
		uint8_t bid = transform_bid((it->length - 2608) / 100);
		for (auto it2 = testVector[idx].begin(); it2 != testVector[idx].end(); it2++) {
			(*it2)->insert((Key_t)it->key.str, bid);
		}
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

	// point query of heavy changers
	double are = 0;
	uint32_t bucket_change = 0;
	for (auto it = ground_truth_hc_dist.begin(); it != ground_truth_hc_dist.end(); ++it) {
		for (auto it2 = it->second.begin(); it2 != it->second.end(); ++it2) {			// true histogram
			if (it2->second == 0)
				continue;
			int v_size = testVector[0].size();
			for (int i = 0; i < v_size; ++i) {
				uint32_t result1 = testVector[0][i]->pointQuery((Key_t)it->first.str, it2->first);
				uint32_t result2 = testVector[1][i]->pointQuery((Key_t)it->first.str, it2->first);
				uint32_t change = abs((int64_t)result1 - (int64_t)result2);
				are = abs((double)change - it2->second) / (double)it2->second;
				if (are < POINT_ARETHRESHOLD) {
					testVector[0][i]->statistics.pointChange++;
				}
				else {
					// cout << change << " " << it2->second << endl;
				}
			}
			bucket_change++;
		}
	}

	// histogram query of changers
	for (auto it = ground_truth_hc_dist.begin(); it != ground_truth_hc_dist.end(); ++it) {
		int v_size = testVector[0].size();
		for (int i = 0; i < v_size; ++i) {
			double totalerr = 0;
			uint32_t totalcnt = 0;
			Hist_t result1 = testVector[0][i]->histogramQuery((Key_t)it->first.str);
			Hist_t result2 = testVector[1][i]->histogramQuery((Key_t)it->first.str);
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
				testVector[0][i]->statistics.histogramChange++;
			}
			else {
				// std::cout << totalerr << " " << totalcnt << std::endl;
			}
		}
	}

	cout << ground_truth_hc_dist.size() << endl;


	for (auto it = testVector[0].begin(); it !=  testVector[0].end(); it++) {
		cout << "--------------- " << (*it)->name << " ---------------" << endl;
		cout << "Point query for heavy changers (proportion of buckets): " <<
			(double)(*it)->statistics.pointChange / bucket_change << endl;
		cout << "Histogram query for heavy changers (proportion of flows): " <<
			(double)(*it)->statistics.histogramChange / ground_truth_hc_dist.size() << endl;
		cout << "Memory usage: " << (double)(*it)->get_memory_usage() / 1024 / 1024 << " MB" << endl;
	}
	
	return 0;
}
