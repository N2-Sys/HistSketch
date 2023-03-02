#ifndef _CBF_H
#define _CBF_H

#include "utils.h"
#include "histogram.h"


template<typename T, uint32_t w, uint32_t hn>
class CBF:public SketchBase {
	private:
		int width = w, hash_num = hn;
		uint64_t h[hn], s[hn], n[hn];

	public:
		std::vector<Histogram<T, BUCKET_NUM>> matrix;

		CBF(std::string na) {
            name = na;
			matrix = std::vector<Histogram<T, BUCKET_NUM>>(width, Histogram<T, BUCKET_NUM>());
			int index = 0;
			for (int i = 0; i < hash_num; ++i) {
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
			for (int i = 0; i < hash_num; ++i) {
				pos = hash(key, i);
				matrix[pos].insert(bid, val);
				// assert(matrix[pos].buckets[bid] != 0);
			}
		}

		uint32_t pointQuery(Key_t key, uint8_t bid) {
			uint32_t result = std::numeric_limits<uint32_t>::max();
			uint32_t pos;
			for (int i = 0; i < hash_num; ++i) {
				pos = hash(key, i);
				result = std::min(result, matrix[pos].pointQuery(bid));
			}
			return (uint32_t)result;
		}

        Hist_t histogramQuery(Key_t key) {
            T total_min = std::numeric_limits<T>::max();
			Hist_t result, tmp_result;
			uint32_t pos;
			for (int i = 0; i < hash_num; ++i) {
				pos = hash(key, i);
				tmp_result = matrix[pos].histogramQuery();
				T total = std::accumulate(tmp_result.begin(), tmp_result.end(), 0);
				if (total < total_min) {
					result = tmp_result;
                    total_min = total;
                }
			}
			return result;
        }

		size_t get_memory_usage() {
			return width * matrix[0].get_memory_usage();
		}
};

#endif