#ifndef _COCOSKETCH_H
#define _COCOSKETCH_H

#include "utils.h"
#include "histogram.h"

template<typename T>
class bucket {
public:
    five_tuple key;
    Histogram<T, BUCKET_NUM> value;

    bucket() {
    }

    size_t get_memory_usage() {
        // std::cout << sizeof(key) + value.get_memory_usage() << std::endl;
        return sizeof(key) + value.get_memory_usage();
    }
};

template<typename T, uint32_t d, uint32_t w>
class CoCoSketch:public SketchBase {
public:
    uint32_t depth = d, width = w;
    std::vector<std::vector<bucket<T>>> matrix;
    uint64_t h[d], s[d], n[d];

    CoCoSketch(std::string na) {
        name = na;
        matrix = std::vector<std::vector<bucket<T>>>(depth, std::vector<bucket<T>>(width, bucket<T>()));
        int index = 0;
		for (int i = 0; i < depth; ++i) {
			h[i] = GenHashSeed(index++);
			s[i] = GenHashSeed(index++);
			n[i] = GenHashSeed(index++);	
		}
    }

    uint32_t hash(Key_t key, int line) {
        return (uint32_t)(AwareHash((unsigned char *)key, CHARKEY_LEN, h[line], s[line], n[line]) % width);
    }

    void insert(Key_t key, uint8_t bid, uint32_t val = 1) {
        uint32_t pos;
        five_tuple tmp_key(key);
        bool matched = false;
        T min_value = std::numeric_limits<T>::max();
        std::pair<int, uint32_t> min_index = std::make_pair(-1, -1);
        uint32_t tmp_total = 0, total = 0;

        for (int i = 0; i < depth; ++i) {
            pos = hash(key, i);
            if (matrix[i][pos].key == tmp_key)  {
                matched = true;
                matrix[i][pos].value.insert(bid, val);
            }
            Hist_t tmp_result = matrix[i][pos].value.histogramQuery();
            tmp_total = std::accumulate(tmp_result.begin(), tmp_result.end(), 0);
            if (min_value > tmp_total) {
                min_value = tmp_total;
                min_index = std::make_pair(i, pos);
                total = tmp_total;
            }
        }
        if (!matched) {
            matrix[min_index.first][min_index.second].value.insert(bid, val);
            double probability_threshold = (double)val / total;
            std::random_device rd;  //Will be used to obtain a seed for the random number engine
			std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
			std::uniform_real_distribution<> dis(0.0, 1.0);
			double probability = dis(gen);
            if (probability < probability_threshold) {
                matrix[min_index.first][min_index.second].key = tmp_key;
            }
        }
    }

    uint32_t pointQuery(Key_t key, uint8_t bid) {
        uint32_t pos;
        five_tuple tmp_key(key);
        for (int i = 0; i < depth; ++i) {
            pos = hash(key, i);
            if (matrix[i][pos].key == tmp_key) {
                return matrix[i][pos].value.pointQuery(bid);
            }
        }
        return 0;
    }

    Hist_t histogramQuery(Key_t key) {
        uint32_t pos;
        five_tuple tmp_key(key);
        for (int i = 0; i < depth; ++i) {
            pos = hash(key, i);
            if (matrix[i][pos].key == tmp_key) {
                return matrix[i][pos].value.histogramQuery();
            }
        }
        return Hist_t{};
    }

    // uint32_t query(Key_t key) {
    //     uint32_t pos;
    //     five_tuple tmp_key(key);
    //     uint32_t min_value = std::numeric_limits<uint32_t>::max();
    //     for (int i = 0; i < depth; ++i) {
    //         pos = hash(key, i);
    //         if (matrix[i][pos].key == tmp_key) {
    //             return matrix[i][pos].value;
    //         }
    //         min_value = std::min(min_value, matrix[i][pos].value);
    //     }
    //     return min_value;
    // }

    size_t get_memory_usage() {
        return depth * width * matrix[0][0].get_memory_usage();
    }
};

#endif