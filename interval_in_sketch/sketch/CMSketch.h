#ifndef _CM_H
#define _CM_H

#include "common.h"
#include "utils.h"

template<typename T, uint32_t d, uint32_t w>
class CMSketch {
	private:
		int depth = d, width = w;
		uint64_t h[d], s[d], n[d];
		std::map<T, uint32_t> distribution;
		uint32_t overflow = 0;

	public:
		std::vector<std::vector<T>> matrix;

		CMSketch() {
			matrix = std::vector<std::vector<T>>(d, std::vector<T>(width, 0));
			distribution.clear();
			int index = 0;
			for (int i = 0; i < depth; ++i) {
				h[i] = GenHashSeed(index++);
				s[i] = GenHashSeed(index++);
				n[i] = GenHashSeed(index++);
			}
		}

		uint32_t hash(Key_t key, int line) {
			return (uint32_t)(AwareHash((unsigned char *)key, CHARKEY_LEN + 1, h[line], s[line], n[line]) % width);
		}

		void insert(Key_t key, uint32_t val = 1) {
			uint32_t pos;
			for (int i = 0; i < depth; ++i) {
				pos = hash(key, i);
				matrix[i][pos] += val;
				// std::cout << i << " " << pos << " " << matrix[i][pos] << std::endl;
				// assert(matrix[i][pos] != 0);
				// if (matrix[i][pos] == 0) {
				// 	overflow++;
				// 	std::cout << overflow << std::endl;
				// }
			}
		}

		void swap_insert(Key_t key, uint32_t val = 1) {
			uint32_t pos;
			for (int i = 0; i < depth; ++i) {
				pos = hash(key, i);
				matrix[i][pos] = std::max(val, (uint32_t)matrix[i][pos]);
			}
		}

		void get_distribution() {
			if (!distribution.empty())
				return;
			// for (int i = 0; i < depth; ++i) {
				for (int j = 0; j < width; ++j) {
					T value = matrix[0][j];
					if (!value)
						continue;
					if (distribution.find(value) == distribution.end())
						distribution.emplace(value, 1);
					else
						distribution[value] += 1;
				}
			// }
		}

		std::pair<uint32_t, double> get_entropy() {
			uint32_t total = 0;
			double entropy = 0;
			get_distribution();
			for (auto iter = distribution.begin(); iter != distribution.end(); ++iter) {
				total += iter->first * iter->second;
				entropy += iter->first * iter->second * log2(iter->first);
			}
			return std::make_pair(total, entropy);
		}

		uint32_t get_cardinality() {
			get_distribution();
			uint32_t cardinality = 0;
			double counter_num = width;

			for (auto iter = distribution.begin(); iter != distribution.end(); ++iter) {
				cardinality += iter->second;
			}

			double rate = (counter_num - cardinality) / counter_num;
			return counter_num * log(1 / rate);
		}

		uint32_t query(Key_t key) {
			T result = std::numeric_limits<T>::max();
			uint32_t pos;
			for (int i = 0; i < depth; ++i) {
				pos = hash(key, i);
				result = std::min(result, matrix[i][pos]);
			}
			return (uint32_t)result;
		}

		size_t get_memory_usage() {
			return depth * width * sizeof(T);
		}
};

#endif
