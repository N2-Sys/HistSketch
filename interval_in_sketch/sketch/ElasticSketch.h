#ifndef _ELASTICSKETCH_H
#define _ELASTICSKETCH_H

#include "CMSketch.h"
#include <iostream>

template<typename T>
struct Bucket {
	new_data_t key[COUNTER_PER_BUCKET];
	T val[COUNTER_PER_BUCKET];
	T negative_vote;
	uint8_t swap_flag;
};

enum strategy {
	TO_HASH,
	EVICT_TO_CM,
	DIRECT_TO_CM
};

template<typename T, int slot_num>
class HeavyPart {
	public:
		Bucket<T> buckets[slot_num];
		uint64_t h, s, n;

		HeavyPart(){
			memset(buckets, 0, sizeof(Bucket<T>) * slot_num);
			int index = 0;
			h = GenHashSeed(index++);
			s = GenHashSeed(index++);
			n = GenHashSeed(index++);
		}

		uint32_t hash(Key_t key) {
			return (uint32_t)(AwareHash((unsigned char *)key, CHARKEY_LEN + 1, h, s, n) % slot_num);
		}

		/* insertion */
		int insert(Key_t key, new_data_t &swap_key, T &swap_val, bool &swap, uint32_t val = 1) {
			int matched = -1, empty = -1, min_counter = 0;
			uint32_t min_counter_val = std::numeric_limits<uint32_t>::max();
			new_data_t tmp_key(key);

			/* find if there has matched bucket */
			uint32_t pos = hash(key);
			for(int i = 0; i < COUNTER_PER_BUCKET; i++) {
				if(buckets[pos].key[i] == tmp_key) {
					matched = i;
					break;
				}
				if(buckets[pos].key[i].isEmpty() && empty == -1)
					empty = i;
				if(min_counter_val > buckets[pos].val[i]) {
					min_counter = i;
					min_counter_val = buckets[pos].val[i];
				}
			}

			/* if matched */
			if(matched != -1) {
				buckets[pos].val[matched] += val;
				return TO_HASH;
			}

			/* if there has empty bucket */
			if(empty != -1) {
				buckets[pos].key[empty] = tmp_key;
				buckets[pos].val[empty] = val;
				return TO_HASH;
			}

			/* check whether to evict */
			buckets[pos].negative_vote += val;

			/* not evict */
			if((min_counter_val << 3) > buckets[pos].negative_vote)
				return DIRECT_TO_CM;

			/* evict */
			swap_key = buckets[pos].key[min_counter];
			swap_val = buckets[pos].val[min_counter];
			swap = buckets[pos].swap_flag & (1 << min_counter);

			buckets[pos].negative_vote = 0;
			buckets[pos].key[min_counter] = tmp_key;
			buckets[pos].val[min_counter] = val;
			buckets[pos].swap_flag |= (1 << min_counter);

			return EVICT_TO_CM;
		}

		/* query */
		uint32_t query(Key_t key, bool &swap) {
			uint32_t pos = hash(key);

			new_data_t tmp_key(key);
			for(int i = 0; i < COUNTER_PER_BUCKET; ++i) {
				if(buckets[pos].key[i] == tmp_key) {
					swap = buckets[pos].swap_flag & (1 << i);
					return buckets[pos].val[i];
				}
			}
			return 0;
		}

		/* interface */
		size_t get_memory_usage() {
			return slot_num * (COUNTER_PER_BUCKET * (sizeof(T) + CHARKEY_LEN) + 1 + sizeof(T));
		}
};

template<typename T, int slot_num, int d, int w>
class ElasticSketch {
    HeavyPart<uint32_t, slot_num> heavy_part;
    CMSketch<T, d, w> light_part;

    public:
        void insert(Key_t key, int val = 1, bool debug = false) {
            new_data_t swap_key;
            uint32_t swap_val = 0;
            bool swap = 0;

            int result = heavy_part.insert(key, swap_key, swap_val, swap, val);

            switch(result) {
                case TO_HASH:
                    return;
                case EVICT_TO_CM: {
                    if(swap)
                        light_part.insert(swap_key.str(), swap_val);
                    else
                        light_part.swap_insert(swap_key.str(), swap_val);
                    return;
                }
                case DIRECT_TO_CM: {
                    light_part.insert(key, val);
                    return;
                }
                default:
                    printf("error return value !\n");
                    exit(1);
            }
        }

        uint32_t query(Key_t key) {
            bool swap = 0;
            uint32_t heavy_result = heavy_part.query(key, swap);
            uint32_t light_result = 0;
            if(heavy_result == 0 || swap) {
                light_result = light_part.query(key);
             //    if (debug) {
	            // 	std::cout << heavy_result << " " << light_result << " " << std::endl;
	            // }
                return heavy_result + light_result;
            }
            
            return heavy_result;
        }

		double get_entropy() {
			std::pair<uint32_t, double> light_entropy = light_part.get_entropy();
			for (int i = 0; i < slot_num; ++i) {
				for (int j = 0; j < COUNTER_PER_BUCKET; ++j) {
					new_data_t cur_key = heavy_part.buckets[i].key[j];
					if (cur_key.isEmpty())
						continue;
					
					T cur_val = heavy_part.buckets[i].val[j];
					
					uint32_t light_result = light_part.query(cur_key.str());

					if ((heavy_part.buckets[i].swap_flag & (1 << j)) && light_result) {
						cur_val += light_result;
						light_entropy.first -= light_result;
						light_entropy.second -= light_result * log2(light_result);
					}

					if (cur_val) {
						light_entropy.first += cur_val;
						light_entropy.second += cur_val * log2(cur_val);
					}
				}
			}
			return - light_entropy.second / light_entropy.first + log2(light_entropy.first);
		}

		uint32_t get_cardinality() {
			uint32_t cardinality = light_part.get_cardinality();
			for (int i = 0; i < slot_num; ++i) {
				for (int j = 0; j < COUNTER_PER_BUCKET; ++j) {
					new_data_t cur_key = heavy_part.buckets[i].key[j];
					if (cur_key.isEmpty())
						continue;
					
					T cur_val = heavy_part.buckets[i].val[j];

					uint32_t light_result = light_part.query(cur_key.str());

					if ((heavy_part.buckets[i].swap_flag & (1 << j)) && light_result) {
						cur_val += light_result;
						cardinality--;
					}

					if (cur_val) {
						cardinality++;
					}
				}
			}
			return cardinality;
		}

        uint32_t get_memory_usage() {
            std::cout << heavy_part.get_memory_usage() << " " << light_part.get_memory_usage() << std::endl;
            return heavy_part.get_memory_usage() + light_part.get_memory_usage();
        }
};

#endif
