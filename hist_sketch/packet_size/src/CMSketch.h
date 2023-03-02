#ifndef _CMSKETCH_H
#define _CMSKETCH_H

#include "common.h"
#include "utils.h"

template<typename T, uint32_t d, uint32_t w, uint32_t key_len>
class CMsketch {
	public:
		uint32_t depth = d, width = calNextPrime(w);
		uint64_t h[d], s[d], n[d];
		T** matrix;
		std::map<uint32_t, uint32_t> distribution;
		
		CMsketch() {
			matrix = new T*[d];
			matrix[0] = new T[width * d];
			for (int i = 1; i < d; ++i) {
				matrix[i] = matrix[i - 1] + width;
			}

			memset(matrix[0], 0, width * d * sizeof(T));

			int index = 0;
			for (int i = 0; i < depth; ++i) {
				h[i] = GenHashSeed(index++);
				s[i] = GenHashSeed(index++);
				n[i] = GenHashSeed(index++);
			}
		}

		inline uint32_t hash(Key_t key, int line) {
			return (uint32_t)(AwareHash((unsigned char *)key, key_len, h[line], s[line], n[line]) % width);
		}

		int insert(Key_t key, T val = 1) {
			uint32_t pos;
			for (int i = 0; i < depth; ++i) {
				pos = hash(key, i);
				/** not used for basic version **/
				// assert(matrix[i][pos] != std::numeric_limits<T>::max());
				matrix[i][pos] += val;
			}
			return 0;
		}
		
		T query(Key_t key) {
			uint32_t pos;
			T result = std::numeric_limits<T>::max();
			for (int i = 0; i < depth; ++i) {
				pos = hash(key, i);
				result = std::min(result, matrix[i][pos]);
			}
			return result;
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

		size_t get_memory_usage() {
			return depth * width * sizeof(T);
		}
};

#endif
