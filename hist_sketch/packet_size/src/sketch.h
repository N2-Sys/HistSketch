#ifndef _SKETCH_H
#define _SKETCH_H

#include "common.h"
#include "HashTable.h"
#include "CMSketch.h"
#include "BloomFilter.h"

template<typename T, int bucket_num, int slot_num, uint32_t d_hist, uint32_t w_hist, uint32_t bf_width, uint32_t bf_hash>
class sketch {
public:
	hashTable<T, bucket_num, slot_num> table;
	CMsketch<uint16_t, d_hist, w_hist, CHARKEY_LEN + 1> lightHist;
	BloomFilter<bf_width, bf_hash, CHARKEY_LEN + 1> bfHist;

	// control plane for histogram
	// std::vector<new_data_t> flowKeysHist;
	new_data_t flowKeysHist[1000005];
	uint32_t flowKeyPtr = 0;
	std::map<five_tuple, std::array<T, BUCKET_NUM>> evictHistResult;
	// std::map<five_tuple, std::map<uint8_t, uint32_t>> evictHistResult;
	// std::map<new_data_t, T> cmControlPlaneHistResult;				// temporarily unuseful
	std::map<five_tuple, std::array<T, BUCKET_NUM>> cmHistResult;	// decoding result for Histogram

	struct statistics {
		uint32_t pointLarge = 0;
		uint32_t pointAll = 0;
		uint32_t histogramLarge = 0;
		uint32_t histogramAll = 0;
		double cardinalityRE = 0;
		double entropyRE = 0;
		uint32_t pointChange = 0;
		uint32_t histogramChange = 0;
	} statistics;

	void insert(Key_t k, uint8_t bid, new_data_t& tmp_key, T v = 1) {
		new_data_t swap_key;
		slot<T, bucket_num> swap_slot;
		T swap_val = 0;

		// bool debug = comkeyHist(k, bid);
		bool debug = false;

		int ret = table.insert(k, bid, swap_key, swap_val, swap_slot, v, debug);

		switch (ret) {
			case HIT_EVICT:				// swap a bucket
			{
				lightHist.insert(swap_key.str, swap_val);
				if (!bfHist.getbit(swap_key.str)) {
					bfHist.setbit(swap_key.str);
					// flowKeysHist.push_back(swap_key);
					flowKeysHist[flowKeyPtr++] = swap_key;
				}
				break;
			}
			case MISS_EVICT:			// swap a flow
			{

				// Comment this if you want to test throughput
				for (int i = 0; i < bucket_num; ++i) {
					if (swap_slot.hist.buckets[i].idx == -1)
						continue;
						evictHistResult[swap_slot.key][swap_slot.hist.buckets[i].idx] += swap_slot.hist.buckets[i].val;
				}
				
				break;
			}
			case MISS_INSERT:			//just insert into the light part
			{
				lightHist.insert(tmp_key.str, v);
				if (!bfHist.getbit(tmp_key.str)) {
					bfHist.setbit(tmp_key.str);
					// flowKeysHist.push_back(tmp_key);
					flowKeysHist[flowKeyPtr++] = tmp_key;
				}

				// std::random_device rd;  // Will be used to obtain a seed for the random number engine
				// std::mt19937 gen(rd()); // Standard mersenne_twister_engine seeded with rd()
				// std::uniform_real_distribution<> dis(0.0, 1.0);
				// double probability = dis(gen);
				// if (probability > CONTROL_PLANE_PRO) {
				// 	lightNZE.insert(k, v);
				// 	if (!bfNZE.getbit(k)) {
				// 		bfNZE.setbit(k);
				// 		flowKeys.push_back(five_tuple(k));
				// 	}
				// }
				// else {
				// 	five_tuple traceKey = five_tuple(k);
				// 	if (cmControlPlaneResult.find(traceKey) != cmControlPlaneResult.end()) {
				// 		cmControlPlaneResult[traceKey] += v;
				// 	}
				// 	else {
				// 		cmControlPlaneResult.emplace(traceKey, v);
				// 	}
				// }

				break;
			}
		}
	}

