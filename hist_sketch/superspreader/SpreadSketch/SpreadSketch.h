#ifndef _SPREAD_SKETCH_H
#define _SPREAD_SKETCH_H

#include "MRBP.h"

class Bucket {
public:
	MRBP v;
	Key_t candidate;
	uint32_t l;
	Bucket(uint32_t b, uint32_t bb, uint32_t c) {
		v = MRBP(b, bb, c);
		l = candidate = 0;
	}

	size_t get_memory_usage() {
		return v.get_memory_usage() + sizeof(Key_t) + sizeof(uint32_t);
	}
};

template<uint32_t d, uint32_t w>
class SpreadSketch {
	std::vector<std::vector<Bucket>> buckets;
	uint32_t depth = d, width = w;
	uint64_t h[d + 1], s[d + 1], n[d + 1];
	uint32_t b, bb, c;
public:
	std::set<Key_t> result;

	SpreadSketch(uint32_t _b, uint32_t _bb, uint32_t _c):b(_b), bb(_bb), c(_c) {
		buckets = std::vector<std::vector<Bucket>>(d, std::vector<Bucket>(w, Bucket(_b, _bb, _c)));
		result.clear();
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

	void update(Key_t k, Key_t dst) {
		uint32_t x = hash2(k, dst);
		for (int i = 0; i < d; ++i) {
			uint32_t pos = hash(k, i);
			uint32_t level = buckets[i][pos].v.update(x);
			if (level >= buckets[i][pos].l) {
				buckets[i][pos].l = level;
				buckets[i][pos].candidate = k;
			}
		}
	}

	void get_superspreaders() {
		std::set<Key_t> s;
		s.clear();
		for (int i = 0; i < d; ++i) {
			for (int j = 0; j < w; ++j) {
				Key_t key = buckets[i][j].candidate;
				s.insert(key);
			}
		}

		for (auto i = s.begin(); i != s.end(); ++i) {
			if (*i == 0) continue;
			uint32_t res = std::numeric_limits<uint32_t>::max();
			for (int j = 0; j < d; ++j) {
				uint32_t pos = hash(*i, j);
				res = std::min(res, buckets[j][pos].v.query());
			}
			if (res > SS_ESTIMATE_THRESHOLD)
				result.insert(*i);
		}
	}

	size_t get_memory_usage() {
		return d * w * buckets[0][0].get_memory_usage();
	}
};

#endif