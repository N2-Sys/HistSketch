#ifndef _HISTOGRAM_H
#define _HISTOGRAM_H

#include "common.h"

#define HashNum 2

template<typename T>
class bucket {
public:
	T val;
	int8_t idx;
	bool flag;
	bucket() {
		val = 0;
		idx = -1;
		flag = false;
	}
};

template<typename T, int bucket_num>
class histogram {
public:
	bucket<T> buckets[bucket_num];			// bucket id & counter
	uint64_t h[HashNum], s[HashNum], n[HashNum];	// parameters for hash function

	histogram() {
		int index = 0;
		for (int i = 0; i < HashNum; ++i) {
			h[i] = GenHashSeed(index++);
			s[i] = GenHashSeed(index++);
			n[i] = GenHashSeed(index++);
		}
	}

	void reset() {						// used when evict a whole histogram
		memset(buckets, 0, sizeof(buckets));
		for (int i = 0; i < bucket_num; ++i) {
			buckets[i].idx = -1;
			buckets[i].flag = true;
		}
	}

	int insert(uint8_t bid, uint32_t v = 1, bucket<T> &b = bucket<T>(), bool debug = false) {		// b for evicting a bucket
		bucket<T> temp;
		uint32_t temp_pos, pos;
		bool exchange = false;
		/** optimized version of HistSketch **/
		for (int i = 0; i < HashNum; ++i) {
			pos = i * (bucket_num / HashNum) + (uint32_t)(AwareHash((unsigned char *)&bid, 1, h[i], s[i], n[i]) % (bucket_num / HashNum));
			// if (!i) temp_pos = pos;

			if (buckets[pos].idx == bid) {			//in the Layer i
				buckets[pos].val += v;
				assert(buckets[pos].val != std::numeric_limits<T>::max());
				return HIST_HIT;
			}
			else if (buckets[pos].idx == -1) {		//empty slot in the Layer i
				buckets[pos].idx = bid;
				buckets[pos].val = v;
				return HIST_HIT;
			}
			else {									//evict
				if (!i || buckets[pos].val < v) {
					temp.idx = buckets[pos].idx;
					temp.val = buckets[pos].val;
					buckets[pos].idx = bid;
					buckets[pos].val = v;
					bid = temp.idx;
					v = temp.val;
					temp_pos = pos;
					exchange = true;
				}
				// if (!i) {
				// 	temp_pos = pos;
				// 	continue;
				// }
				// if (buckets[pos].val < buckets[temp_pos].val) {
				// 	temp_pos = pos;
				// 	temp.idx = buckets[temp_pos].idx;
				// 	temp.val = buckets[temp_pos].val;
				// 	buckets[temp_pos].idx = bid;
				// 	buckets[temp_pos].val = v;
				// 	// bid = temp.idx;
				// 	// v = temp.val;
				// 	// temp_pos = pos;
				// 	exchange = true;
				// } 
				// if (buckets[pos].val < v && i) {
				// 	stop = true;
				// }
			}
			// if (stop) break;
		}

		if (exchange) {
			b = temp;
			buckets[temp_pos].flag = true;
			return HIST_EVICT;
		}

		return HIST_HIT;

		/** basic version of HistSketch **/
		// buckets[bid].val += v;
		// return HIST_HIT;
	}

	T query(int bid, uint16_t &swap) {
		/** optimized version of HistSketch **/
		T result = 0;
		swap = 0;

		// uint32_t pos1 = (bucket_num / HashNum) + (uint32_t)(AwareHash((unsigned char *)&bid, 1, h[0], s[0], n[0]) % (bucket_num / HashNum));
		uint32_t pos1 = (uint32_t)(AwareHash((unsigned char *)&bid, 1, h[0], s[0], n[0]) % (bucket_num / HashNum));
		uint32_t pos2 = (bucket_num / HashNum) + (uint32_t)(AwareHash((unsigned char *)&bid, 1, h[1], s[1], n[1]) % (bucket_num / HashNum));

		for (int i = 0; i < bucket_num; ++i) {		//check whether in the buckets
			if (buckets[i].idx == bid) {
				result += buckets[i].val;
				// if (buckets[i].flag) {
				// 	swap |= (1 << i);
				// }
			}
		}

		if (result) {
			if (buckets[pos1].flag)
				swap |= (1 << pos1);
			if (buckets[pos2].flag)
				swap |= (1 << pos2);
		}
		

		return result;

		/** basic version of HistSketch **/
		// swap = buckets[bid].flag;
		// return buckets[bid].val;
	}

	T getTotal() {
		T sum = 0;
		for (int i = 0; i < bucket_num; ++i)
			sum += buckets[i].val;
		return sum;
	}

	static size_t get_memory_usage() {
		// cout << "histogram size: " << sizeof(T) * (bucket_num + 1) + bucket_num + bucket_num / 8 << endl;
		// return sizeof(bucket<T>) * bucket_num + sizeof(T) + bucket_num / 8;
		// return sizeof(T) * (bucket_num + 1) + bucket_num / 8;

		/** optimized version of HistSketch **/
		return sizeof(T) * bucket_num + bucket_num / 8 + (int)(log(BUCKET_NUM) / log(2)) * bucket_num / 8;	// counter + flag + idx

		/** basic version of HistSketch **/
		// return bucket_num * sizeof(T);
	}
};

#undef HashNum

#endif