	void solve_equations(std::map<five_tuple, std::array<T, BUCKET_NUM> > & mp) {
		if (!mp.empty())
			return;

		uint32_t w = lightHist.width;
		int M = d_hist * w, N;

		// N = flowKeysHist.size();
		N = flowKeyPtr;

		Eigen::VectorXd X(N), b(M);
		Eigen::SparseMatrix<double> A(M, N);
		std::vector<Eigen::Triplet<double>> tripletlist;

		auto t_a = std::chrono::high_resolution_clock::now();
		
		for (int i = 0, j; i < d_hist; i++) {
			j = 0;
			// for (std::vector<new_data_t>::iterator it = flowKeysHist.begin(); it != flowKeysHist.end(); ++it, ++j) {
			for (int k = 0; k < N; ++k, ++j) {
				// int idx = i * w + lightHist.hash(it->str, i);
				int idx = i * w + lightHist.hash(flowKeysHist[k].str, i);
				//按Triplet方式填充，速度快
				tripletlist.push_back(Eigen::Triplet<double>(idx, j, 1));
			}
			for (j = 0; j < w; ++j) {
				b(i * w + j) = lightHist.matrix[i][j];
			}
		}
		
		A.setFromTriplets(tripletlist.begin(), tripletlist.end());

		// 压缩优化矩阵
		A.makeCompressed();
		Eigen::LeastSquaresConjugateGradient<Eigen::SparseMatrix<double>> Solver_sparse;
		Solver_sparse.compute(A);
		X = Solver_sparse.solve(b);

		auto t_b = std::chrono::high_resolution_clock::now();
		std::cout << "Solved time: " << std::chrono::duration_cast<std::chrono::microseconds>(t_b - t_a).count() << "us" << std::endl;

		for (int i = 0; i < N; ++i) {
			auto iter = mp.find(flowKeysHist[i].key);
			int ans;
			if (iter == mp.end()) {
				std::array<T, BUCKET_NUM> arr{};
				for (int j = 0; j < BUCKET_NUM; ++j)
					arr.at(j) = 0;
				ans = (int)(X(i) + 0.5);
				arr.at(flowKeysHist[i].bid) = (ans <= 0)?1:ans;
				mp.emplace(flowKeysHist[i].key, arr);
			} else {
				ans = (int)(X(i) + 0.5);
				assert(iter->second.at(flowKeysHist[i].bid) == 0);
				iter->second.at(flowKeysHist[i].bid) = (ans <= 0)?1:ans;
			}
		}
	}

	T pointQuery(Key_t k, uint8_t bid) {
		uint16_t swap = 0;
		T result = 0;

		bool debug = false;

		// Hash table result
		result = table.query(k, bid, swap);
		if (debug)
			std::cout << "hash table result:" << result << std::endl;

		// CM result
		if (swap || result == 0) {
		#ifdef SOLVED
			solve_equations(cmHistResult);
			five_tuple tmp = five_tuple(k);
			if (cmHistResult.find(tmp) != cmHistResult.end()) {
				result += cmHistResult[tmp].at(bid);
				if (debug)
					std::cout << "cm Result:" << cmHistResult[tmp].at(bid) << std::endl;
			}
		#else
			if (bfHist.getbit(new_data_t(k, bid).str))
				result += lightHist.query(new_data_t(k, bid).str);
		#endif
		}

		// control plane result
		five_tuple tmpKey(k);
		if (evictHistResult.find(tmpKey) != evictHistResult.end()) {
			result += evictHistResult[tmpKey][bid];
			if (debug)
				std::cout << "control plane result:" << evictHistResult[tmpKey][bid] << std::endl;
		}

		// std::cout << std::endl;
		return result;
	}

	Hist_t histogramQuery(Key_t key) {
		Hist_t result{};
		for (int i = 0; i < BUCKET_NUM; ++i) {
			result.at(i) = pointQuery(key, i);
		}
		return result;
	}

	uint32_t get_cardinality() {
	#ifndef SOLVED
		uint32_t cardinality = lightHist.get_cardinality();

		for (int i = 0; i < slot_num; ++i) {
			five_tuple cur_five_tuple = table.slots[i].key;
			if (cur_five_tuple.isEmpty())
				continue;

			Hist_t table_result{};
			std::array<bool, BUCKET_NUM> table_flag{};
			// // initialize
			// for (int j = 0; j < BUCKET_NUM; ++j) {
			// 	table_result.at(j) = 0;
			// 	table_flag.at(j) = false;
			// }

			for (int j = 0; j < bucket_num; ++j) {
				if (table.slots[i].hist.buckets[j].idx == -1)
					continue;
				table_result.at(table.slots[i].hist.buckets[j].idx) += table.slots[i].hist.buckets[j].val;
				table_flag.at(table.slots[i].hist.buckets[j].idx) |= table.slots[i].hist.buckets[j].flag;
			}

			for (int j = 0; j < BUCKET_NUM; ++j) {
				new_data_t cur_key((Key_t)cur_five_tuple.str, (uint8_t)j);
				T cur_val = table_result.at(j);
				uint32_t light_result = 0;

				if (evictHistResult.find(cur_five_tuple) != evictHistResult.end())
					cur_val += evictHistResult[cur_five_tuple][j];

				if (bfHist.getbit(cur_key.str))
					light_result = lightHist.query(cur_key.str);

				if (table_flag.at(j) && light_result) {
					cur_val += light_result;
					cardinality--;
				}

				if (cur_val) {
					cardinality++;
				}
			}
		}
	#else
		uint32_t cardinality = 0;
		std::map<five_tuple, std::map<uint8_t, uint32_t>> final_result;

		solve_equations(cmHistResult);
		for (auto iter = cmHistResult.begin(); iter != cmHistResult.end(); iter++) {
			for (int i = 0; i < BUCKET_NUM; ++i) {
				if (iter->second.at(i) != 0) {
					final_result[iter->first][(uint8_t)i] += iter->second.at(i);
				}
			}
		}

		for (auto iter = evictHistResult.begin(); iter != evictHistResult.end(); iter++) {
			// for (auto iter2 = iter->second.begin(); iter2 != iter->second.end(); iter2++)
				// final_result[iter->first][iter2->first] += iter2->second;
			for (int i = 0; i < BUCKET_NUM; ++i)
				if (iter->second.at(i))
					final_result[iter->first][i] += iter->second.at(i);
		}

		for (int i = 0; i < SLOT_NUM; ++i) {
			for (int j = 0; j < bucket_num; ++j) {
				if (table.slots[i].hist.buckets[j].idx == -1) 
					continue;
				final_result[table.slots[i].key][table.slots[i].hist.buckets[j].idx] += table.slots[i].hist.buckets[j].val;
			}
		}

		for (auto iter = final_result.begin(); iter != final_result.end(); iter++) {
			cardinality += iter->second.size();
		}
	#endif

		return cardinality;
	}

