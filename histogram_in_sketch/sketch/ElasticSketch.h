#ifndef _ELASTICSKETCH_H
#define _ELASTICSKETCH_H

#include "CMSketch.h"
#include "parameters.h"

template<typename T>
struct Bucket {
	five_tuple key[COUNTER_PER_BUCKET];
	Histogram<T, BUCKET_NUM> val[COUNTER_PER_BUCKET];
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
		Bucket<T> slots[slot_num];
		uint64_t h, s, n;

		HeavyPart(){
			int index = 0;
			h = GenHashSeed(index++);
			s = GenHashSeed(index++);
			n = GenHashSeed(index++);
		}

		uint32_t hash(Key_t key) {
			return (uint32_t)(AwareHash((unsigned char *)key, CHARKEY_LEN, h, s, n) % slot_num);
		}

		/* insertion */
		int insert(Key_t key, uint8_t bid, five_tuple &swap_key, Histogram<T, BUCKET_NUM> &swap_val, bool &swap, uint32_t val = 1) {
			int matched = -1, empty = -1, min_counter = 0;
			T min_counter_val = std::numeric_limits<T>::max();
			five_tuple tmp_key(key);

			/* find if there has matched bucket */
			uint32_t pos = hash(key);
			uint32_t total = 0;
			for(int i = 0; i < COUNTER_PER_BUCKET; i++) {
				if(slots[pos].key[i] == tmp_key) {
					matched = i;
					break;
				}
				if(slots[pos].key[i].isEmpty() && empty == -1)
					empty = i;
				total = slots[pos].val[i].getTotal();
				if(min_counter_val > total) {
					min_counter = i;
					min_counter_val = total;
				}
			}
			
			/* if matched */
			if(matched != -1) {
				slots[pos].val[matched].insert(bid, val);
				return TO_HASH;
			}

			/* if there has empty bucket */
			if(empty != -1) {
				slots[pos].key[empty] = tmp_key;
				slots[pos].val[empty].insert(bid, val);
				return TO_HASH;
			}

			/* check whether to evict */
			slots[pos].negative_vote += val;

			/* not evict */
			if((min_counter_val << 3) >= slots[pos].negative_vote)
				return DIRECT_TO_CM;

			/* evict */
			swap_key = slots[pos].key[min_counter];
			swap_val = slots[pos].val[min_counter];
			swap = slots[pos].swap_flag & (1 << min_counter);

			slots[pos].negative_vote = 0;
			slots[pos].key[min_counter] = tmp_key;
			slots[pos].val[min_counter] = Histogram<T, BUCKET_NUM>();
			slots[pos].val[min_counter].insert(bid, val);
			slots[pos].swap_flag |= (1 << min_counter);

			return EVICT_TO_CM;
		}

		/* query */
		uint32_t pointQuery(Key_t key, uint8_t bid, bool &swap) {
			uint32_t pos = hash(key);
			five_tuple tmp_key(key);
			for(int i = 0; i < COUNTER_PER_BUCKET; ++i) {
				if (slots[pos].key[i] == tmp_key) {
					swap = slots[pos].swap_flag & (1 << i);
					return slots[pos].val[i].pointQuery(bid);
				}
			}
			return 0;
		}

		Hist_t histogramQuery(Key_t key, bool &swap) {
        	Hist_t result{};
        	uint32_t pos = hash(key);
        	for (int i = 0; i < COUNTER_PER_BUCKET; ++i) {
        		if (slots[pos].key[i] == five_tuple(key)) {
        			result = slots[pos].val[i].histogramQuery();
        			break;
        		}
        	}
        	return result;
        }

		/* interface */
		size_t get_memory_usage() {
			return slot_num * (COUNTER_PER_BUCKET * (Histogram<T, BUCKET_NUM>::get_memory_usage() + CHARKEY_LEN) + 1 + sizeof(T));
		}
};

template<typename T, int slot_num, int d, int w>
class ElasticSketch:public SketchBase {
    HeavyPart<T, slot_num> heavy_part;
    CMSketch<T, d, w> light_part = CMSketch<T, d, w>("CM");

    public:
    	ElasticSketch(std::string na) {
    		name = na;
    	}

        void insert(Key_t key, uint8_t bid, uint32_t val = 1) {
            five_tuple swap_key;
            Histogram<T, BUCKET_NUM> swap_val;
            bool swap = 0;

            int result = heavy_part.insert(key, bid, swap_key, swap_val, swap, val);

            switch(result) {
                case TO_HASH:
                    return;
                case EVICT_TO_CM: {
                    if(swap) {
                    	// Hist_t tmp_hist = swap_val.histogramQuery();
	                    light_part.insert_hist(swap_key.str, swap_val);
                        // light_part.insert(swap_key.str, swap_val);
                    }
                    else {
                        light_part.swap_insert(swap_key.str, swap_val);
                    }
                    return;
                }
                case DIRECT_TO_CM: {
                    light_part.insert(key, bid, val);
                    return;
                }
                default:
                    printf("error return value !\n");
                    exit(1);
            }
        }

        uint32_t pointQuery(Key_t key, uint8_t bid) {
            bool swap = false;
            uint32_t heavy_result = heavy_part.pointQuery(key, bid, swap);
            if(heavy_result == 0 || swap) {
                int light_result = light_part.pointQuery(key, bid);
                return heavy_result + light_result;
            }
            return heavy_result;
        }

        Hist_t histogramQuery(Key_t key) {
        	bool swap = false, empty = true;
        	Hist_t heavy_result = heavy_part.histogramQuery(key, swap);
        	for (int i = 0; i < BUCKET_NUM; ++i) {
        		if (heavy_result.at(i) != 0) {
        			empty = false;
        			break;
        		}
        	}
        	if (empty || swap) {
        		Hist_t light_result = light_part.histogramQuery(key);
        		for (int i = 0; i < BUCKET_NUM; ++i)
        			heavy_result.at(i) += light_result.at(i);
        	}
        	return heavy_result;
        }

        size_t get_memory_usage() {
            return heavy_part.get_memory_usage() + light_part.get_memory_usage();
        }
};

#endif
