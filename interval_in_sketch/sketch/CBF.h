#ifndef _CBF_H
#define _CBF_H

#include "common.h"
#include "utils.h"

template<typename T, uint32_t w, uint32_t hn>
class CBF {
	private:
		int width = w, hash_num = hn;
		uint64_t h[hn], s[hn], n[hn];

	public:
		std::vector<T> matrix;

		CBF() {
			matrix = std::vector<T>(width, 0);
			int index = 0;
			for (int i = 0; i < hash_num; ++i) {
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
			for (int i = 0; i < hash_num; ++i) {
				pos = hash(key, i);
				matrix[pos] += val;
				// std::cout << i << " " << pos << " " << matrix[i][pos] << std::endl;
				// assert(matrix[pos] != 0);
			}
		}

		uint32_t query(Key_t key) {
			T result = std::numeric_limits<T>::max();
			uint32_t pos;
			for (int i = 0; i < hash_num; ++i) {
				pos = hash(key, i);
				result = std::min(result, matrix[pos]);
			}
			return (uint32_t)result;
		}

		size_t get_memory_usage() {
			return width * sizeof(T);
		}
};

#endif
