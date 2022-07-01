#ifndef _CM_HEAP_H
#define _CM_HEAP_H

#include "MRBP.h"

template<uint32_t d, uint32_t w>
class CMHeap {
public:
    uint32_t depth = d, width = w, heap_cnt = 0;
    uint32_t b, bb, c;
    std::vector<std::vector<MRBP>> matrix;
    std::map<Key_t, uint32_t> heap;
    std::set<Key_t> result;
    uint64_t h[d + 1], s[d + 1], n[d + 1];

    CMHeap(uint32_t _b, uint32_t _bb, uint32_t _c):b(_b), bb(_bb), c(_c) {
        matrix = std::vector<std::vector<MRBP>>(depth, std::vector<MRBP>(width, MRBP(_b, _bb, _c)));
        heap.clear();
        int index = 0;
		for (int i = 0; i < d + 1; ++i) {
			h[i] = GenHashSeed(index++);
			s[i] = GenHashSeed(index++);
			n[i] = GenHashSeed(index++);
		}
    }

    uint32_t hash(Key_t key, int line) {
		return (uint32_t)(AwareHash((unsigned char *)&key, 4, h[line], s[line], n[line]) % width);
	}

	uint32_t hash2(Key_t key, Key_t dst) {
		uint64_t tmpKey = ((uint64_t)key << 32) | dst;
		return (uint32_t)AwareHash((unsigned char *)&tmpKey, 8, h[d], s[d], n[d]);
	}

    void update(Key_t key, Key_t dst) {
        uint32_t pos, hash_val, min_val = std::numeric_limits<uint32_t>::max(), min_card = std::numeric_limits<uint32_t>::max();
        for (int i = 0; i < depth; ++i) {
            pos = hash(key, i);
            hash_val = hash2(key, dst);
            matrix[i][pos].update(hash_val);
            min_val = std::min(min_val, matrix[i][pos].query());
        }

        if (heap.find(key) != heap.end()) {
            heap[key] = min_val;
        }
        else if (heap_cnt < HEAP_COUNT) {
            heap[key] = min_val;
            heap_cnt++;
        }
        else {
            std::map<Key_t, uint32_t>::iterator min_it;
            for (auto it = heap.begin(); it != heap.end(); it++) {
                if (it->second < min_card) {
                    min_card = it->second;
                    min_it = it;
                }
            }
            if (min_val > min_card) {
                heap.erase(min_it->first);
                heap.emplace(key, min_val);
            }
        }
    }

    void get_superspreaders() {
        for (auto it = heap.begin(); it != heap.end(); ++it) {
            if (it->second > SS_ESTIMATE_THRESHOLD) {
                result.insert(it->first);
            }
        }
    }

    size_t get_memory_usage() {
        return depth * width * matrix[0][0].get_memory_usage() + HEAP_COUNT * 8;
    }
};

#endif