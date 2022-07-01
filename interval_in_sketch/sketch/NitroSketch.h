#ifndef NITROSKETC_H
#define NITROSKETC_H

#include "common.h"
#include "utils.h"

template<typename T, uint32_t d, uint32_t w>
class NitroSketch {
public:
	uint32_t depth = d, width = w, next_packet = 0, next_depth = 0;
	double probability = NS_PROBABILITY;
	std::default_random_engine generator;
	std::vector<std::vector<T>> matrix;
	uint64_t h[d << 1], s[d << 1], n[d << 1];

	NitroSketch() {
		matrix = std::vector<std::vector<T>>(d, std::vector<T>(width, 0));
		int index = 0;
		for (int i = 0; i < (depth << 1); ++i) {
			h[i] = GenHashSeed(index++);
			s[i] = GenHashSeed(index++);
			n[i] = GenHashSeed(index++);
		}
	}

	uint32_t hash(Key_t key, int line) {
		return (uint32_t)(AwareHash((unsigned char *)key, CHARKEY_LEN + 1, h[line], s[line], n[line]) % width);
	}

	void insert(Key_t key, uint32_t val = 1) {
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
			matrix[next_depth][pos] += static_cast<T>(delta);
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

	uint32_t query(Key_t key) {
		std::vector<T> result;
		uint32_t pos;

		for (int i = 0; i < depth; ++i) {
			pos = hash(key, i);
			result.push_back(matrix[i][pos] * (static_cast<int32_t>(hash(key, depth + i) & 1) * 2 - 1));
		}

		std::sort(result.begin(), result.end());
		if (depth & 1)
			return std::abs(result.at(depth / 2));
		else
			return std::abs(result.at(depth / 2) + result.at(depth / 2 - 1)) / 2;
	}

	size_t get_memory_usage() {
		return depth * width * sizeof(T);
	}
};

#endif