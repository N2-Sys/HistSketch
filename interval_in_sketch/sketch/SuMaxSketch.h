#ifndef _SUMAXSKETCH_H
#define _SUMAXSKETCH_H

#include "common.h"
#include "utils.h"

template<typename T, uint32_t d, uint32_t w>
class SuMaxSketch {
	private:
		int depth = d, width = w;
		uint64_t h[d], s[d], n[d];

	public:
		std::vector<std::vector<T>> matrix;

		SuMaxSketch() {
			matrix = std::vector<std::vector<T>>(d, std::vector<T>(width, 0));
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
			T omiga = std::numeric_limits<T>::max();

			for (int i = 0; i < depth; ++i) {
				pos = hash(key, i);
				T tmp = matrix[i][pos] + val;
				// assert(tmp != 0);
				if (tmp < omiga) {
					matrix[i][pos] = tmp;
					omiga = tmp;
				}
				else if (matrix[i][pos] < omiga) {
					matrix[i][pos] = omiga;
				}
				else {
					continue;
				}
			}
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
