#ifndef _IBLT_H
#define _IBLT_H

#include "common.h"
#include "utils.h"
#include "BloomFilter.h"

template<typename T>
struct Node {
	uint8_t flowNum;
	T packetNum;
	char flowSum[CHARKEY_LEN];
	Node() {
		memset(flowSum, 0, sizeof(flowSum));
		flowNum = packetNum = 0;
	}
	Node & operator ^= (Key_t key) {
		for (int i = 0; i < CHARKEY_LEN; ++i) {
			flowSum[i] ^= key[i];
		}
		return *this;
	}
	size_t get_memory_usage() {
		return sizeof(flowNum) + sizeof(packetNum) + sizeof(flowSum);
	}
};

template<typename T, uint32_t bw, uint32_t bk, uint32_t iw, uint32_t ik>
class IBLT {
	uint32_t width = iw, hashNum = ik;
	BloomFilter<bw, bk> bf;
	Node<T> nodes[iw];
	uint64_t h[ik], s[ik], n[ik];

public:
	std::map<five_tuple, uint32_t> result;

	IBLT () {
		uint64_t index = 0;
		for (int i = 0; i < hashNum; ++i) {
			h[i] = GenHashSeed(index++);
			s[i] = GenHashSeed(index++);
			n[i] = GenHashSeed(index++);
		}
		memset(nodes, 0, sizeof(nodes));
		result.clear();
	}

	uint32_t hash(Key_t key, int line) {
		return (uint32_t)(AwareHash((unsigned char *)key, CHARKEY_LEN, h[line], s[line], n[line]) % width);
	}

	void insert(Key_t key, uint32_t val = 1) {
		if (bf.getbit(key)) {
			for (int i = 0; i < hashNum; ++i) {
				uint32_t pos = hash(key, i);
				nodes[pos].packetNum += val;
			}
		}
		else {
			bf.setbit(key);
			for (int i = 0; i < hashNum; ++i) {
				uint32_t pos = hash(key, i);
				nodes[pos].packetNum += val;
				nodes[pos].flowNum++;
				nodes[pos] ^= key;
			}
		}
	}

	void dump() {
		five_tuple tmp;
        uint32_t count;
        // std::cout << "Dump\n";

		while (1) {
		    bool flag = true;
			for (int i = 0; i < width; ++i) {
				if (nodes[i].flowNum == 1) {
	                tmp = five_tuple(nodes[i].flowSum);
					count = nodes[i].packetNum;

					if (result.find(tmp) == result.end()) {
						result[tmp] = count;
					}
					else {
						std::cout << "already exists" << std::endl;
						continue;
					}

					for (int j = 0; j < hashNum; j++) {
						uint32_t pos = hash(tmp.str, j);
						nodes[pos].flowNum -= 1;
						nodes[pos] ^= tmp.str;
						if (nodes[pos].packetNum < count) {
							count = nodes[pos].packetNum;
						}
						nodes[pos].packetNum -= count;
					}
					flag = false;
				}	            
			}
			if (flag) break;
		}
	}

	uint32_t query(Key_t key) {
		if (result.empty())
			dump();
		five_tuple tmp = five_tuple(key);
		if (result.find(tmp) != result.end())
			return result[tmp];
		return 0;
	}

	size_t get_memory_usage() {
		// std::cout << bf.get_memory_usage() << " " << nodes[0].get_memory_usage() << std::endl;
		return nodes[0].get_memory_usage() * width + bf.get_memory_usage();
	}
};

#endif