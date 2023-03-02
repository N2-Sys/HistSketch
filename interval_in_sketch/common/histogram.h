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
		uint32_t pointChange = 0;
		uint32_t histogramChange = 0;
		double pointLargeARE = 0;
		double pointAllARE = 0;
		double histogramLargeARE = 0;
		double histogramAllARE = 0;
		double cardinalityRE = 0;
		double entropyRE = 0;
		double _90_avgRe = 0;
		double _95_avgRe = 0;
		double total_insert_time = 0;
	} statistics;

	virtual void insert(Key_t key, uint8_t bid, uint32_t val = 1) {}

	virtual uint32_t pointQuery(Key_t key, uint8_t bid) {return 0;}

	virtual Hist_t histogramQuery(Key_t key) {return Hist_t{};}

	virtual size_t get_memory_usage() {return 0;}
};


template<class T>
class Histogram:public HistogramBase {
public:
	T sketch;

	Histogram(std::string n) {
		name = n;
	}

	void insert(Key_t key, uint8_t bid, uint32_t val = 1) {
		new_data_t tmp_new_key(key, bid);
		sketch.insert(tmp_new_key.str());
	}

	uint32_t pointQuery(Key_t key, uint8_t bid) {
		new_data_t tmp_new_key(key, bid);
		uint32_t result = sketch.query(tmp_new_key.str());
		return result;
	}

	Hist_t histogramQuery(Key_t key) {
		Hist_t arr{};
		for (int i = 0; i < BUCKET_NUM; ++i) {
			new_data_t tmp_new_key(key, i);
			arr.at(i) = sketch.query(tmp_new_key.str());
		}
		return arr;
	}

	size_t get_memory_usage() {return sketch.get_memory_usage();}
};

#endif