	double get_entropy() {
	#ifndef SOLVED
		std::pair<uint32_t, double> entropy = lightHist.get_entropy();
		
		for (int i = 0; i < slot_num; ++i) {
			five_tuple cur_five_tuple = table.slots[i].key;
			if (cur_five_tuple.isEmpty())
				continue;

			Hist_t table_result{};
			std::array<bool, BUCKET_NUM> table_flag{};

			// initialize
			for (int j = 0; j < BUCKET_NUM; ++j) {
				table_result.at(j) = 0;
				table_flag.at(j) = false;
			}

			for (int j = 0; j < bucket_num; ++j) {
				if (table.slots[i].hist.buckets[j].idx == -1)
					continue;
				table_result.at(table.slots[i].hist.buckets[j].idx) += table.slots[i].hist.buckets[j].val;
				table_flag.at(table.slots[i].hist.buckets[j].idx) |= table.slots[i].hist.buckets[j].flag;
			}

			for (int j = 0; j < BUCKET_NUM; ++j) {
				new_data_t cur_key((Key_t)cur_five_tuple.str, j);
				T cur_val = table_result.at(j);
				uint32_t light_result = 0;

				if (evictHistResult.find(cur_five_tuple) != evictHistResult.end())
					cur_val += evictHistResult[cur_five_tuple][j];

				if (bfHist.getbit(cur_key.str))
					light_result = lightHist.query(cur_key.str);

				if (table_flag.at(j) && light_result) {
					cur_val += light_result;
					entropy.first -= light_result;
					entropy.second -= light_result * log2(light_result);
				}

				if (cur_val) {
					entropy.first += cur_val;
					entropy.second += cur_val * log2(cur_val);
				}
			}
		}
		return -entropy.second / entropy.first + log2(entropy.first);
	#else
		double entropy = 0;
		uint32_t total = 0;
		std::map<five_tuple, std::map<uint8_t, uint32_t>> final_result;

		solve_equations(cmHistResult);
		for (auto iter = cmHistResult.begin(); iter != cmHistResult.end(); iter++) {
			for (int i = 0; i < BUCKET_NUM; ++i) {
				if (iter->second.at(i) != 0) {
					final_result[iter->first][(uint8_t)i] += iter->second.at(i);
				}
			}
		}

		for (auto iter = evictHistResult.begin(); iter != evictHistResult.end(); iter++) {
			// for (auto iter2 = iter->second.begin(); iter2 != iter->second.end(); iter2++)
			// 	final_result[iter->first][iter2->first] += iter2->second;
			for (int i = 0; i < BUCKET_NUM; ++i)
				if (iter->second.at(i))
					final_result[iter->first][i] += iter->second.at(i);
		}

		for (int i = 0; i < SLOT_NUM; ++i) {
			for (int j = 0; j < bucket_num; ++j) {
				if (table.slots[i].hist.buckets[j].idx == -1) 
					continue;
				final_result[table.slots[i].key][table.slots[i].hist.buckets[j].idx] += table.slots[i].hist.buckets[j].val;
			}
		}

		for (auto iter = final_result.begin(); iter != final_result.end(); iter++) {
			for (auto iter2 = iter->second.begin(); iter2 != iter->second.end(); iter2++) {
				total += iter2->second;
				entropy += iter2->second * log2(iter2->second);
			}
		}
		return -entropy / total + log2(total);
	#endif
		
	}

	size_t get_memory_usage() {
		std::cout << "\nHash table:" << table.get_memory_usage() << std::endl;
		std::cout << "CM Sketch:" << lightHist.get_memory_usage() << std::endl;
		std::cout << "Bloom Filter:" << bfHist.get_memory_usage() << std::endl;
		return table.get_memory_usage() + lightHist.get_memory_usage() + bfHist.get_memory_usage();
	}
};

#endif