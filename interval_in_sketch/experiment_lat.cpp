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
#include "PRSketch.h"
#include "histogram.h"

using namespace std;

const uint32_t flow_number = 100000;
const char *path = "../../data/test-4s-lat.dat";

vector<data_t> traces;
map<five_tuple, map<uint8_t, uint32_t>> ground_truth_all;
map<five_tuple, uint32_t> ground_truth_90_size;
map<five_tuple, uint32_t> ground_truth_95_size;

vector<HistogramBase *> testVector;

int main(int argc, char *argv[]) {
	readTraces(path, traces);

	// register sketch algorithms
	for (int i = 1; i < argc; ++i) {
		if (string(argv[i]) == "CM")
			testVector.push_back(new Histogram<CMSketch<uint16_t, CM_DEPTH, CM_WIDTH>>("CM"));
		else if (string(argv[i]) == "CU")
			testVector.push_back(new Histogram<CUSketch<uint32_t, CU_DEPTH, CU_WIDTH>>("CU"));
		else if (string(argv[i]) == "IBLT")
			testVector.push_back(new Histogram<IBLT<uint32_t, BF_WIDTH, BF_HASHNUM, IBLT_WIDTH, IBLT_HASHNUM>>("IBLT"));
		else if (string(argv[i]) == "CS")
			testVector.push_back(new Histogram<CountSketch<int32_t, CS_DEPTH, CS_WIDTH>>("CS"));
		else if (string(argv[i]) == "SuMax")
			testVector.push_back(new Histogram<SuMaxSketch<uint32_t, SM_DEPTH, SM_WIDTH>>("SuMax"));
		else if (string(argv[i]) == "ES")
			testVector.push_back(new Histogram<ElasticSketch<uint32_t, SLOT_NUM, LIGHT_DEPTH, LIGHT_WIDTH>>("ES"));
		else if (string(argv[i]) == "CBF")
			testVector.push_back(new Histogram<CBF<uint32_t, CBF_WIDTH, CBF_HASHNUM>>("CBF"));
		else if (string(argv[i]) == "NS")
			testVector.push_back(new Histogram<NitroSketch<int32_t, NS_DEPTH, NS_WIDTH>>("NS"));
		else if (string(argv[i]) == "UM")
			testVector.push_back(new Histogram<UnivMon<int32_t, UM_LAYER, UM_DEPTH, UM_WIDTH>>("UM"));
		else if (string(argv[i]) == "CCS")
			testVector.push_back(new Histogram<CoCoSketch<uint32_t, CCS_DEPTH, CCS_WIDTH>>("CCS"));
		else if (string(argv[i]) == "PR")
			testVector.push_back(new Histogram<PRSketch<uint16_t, PR_WIDTH, PR_DEPTH>>("PR"));
	}

	if (testVector.size() == 0) {
		testVector.push_back(new Histogram<CMSketch<uint16_t, CM_DEPTH, CM_WIDTH>>("CM"));
		testVector.push_back(new Histogram<CUSketch<uint32_t, CU_DEPTH, CU_WIDTH>>("CU"));
		testVector.push_back(new Histogram<IBLT<uint32_t, BF_WIDTH, BF_HASHNUM, IBLT_WIDTH, IBLT_HASHNUM>>("IBLT"));
		testVector.push_back(new Histogram<CountSketch<int32_t, CS_DEPTH, CS_WIDTH>>("CS"));
		testVector.push_back(new Histogram<SuMaxSketch<uint32_t, SM_DEPTH, SM_WIDTH>>("SuMax"));
		testVector.push_back(new Histogram<ElasticSketch<uint32_t, SLOT_NUM, LIGHT_DEPTH, LIGHT_WIDTH>>("ES"));
		testVector.push_back(new Histogram<CBF<uint32_t, CBF_WIDTH, CBF_HASHNUM>>("CBF"));
		testVector.push_back(new Histogram<NitroSketch<int32_t, NS_DEPTH, NS_WIDTH>>("NS"));
		testVector.push_back(new Histogram<UnivMon<int32_t, UM_LAYER, UM_DEPTH, UM_WIDTH>>("UM"));
		testVector.push_back(new Histogram<CoCoSketch<uint32_t, CCS_DEPTH, CCS_WIDTH>>("CCS"));
		testVector.push_back(new Histogram<PRSketch<uint16_t, PR_WIDTH, PR_DEPTH>>("PR"));
	}

	// insert data
	uint16_t max_length = 0, min_length = 0xffff;
	for (auto it = traces.begin(); it != traces.end(); it++) {
		uint8_t bid = (it->length - 90) / BUCKET_WIDTH;
		for (auto it2 = testVector.begin(); it2 != testVector.end(); it2++) {
			(*it2)->insert((Key_t)it->key.str, bid);
		}
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
		uint32_t _90_percentile = 0, _95_percentile = 0;

		if (ground_truth_90_size.find(iter->first) != ground_truth_90_size.end())
			_90_percentile = ground_truth_90_size[iter->first];

		if (ground_truth_95_size.find(iter->first) != ground_truth_95_size.end())
			_95_percentile = ground_truth_95_size[iter->first];

		for (auto it2 = testVector.begin(); it2 != testVector.end(); it2++) {
			uint32_t _90_percentile_estimate = 0, _95_percentile_estimate = 0;
			Hist_t result = (*it2)->histogramQuery((Key_t)iter->first.str);
			for (int i = 0; i  < BUCKET_NUM; ++i) {
				if (i <= 17)
					_90_percentile_estimate += result.at(i);
				if (i <= 18)
					_95_percentile_estimate += result.at(i);
			}

			if (_90_percentile)
				(*it2)->statistics._90_avgRe += fabs((double)_90_percentile_estimate - (double)_90_percentile) / _90_percentile;
			if (_95_percentile)
				(*it2)->statistics._95_avgRe += fabs((double)_95_percentile_estimate - (double)_95_percentile) / _95_percentile;
		}		
	}

	for (auto it = testVector.begin(); it !=  testVector.end(); it++) {
		cout << "--------------- " << (*it)->name << " ---------------" << endl;
		cout << "90% RE:" << (*it)->statistics._90_avgRe / ground_truth_all.size() << endl;
		cout << "95% RE:" << (*it)->statistics._95_avgRe / ground_truth_all.size() << endl;
		cout << "Memory usage:" << (double)(*it)->get_memory_usage() / (double)1024 / 1024 << endl;
	}
	return 0;
}




