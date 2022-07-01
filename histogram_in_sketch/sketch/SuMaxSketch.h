#ifndef _SUMAXSKETCH_H
#define _SUMAXSKETCH_H

#include "utils.h"
#include "histogram.h"

template<typename T, uint32_t d, uint32_t w>
class SuMaxSketch: public SketchBase {
	private:
		int depth = d, width = w;
		uint64_t h[d], s[d], n[d];

	public:
		std::vector<std::vector<Histogram<T, BUCKET_NUM>>> matrix;

		SuMaxSketch(std::string na) {
			name = na;
			matrix = std::vector<std::vector<Histogram<T, BUCKET_NUM>>>(d, std::vector<Histogram<T, BUCKET_NUM>>(width));
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

		void insert(Key_t key, uint32_t val = 1) {
			uint32_t pos;
			T omiga = std::numeric_limits<T>::max();

			for (int i = 0; i < depth; ++i) {
				pos = hash(key, i);
				T tmp = matrix[i][pos] + val;
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

        void insert(Key_t key, uint8_t bid, uint32_t val = 1) {
            uint32_t pos;
            T omiga = std::numeric_limits<T>::max();

            for (int i = 0; i < depth; ++i) {
                pos = hash(key, i);
                T tmp = matrix[i][pos].buckets[bid] + val;
                if (tmp < omiga) {
                    matrix[i][pos].buckets[bid] = tmp;
					omiga = tmp;
                }
                else if (matrix[i][pos].buckets[bid] < omiga) {
                    matrix[i][pos].buckets[bid] = omiga;
                }
                else {
                    continue;
                }
            }
        }

        uint32_t pointQuery(Key_t key, uint8_t bid) {
			T result = std::numeric_limits<T>::max();
			uint32_t pos;
			for (int i = 0; i < depth; ++i) {
				pos = hash(key, i);
				result = std::min(result, matrix[i][pos].pointQuery(bid));
			}
			return (uint32_t)result;
		}

		Hist_t histogramQuery(Key_t key) {
			T total_min = std::numeric_limits<T>::max();
			Hist_t result, tmp_result;
			uint32_t pos;
			for (int i = 0; i < depth; ++i) {
				pos = hash(key, i);
				tmp_result = matrix[i][pos].histogramQuery();
				T total = std::accumulate(tmp_result.begin(), tmp_result.end(), 0);
				if (total < total_min) {
					result = tmp_result;
                    total_min = total;
                }
			}
			return result;
		}

        size_t get_memory_usage() {
			return depth * width * matrix[0][0].get_memory_usage();
		}

};

#endif