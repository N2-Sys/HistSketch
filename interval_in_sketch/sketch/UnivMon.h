#ifndef UNIVMON_H
#define UNIVMON_H

#include "common.h"
#include "utils.h"
#include "CountSketch.h"

template<typename T, uint32_t layer, uint32_t d, uint32_t w>
class UnivMon {
public:
	std::vector<CountSketch<T, d, w>> sketches;
	uint32_t layer_num = layer, element_num = 0;
	uint64_t h[layer - 1], s[layer - 1], n[layer - 1];
	std::map<new_data_t, uint32_t> heap[layer];

	UnivMon() {
		sketches = std::vector<CountSketch<T, d, w>>(layer, CountSketch<T, d, w>());
		int index = 0;
		for (int i = 0; i < layer - 1; ++i) {
			h[i] = GenHashSeed(index++);
			s[i] = GenHashSeed(index++);
			n[i] = GenHashSeed(index++);	
		}
	}

	uint32_t hash(Key_t key, uint32_t line) {
		return (uint32_t)(AwareHash((unsigned char *)key, CHARKEY_LEN + 1, h[line], s[line], n[line]) & 0x1);
	}

	void insert(Key_t key, uint32_t val = 1) {
		element_num++;

		uint32_t hash_result;
		for (int i = 0; i < layer; ++i) {
			if (i) {
				hash_result = hash(key, i);
				if (!hash_result)
					break;
			}
			sketches[i].insert(key, val);
			uint32_t result;
			new_data_t tmp_key(key);
			if (heap[i].find(tmp_key) != heap[i].end()) {
				heap[i][tmp_key] += val;
			}
			else {
				result = sketches[i].query(key);
				heap[i].emplace(tmp_key, result);
			}
		}

		// // insert into heap
		// uint32_t result;
		// new_data_t tmp_key(key);
		// for (int i = 0; i < layer; ++i) {
		// 	if (heap[i].find(tmp_key) != heap[i].end()) {
		// 		heap[i][tmp_key] += val;
		// 	}
		// 	else {
		// 		result = sketches[i].query(key);
		// 		heap[i].emplace(tmp_key, result);
		// 	}
		// }
		
	}

	uint32_t query(Key_t key) {
		uint32_t hash_result, result = 0;
		int j;
		for (j = 0; j < layer; ++j) {
			if (j) {
				hash_result = hash(key, j);
				if (!hash_result)
					break;
			}
		}
		j--;

		for (int i = j; i >= 0; --i) {
			// hash_result = hash(key, i);
			// if (!hash_result)
			// 	continue;
			if (!result) {
				result = sketches[i].query(key);
				// result = heap[i][new_data_t(key)];
				// std::cout << result << std::endl;
			}
			else if (result * 2 > sketches[i].query(key)) {
				// std::cout << result << " " << result * 2 << " " << sketches[i].query(key) << std::endl;
				result = result * 2 - sketches[i].query(key);
				// result = result * 2 - heap[i][new_data_t(key)];
			}
			// result = sketches[i].query(key);
			// break;
		}
		return result;
	}

	std::map<new_data_t, uint32_t> get_topK(uint32_t K, int layer_idx) {
		std::map<new_data_t, uint32_t> result;
		std::vector<std::pair<uint32_t, new_data_t>> tmp_result;

		for (auto iter = heap[layer_idx].begin(); iter != heap[layer_idx].end(); ++iter) {
			tmp_result.push_back(std::make_pair(iter->second, iter->first));
		}

		sort(tmp_result.begin(), tmp_result.end());

		for (auto iter = tmp_result.rbegin(); iter != tmp_result.rend(); iter++) {
			result[iter->second] = iter->first;
			if (K-- == 0)
				break;
		}
		return result;
	}

	uint32_t get_cardinality() {
		std::set<new_data_t> s;
		for (int i = layer - 1; i >= 0; --i) {
			std::map<new_data_t, uint32_t> result = get_topK(TOPK, i);
			for (auto iter = result.begin(); iter != result.end(); iter++)
				s.insert(iter->first);
		}
		return s.size();
	}

	double get_entropy() {
		double sum[layer] = {0};
		uint32_t hash_result = 0;
		for (int i = layer - 1; i >= 0; --i) {
			std::map<new_data_t, uint32_t> result = get_topK(TOPK, i);
			for (auto iter = result.begin(); iter != result.end(); iter++) {
				double delta = (iter->second == 0) ? 0 : (iter->second * log2(iter->second));
				if (i == layer - 1) {
					sum[i] += delta;
				}
				else {
					hash_result = hash(iter->first.str(), i + 1);
					if (!hash_result)
						sum[i] += delta;
					else
						sum[i] -= delta;
				}
			}
			if (i != layer - 1) {
				sum[i] = 2 * sum[i + 1] + sum[i];
			}
		}

		

		return log2(element_num) - sum[layer - 1] / element_num;
	}

	size_t get_memory_usage() {
		return sketches[0].get_memory_usage() * layer_num;
	}
};

#endif