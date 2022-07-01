#ifndef _HEAVY_H
#define _HEAVY_H

#include "histogram.h"
#include "common.h"
#include "utils.h"

template<typename T, int bucket_num>
class slot {
public:
	five_tuple key;					// flowkey
	histogram<T, bucket_num> hist;	// histogram
	T negativeCounter;				// negative counter

	// slot(five_tuple k, uint8_t bid, T v = 0) {
	// 	key = k;
	// 	hist.insert(bid, v);
	// 	negativeCounter = 0;
	// }


	slot(Key_t k) {
		key = five_tuple(k);
		negativeCounter = 0;
	}

	slot() {
		negativeCounter = 0;
	}

	static size_t get_memory_usage() {
		return sizeof(T) + CHARKEY_LEN + histogram<T, bucket_num>::get_memory_usage();
	}
};

template <typename T, int bucket_num, int size>
class hashTable {
public:
	/* key + histogram + negative_vote */
	slot<T, bucket_num> *slots;
	int nonempty;
	uint32_t slot_num;
	uint64_t h, s, n;

	hashTable() {
		int index = 0;
		h = GenHashSeed(index++);
		s = GenHashSeed(index++);
		n = GenHashSeed(index++);
		nonempty = 0;
		slot_num = calc_next_prime(size);
		slots = new slot<T, bucket_num>[slot_num];
	}

	uint32_t calPos(Key_t key) {
		return (uint32_t)(AwareHash((unsigned char *)key, CHARKEY_LEN, h, s, n) % slot_num);
	}

	int insert(Key_t k, uint8_t bid, new_data_t &swap_key, T &swap_val, slot<T, bucket_num> &swap_slot,
				uint32_t v = 1, bool debug = false)					// v: number of packets / pakcet length
	{
		five_tuple tempKey = five_tuple(k);
		uint32_t pos = calPos(k);
		bucket<T> b;

		// if (compareKeyHist(k, bid)) {
		// 	uint16_t swap = 0;
		// 	std::cout << slots[pos].hist.query(bid, swap) << std::endl;
		// }
		
		if (slots[pos].key == tempKey) {
			int ret = slots[pos].hist.insert(bid, v, b, false);
			if (ret == HIST_EVICT) {									// evict a bucket of the histogram
				swap_key = new_data_t(k, b.idx);
				swap_val = b.val;
				// swap = 1;
				// if (debug)
				// 	std::cout << "HIT_EVICT\n";
				return HIT_EVICT;
			}
			// if (debug)
			// 	std::cout << "HIT (same key)\n";
			return HIT;
		}
		else if (slots[pos].key.isEmpty()) {
			slots[pos].key = tempKey;
			slots[pos].hist.insert(bid, v, b, false);
			nonempty++;
			// if (debug)
			// 	std::cout << "HIT (an empty slot) " << pos << std::endl;
			return HIT;
		}
		else {
			slots[pos].negativeCounter += v;
			T total = slots[pos].hist.getTotal();
			if (slots[pos].negativeCounter / total >= EVICT_THRESHOLD) {	// evict the flow
				swap_slot = slots[pos];
				slots[pos] = slot<T, bucket_num>(k);
				// for (int i = 0; i < bucket_num; ++i) {
				// 	if (swap_slot.flag[i])
				// 		swap |= (1 << i);
				// }
				slots[pos].hist.reset();
				slots[pos].hist.insert(bid, v, b);
				// if (debug)
				// 	std::cout << "MISS_EVICT\n";
				return MISS_EVICT;
			}
		}

		// if (debug)
		// 	std::cout << "MISS_INSERT\n";

		return MISS_INSERT;
	}

	T query(Key_t k, uint8_t bid, uint16_t &swap) {
		five_tuple tempKey(k);
		uint32_t pos = calPos(k);
		swap = 0;

		if (tempKey == slots[pos].key) {
			T result = slots[pos].hist.query(bid, swap);
			return result;
		}
		else {
			return 0;
		}
	}

	double calLoadRate() {
		return (double)nonempty / slot_num;
	}

	size_t get_memory_usage() {
		return slot_num * slot<T, bucket_num>::get_memory_usage();
	}
};

#endif