#ifndef _CU_H
#define _CU_H

#include "common.h"
#include "utils.h"

template<typename T, uint32_t d, uint32_t w>
class CUSketch {
	private:
		uint32_t depth = d, width = w, overflow = 0;
		uint64_t h[d], s[d], n[d];

	public:
		std::vector<std::vector<T>> matrix;

		CUSketch() {
			matrix = std::vector<std::vector<T>>(depth, std::vector<T>(width, 0));
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
			int index[d] = {0};
    		T min_val = std::numeric_limits<T>::max();
			
    		for (int i = 0; i < depth; ++i) {
    			index[i] = hash(key, i);
    			min_val = std::min(min_val, matrix[i][index[i]]);
    		}

    		T update = min_val + val;
			// assert(update != 0);
			// if (update == 0) {
			// 	overflow++;
			// 	std::cout << overflow << " " << min_val << " " << val << std::endl;
			// }
    		for (int i = 0; i < d; ++i)
    			matrix[i][index[i]] = std::max(matrix[i][index[i]], update);
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
