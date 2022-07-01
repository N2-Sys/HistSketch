#ifndef _NITROSKETCH_H
#define _NITROSKETCH_H


#include "utils.h"
#include "histogram.h"

template<typename T, uint32_t d, uint32_t w>
class NitroSketch:public SketchBase {
public:
	uint32_t depth = d, width = w, next_packet = 0, next_depth = 0;
	double probability = NS_PROBABILITY;
	std::default_random_engine generator;
	std::vector<std::vector<Histogram<T, BUCKET_NUM>>> matrix;
	uint64_t h[d << 1], s[d << 1], n[d << 1];

	NitroSketch(std::string na) {
        name = na;
		matrix = std::vector<std::vector<Histogram<T, BUCKET_NUM>>>(d, std::vector<Histogram<T, BUCKET_NUM>>(width));
		int index = 0;
		for (int i = 0; i < (depth << 1); ++i) {
			h[i] = GenHashSeed(index++);
			s[i] = GenHashSeed(index++);
			n[i] = GenHashSeed(index++);
		}
	}

	uint32_t hash(Key_t key, int line) {
		return (uint32_t)(AwareHash((unsigned char *)key, CHARKEY_LEN, h[line], s[line], n[line]) % width);
	}

	void insert(Key_t key, uint8_t bid, uint32_t val = 1) {
		int update_cnt = 0;
		while (true) {
			// std::cout << next_packet << std::endl;
			if (next_packet) {
				next_packet--;
				return;
			}
			update_cnt++;
			// std::cout << next_depth << std::endl;
			uint32_t pos = hash(key, next_depth);
			double delta = 1.0 * val / probability * (static_cast<int32_t>(hash(key, next_depth + depth) & 1) * 2 - 1);
			matrix[next_depth][pos].insert(bid, static_cast<T>(delta));
			getNextUpdate();
			if (update_cnt == depth)
				break;
		}
	}

	void getNextUpdate() {
		uint32_t sample = 1;
		if (probability < 1.0) {
			std::geometric_distribution<int> dist(probability);
			sample = 1 + dist(generator);
		}
		next_packet = sample / d;
		next_depth = (next_depth + sample) % d;
	}

    uint32_t pointQuery(Key_t key, uint8_t bid) {
        std::vector<T> result;
        uint32_t pos;

        for (int i = 0; i < depth; ++i) {
            pos = hash(key, i);
            result.push_back(static_cast<int32_t>(matrix[i][pos].buckets[bid]) * (static_cast<int32_t>(hash(key, i + depth) & 1) * 2 - 1));
            // std::cout << ((int)(hash(key, depth + i) & 1) * 2 - 1) << std::endl;
        }

        std::sort(result.begin(), result.end());
        // std::cout << result.at(depth / 2) << " " << result.at(depth / 2) + result.at(depth / 2 - 1) << std::endl;
        if (depth & 1)
            return std::abs(result.at(depth / 2));
        else
            return std::abs(result.at(depth / 2) + result.at(depth / 2 - 1)) / 2;
    }

    Hist_t histogramQuery(Key_t key) {
        std::vector<std::pair<T, std::array<int32_t, BUCKET_NUM>>> result_vector;
        uint32_t pos;
        std::array<int32_t, BUCKET_NUM> tmp_result{};
        int32_t total;
        Hist_t result;

        for (int i = 0; i < depth; ++i) {
            pos = hash(key, i);
            for (int j = 0; j < BUCKET_NUM; ++j)
                tmp_result.at(j) = matrix[i][pos].buckets[j];

            total = std::accumulate(tmp_result.begin(), tmp_result.end(), 0);
            result_vector.push_back(std::make_pair(total, tmp_result));
        }

        std::sort(result_vector.begin(), result_vector.end());

        if (depth & 1) {
            for (int i = 0; i < BUCKET_NUM; ++i) {
                result.at(i) = std::abs(result_vector.at(depth / 2).second.at(i));
            }
            return result;
        }
        else {
            for (int i = 0; i < BUCKET_NUM; ++i) {
                result.at(i) = std::abs(result_vector.at(depth / 2).second.at(i) + result_vector.at(depth / 2 - 1).second.at(i)) / 2;
            }
            return result;
        }
    }

	size_t get_memory_usage() {
        return depth * width * matrix[0][0].get_memory_usage();
    }
};

#endif