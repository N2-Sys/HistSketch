#ifndef _IBLT_H
#define _IBLT_H

#include "utils.h"
#include "histogram.h"
#include "BloomFilter.h"

template<typename T>
struct Node {
	uint8_t flowNum;
	Histogram<T, BUCKET_NUM> packetNum;
	char flowSum[CHARKEY_LEN];
	Node() {
		memset(flowSum, 0, sizeof(flowSum));
		flowNum = 0;
        packetNum = Histogram<T, BUCKET_NUM>();
	}
	Node & operator ^= (Key_t key) {
		for (int i = 0; i < CHARKEY_LEN; ++i) {
			flowSum[i] ^= key[i];
		}
		return *this;
	}
	size_t get_memory_usage() {
		return sizeof(flowNum) + sizeof(flowSum) + packetNum.get_memory_usage();
	}
};

template<typename T, uint32_t bw, uint32_t bk, uint32_t iw, uint32_t ik>
class IBLT:public SketchBase {
	uint32_t width = iw, hashNum = ik;
	BloomFilter<bw, bk> bf;
	Node<T> *nodes;
	uint64_t h[ik], s[ik], n[ik];

public:
	std::map<five_tuple, Histogram<T, BUCKET_NUM>> result;

	IBLT (std::string na) {
        name = na;
		uint64_t index = 0;
		for (int i = 0; i < hashNum; ++i) {
			h[i] = GenHashSeed(index++);
			s[i] = GenHashSeed(index++);
			n[i] = GenHashSeed(index++);
		}
        // for (int i = 0; i < width; ++i) {
        //     nodes[i] = Node<T>();
        // }
        // memset(nodes, 0, sizeof(nodes));
        nodes = new Node<T>[width];
		result.clear();
	}

	uint32_t hash(Key_t key, int line) {
		return (uint32_t)(AwareHash((unsigned char *)key, CHARKEY_LEN, h[line], s[line], n[line]) % width);
	}

	void insert(Key_t key, uint8_t bid, uint32_t val = 1) {
        uint32_t pos;
		if (bf.getbit(key)) {
			for (int i = 0; i < hashNum; ++i) {
				pos = hash(key, i);
				nodes[pos].packetNum.insert(bid, val);
			}
		}
		else {
			bf.setbit(key);
			for (int i = 0; i < hashNum; ++i) {
				pos = hash(key, i);
				nodes[pos].packetNum.insert(bid, val);
                // nodes[pos].packetNum.buckets[bid] += val;
				nodes[pos].flowNum++;
				nodes[pos] ^= key;
			}
		}
	}

	void dump() {
		five_tuple tmp;
        Histogram<T, BUCKET_NUM> count;

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
                        for (int k = 0; k < BUCKET_NUM; k++) {
                            if (nodes[pos].packetNum.buckets[k] < count.buckets[k]) {
							    count.buckets[k] = nodes[pos].packetNum.buckets[k];
						    }
						    nodes[pos].packetNum.buckets[k] -= count.buckets[k];
                        }
						
					}
					flag = false;
				}	            
			}
			if (flag) break;
		}
	}

	uint32_t pointQuery(Key_t key, uint8_t bid) {
		if (result.empty())
			dump();
		five_tuple tmp = five_tuple(key);
		if (result.find(tmp) != result.end())
			return result[tmp].pointQuery(bid);
		return 0;
	}

    Hist_t histogramQuery(Key_t key) {
        if (result.empty())
            dump();
        five_tuple tmp = five_tuple(key);
        if (result.find(tmp) != result.end())
            return result[tmp].histogramQuery();
        return Hist_t{};
    }

	size_t get_memory_usage() {
		return nodes[0].get_memory_usage() * width + bf.get_memory_usage();
	}
};

#endif