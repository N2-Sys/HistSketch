#ifndef _UNIVMON_H
#define _UNIVMON_H

#include "utils.h"
#include "histogram.h"
#include "CountSketch.h"

template<typename T, uint32_t layer, uint32_t d, uint32_t w>
class UnivMon:public SketchBase {
public:
	std::vector<CountSketch<T, d, w>> sketches;
	uint32_t layer_num = layer;
	uint64_t h[layer - 1], s[layer - 1], n[layer - 1];

	UnivMon(std::string na) {
        name = na;
		sketches = std::vector<CountSketch<T, d, w>>(layer, CountSketch<T, d, w>("CS"));
		int index = 0;
		for (int i = 0; i < layer - 1; ++i) {
			h[i] = GenHashSeed(index++);
			s[i] = GenHashSeed(index++);
			n[i] = GenHashSeed(index++);	
		}
	}

	uint32_t hash(Key_t key, uint32_t line) {
		return (uint32_t)(AwareHash((unsigned char *)key, CHARKEY_LEN, h[line], s[line], n[line]) & 0x1);
	}

	void insert(Key_t key, uint8_t bid, uint32_t val = 1) {
		uint32_t hash_result;
		for (int i = 0; i < layer; ++i) {
			if (i) {
				hash_result = hash(key, i);
				if (!hash_result)
					break;
			}
			sketches[i].insert(key, bid, val);
		}
	}

    uint32_t pointQuery(Key_t key, uint8_t bid) {
        uint32_t hash_result, result = 0;
		for (int i = layer - 1; i >= 0; --i) {
			hash_result = hash(key, i);
			if (!hash_result)
				continue;
			if (!result)
				result = sketches[i].pointQuery(key, bid);
			else
				result = result * 2 - sketches[i].pointQuery(key, bid);
		}
		return result;
    }

    Hist_t histogramQuery(Key_t key) {
        uint32_t hash_result;
        Hist_t result{}, tmp_result;
        for (int i = layer - 1; i >= 0; --i) {
            hash_result = hash(key, i);
            if (!hash_result)
                continue;
            if (result == Hist_t{})
                result = sketches[i].histogramQuery(key);
            else {
                tmp_result = sketches[i].histogramQuery(key);
                for (int i = 0; i < BUCKET_NUM; ++i) {
                    result.at(i) = result.at(i) * 2 - tmp_result.at(i);
                }
            }
        }
        return result;
    }

	size_t get_memory_usage() {
		return sketches[0].get_memory_usage() * layer_num;
	}
};

#endif