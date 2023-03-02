#ifndef _HISTOGRAM_H
#define _HISTOGRAM_H

#include "common.h"

class HistogramBase {
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


template<class T, uint32_t n, uint32_t w>
class Histogram: public HistogramBase {
	private:
		std::vector<T> buckets;

	public:
		Histogram(std::string s) {
			name = s;
			buckets = std::vector<T>(n);
		}

		void insert(Key_t key, uint8_t bid, uint32_t val = 1) {
			// assert(bid <= n);
			// std::cout << "bid: " << bid << std::endl;
			buckets[bid].insert(key, val);
		}

		uint32_t pointQuery(Key_t key, uint8_t bid) {
			return buckets[bid].query(key);
		}

		Hist_t histogramQuery(Key_t key) {
			Hist_t result = {};
			for (int i = 0; i < n; ++i) {
				result.at(i) = pointQuery(key, i);
			}
			return result;
		}

		size_t get_memory_usage() {
			return n * buckets[0].get_memory_usage();
		}
};

#endif
