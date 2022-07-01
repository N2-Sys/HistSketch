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

using namespace std;

vector<data_t> traces;

const uint32_t flow_number = 100000;
const char *path = "../../data/test-4s.dat";

vector<SketchBase *> testVector;

map<five_tuple, map<uint8_t, uint32_t>> ground_truth_all;
map<five_tuple, map<uint8_t, uint32_t>> ground_truth_large;
map<five_tuple, uint32_t> ground_truth_size;
map<uint32_t, vector<five_tuple>> sort_ground_truth_size;

int main (int argc, char *argv[]) {
	readTraces(path, traces);

	// register sketch algorithms
	for (int i = 1; i < argc; ++i) {
		if (string(argv[i]) == "CM")
			testVector.push_back(new CMSketch<uint32_t, CM_DEPTH, CM_WIDTH>("CM"));
		else if (string(argv[i]) == "CU")
			testVector.push_back(new CUSketch<uint32_t, CU_DEPTH, CU_WIDTH>("CU"));
		else if (string(argv[i]) == "IBLT")
			testVector.push_back(new IBLT<uint32_t, BF_WIDTH, BF_HASHNUM, IBLT_WIDTH, IBLT_HASHNUM>("IBLT"));
		else if (string(argv[i]) == "CS")
			testVector.push_back(new CountSketch<int32_t, CS_DEPTH, CS_WIDTH>("CS"));
		else if (string(argv[i]) == "SuMax")
			testVector.push_back(new SuMaxSketch<uint32_t, SM_DEPTH, SM_WIDTH>("SuMax"));
		else if (string(argv[i]) == "ES")
			testVector.push_back(new ElasticSketch<uint32_t, SLOT_NUM, LIGHT_DEPTH, LIGHT_WIDTH>("ES"));
		else if (string(argv[i]) == "CBF")
			testVector.push_back(new CBF<uint32_t, CBF_WIDTH, CBF_HASHNUM>("CBF"));
		else if (string(argv[i]) == "NS")
			testVector.push_back(new NitroSketch<int32_t, NS_DEPTH, NS_WIDTH>("NS"));
		else if (string(argv[i]) == "UM")
			testVector.push_back(new UnivMon<int32_t, UM_LAYER, UM_DEPTH, UM_WIDTH>("UM"));
		else if (string(argv[i]) == "CCS")
			testVector.push_back(new CoCoSketch<uint32_t, CCS_DEPTH, CCS_WIDTH>("CCS"));
	}

	if (testVector.size() == 0) {
		testVector.push_back(new CMSketch<uint32_t, CM_DEPTH, CM_WIDTH>("CM"));
		testVector.push_back(new CUSketch<uint32_t, CU_DEPTH, CU_WIDTH>("CU"));
		testVector.push_back(new IBLT<uint32_t, BF_WIDTH, BF_HASHNUM, IBLT_WIDTH, IBLT_HASHNUM>("IBLT"));
		testVector.push_back(new CountSketch<int32_t, CS_DEPTH, CS_WIDTH>("CS"));
		testVector.push_back(new SuMaxSketch<uint32_t, SM_DEPTH, SM_WIDTH>("SuMax"));
		testVector.push_back(new ElasticSketch<uint32_t, SLOT_NUM, LIGHT_DEPTH, LIGHT_WIDTH>("ES"));
		testVector.push_back(new CBF<uint32_t, CBF_WIDTH, CBF_HASHNUM>("CBF"));
		testVector.push_back(new NitroSketch<int32_t, NS_DEPTH, NS_WIDTH>("NS"));
		testVector.push_back(new UnivMon<int32_t, UM_LAYER, UM_DEPTH, UM_WIDTH>("UM"));
		testVector.push_back(new CoCoSketch<uint32_t, CCS_DEPTH, CCS_WIDTH>("CCS"));
	}

	// insert data
	uint16_t max_length = 0, min_length = 0xffff;
	for (auto it = traces.begin(); it != traces.end(); it++) {
		uint8_t bid = it->length / BUCKET_WIDTH;
		for (auto it2 = testVector.begin(); it2 != testVector.end(); it2++) {
			(*it2)->insert((Key_t)it->key.str, bid);
		}
		//decode the ground truth
		ground_truth_all[it->key][bid]++;
		ground_truth_size[it->key]++;
		max_length = max(max_length, it->length);
		min_length = min(min_length, it->length);
		// if (ground_truth_size.size() == flow_number) {
		// 	break;
		// }
	}

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
			for (auto it3 = testVector.begin(); it3 != testVector.end(); it3++) {
				uint32_t result = (*it3)->pointQuery((Key_t)it->first.str, it2->first);
				are = abs((double)result - it2->second) / it2->second;
				if (are <= POINT_ARETHRESHOLD) {
					(*it3)->statistics.pointLarge++;
				}
				else {
					// cout << result << " " << it2->second << endl;
				}
			}
			bucket_large++;
		}
	}

	// point query of all flows
	uint32_t bucket_all = 0;
	for (auto it = ground_truth_all.begin(); it != ground_truth_all.end(); ++it) {
		for (auto it2 = it->second.begin(); it2 != it->second.end(); ++it2) {			// true histogram
			for (auto it3 = testVector.begin(); it3 != testVector.end(); it3++) {
				uint32_t result = (*it3)->pointQuery((Key_t)it->first.str, it2->first);
				are = abs((double)result - it2->second) / it2->second;
				if (are <= POINT_ARETHRESHOLD) {
					(*it3)->statistics.pointAll++;
				}
				else {
					// cout << result << " " << it2->second << endl;
				}
			}
			bucket_all++;
		}
	}

	//histogram query of large flows
	for (auto it = ground_truth_large.begin(); it != ground_truth_large.end(); ++it) {
		for (auto it2 = testVector.begin(); it2 != testVector.end(); it2++) {
			double totalerr = 0;
			uint32_t totalcnt = 0;
			Hist_t result = (*it2)->histogramQuery((Key_t)it->first.str);
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
			if (are <= HISTOGRAM_ARETHRESHOLD) {
				(*it2)->statistics.histogramLarge++;
			}
			else {
				// std::cout << totalerr << " " << totalcnt << std::endl;
			}
		}
	}

	// histogram query of all flows
	for (auto it = ground_truth_all.begin(); it != ground_truth_all.end(); ++it) {
		for (auto it2 = testVector.begin(); it2 != testVector.end(); it2++) {
			double totalerr = 0;
			uint32_t totalcnt = 0;
			Hist_t result = (*it2)->histogramQuery((Key_t)it->first.str);
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
			if (are <= HISTOGRAM_ARETHRESHOLD) {
				(*it2)->statistics.histogramAll++;
			}
			else {
				// std::cout << totalerr << " " << totalcnt << std::endl;
			}
		}
	}

	for (auto it = testVector.begin(); it !=  testVector.end(); it++) {
		cout << "--------------- " << (*it)->name << " ---------------" << endl;
		cout << "Point query for large flows (proportion of buckets): " <<
			(double)(*it)->statistics.pointLarge / bucket_large << endl;
		cout << "Point query for all flows (proportion of buckets): " <<
			(double)(*it)->statistics.pointAll / bucket_all << endl;
		cout << "Histogram query for large flows (proportion of flows): " <<
			(double)(*it)->statistics.histogramLarge / ground_truth_large.size() << endl;
		cout << "Histogram query for all flows (proportion of flows): " <<
			(double)(*it)->statistics.histogramAll / ground_truth_all.size() << endl;
		cout << "Memory usage: " << (double)(*it)->get_memory_usage() / 1024 / 1024 << " MB" << endl;
	}
	
	return 0;
}
