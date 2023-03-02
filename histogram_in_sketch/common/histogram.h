#ifndef _HISTOGRAM_H
#define _HISTOGRAM_H

#include "common.h"

class SketchBase {
public:
	std::string name;

	struct statistics {
		uint32_t pointLarge = 0;
		uint32_t pointAll = 0;
		uint32_t histogramLarge = 0;
		uint32_t histogramAll = 0;
	} statistics;

	virtual void insert(Key_t key, uint8_t bid, uint32_t val = 1) {}
	virtual uint32_t pointQuery(Key_t key, uint8_t bid) {return 0;}
	virtual Hist_t histogramQuery(Key_t key) {return Hist_t{};}
	virtual size_t get_memory_usage() {return 0;}
};

template<typename T, uint32_t bucket_num>
class Histogram {
	public:
		std::vector<T> buckets;
		Histogram() {
			buckets = std::vector<T>(bucket_num, 0);
		}

		void insert(uint8_t bid, uint32_t val = 1) {
			if (!val)
				return;
			assert(bid <= bucket_num);
			// std::cout << buckets[bid] << " " << val << " " << buckets[bid] + val <<  std::endl;
			buckets[bid] += val;
			// assert(buckets[bid] != 0);
		}

		uint32_t pointQuery(uint8_t bid) {
			return (uint32_t)buckets[bid];
		}

		Hist_t histogramQuery() {
			Hist_t result = {};
			for (int i = 0; i < bucket_num; ++i) {
				result.at(i) = pointQuery(i);
			}
			return result;
		}

		uint32_t getTotal() {
			return std::accumulate(buckets.begin(), buckets.end(), 0);
		}

		static size_t get_memory_usage() {
			return bucket_num * sizeof(T);
		}
};

#endif
