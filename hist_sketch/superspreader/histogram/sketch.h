#ifndef _SKETCH_H
#define _SKETCH_H

#include "common.h"
#include "HashTable.h"
#include "CMSketch.h"
#include "BloomFilter.h"

uint32_t hist_card, hist_total;

template<typename T, int bucket_num, int slot_num, uint32_t d_hist, uint32_t w_hist, uint32_t bf_width, uint32_t bf_hash>
class sketch {
public:
	hashTable<T, bucket_num, slot_num> table;
	CMsketch<uint16_t, d_hist, w_hist, INTKEY_LEN + 2> lightHist;
	BloomFilter<bf_width, bf_hash, INTKEY_LEN + 2> bfHist;

	// control plane for histogram
	std::vector<new_int_data_t> flowKeysHist;
	std::map<Key_t, std::map<uint16_t, uint32_t>> evictHistResult;
	// std::map<new_data_t, T> cmControlPlaneHistResult;				// temporarily unuseful
	std::map<Key_t, std::array<T, BUCKET_NUM>> cmHistResult;	// decoding result for Histogram

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

	void insert(Key_t k, uint16_t bid, T v = 1) {
		new_int_data_t swap_key;
		slot<T, bucket_num> swap_slot;
		T swap_val = 0;

		// bool debug = comkeyHist(k, bid);
		bool debug = false;

		int ret = table.insert(k, bid, swap_key, swap_val, swap_slot, v, debug);

		if (debug)
			std::cout << "Return value:" << ret << std::endl;

		switch (ret) {
			case HIT_EVICT:				// swap a bucket
			{
				// bool test = comkeyHist(swap_key.key.str, swap_key.bid);
				bool test = false;
				if (test)
					std::cout << "Evict a histogram bucket\n";
				lightHist.insert(swap_key.str(), swap_val);
				if (!bfHist.getbit(swap_key.str())) {
					bfHist.setbit(swap_key.str());
					flowKeysHist.push_back(swap_key);
				}
				break;
			}
			case MISS_EVICT:			// swap a flow
			{
				// bool test = (swap_slot.key == -1);

				for (int i = 0; i < bucket_num; ++i) {
					if (swap_slot.hist.buckets[i].idx == -1)
						continue;
					if (evictHistResult[swap_slot.key].find(swap_slot.hist.buckets[i].idx) == evictHistResult[swap_slot.key].end())
						evictHistResult[swap_slot.key].emplace(swap_slot.hist.buckets[i].idx, swap_slot.hist.buckets[i].val);
					else
						evictHistResult[swap_slot.key][swap_slot.hist.buckets[i].idx] += swap_slot.hist.buckets[i].val;
				}
				
				break;
			}
			case MISS_INSERT:			//just insert into the light part
			{
				new_int_data_t tmp = new_int_data_t(k, bid);
				lightHist.insert(tmp.str(), v);
				if (!bfHist.getbit(tmp.str())) {
					bfHist.setbit(tmp.str());
					flowKeysHist.push_back(tmp);
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

	void solve_equations(std::map<Key_t, std::array<T, BUCKET_NUM> > & mp) {
		if (!mp.empty())
			return;

		uint32_t w = lightHist.width;
		int M = d_hist * w, N;

		N = flowKeysHist.size();

		Eigen::VectorXd X(N), b(M);
		Eigen::SparseMatrix<double> A(M, N);
		std::vector<Eigen::Triplet<double>> tripletlist;

		for (int i = 0, j; i < d_hist; i++) {
			j = 0;
			for (std::vector<new_int_data_t>::iterator it = flowKeysHist.begin(); it != flowKeysHist.end(); ++it, ++j) {
				int idx = i * w + lightHist.hash(it->str(), i);
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

	T pointQuery(Key_t k, uint16_t bid) {
		uint16_t swap = 0;
		T result = 0;

		// bool debug = compareKeyHist(k, bid);
		bool debug = false;

		// Hash table result
		result = table.query(k, bid, swap);
		if (debug)
			std::cout << "hash table result:" << result << std::endl;

		// CM result
		if (swap || result == 0) {
		#ifdef SOLVED
			solve_equations(cmHistResult);
			
			
			if (cmHistResult.find(k) != cmHistResult.end()) {
				result += cmHistResult[k].at(bid);
				if (debug)
					std::cout << "cm Result:" << cmHistResult[k].at(bid) << std::endl;
			}
		#else
			if (bfHist.getbit(new_int_data_t(k, bid).str()))
				result += lightHist.query(new_int_data_t(k, bid).str());
		#endif
		}

		// control plane result
		if (evictHistResult.find(k) != evictHistResult.end() && evictHistResult[k].find(bid) != evictHistResult[k].end()) {
			result += evictHistResult[k][bid];
			if (debug)
				std::cout << "control plane result:" << evictHistResult[k][bid] << std::endl;
		}

		// std::cout << std::endl;
		return result;
	}

	Hist_t histogramQuery(Key_t key) {
		Hist_t result{};
		hist_card = hist_total = 0;
		for (int i = 0; i < BUCKET_NUM; ++i) {
			// uint32_t tmp = 0;
			// if ((tmp = pointQuery(key, i)) != 0) {
			// 	hist_card++;
			// }
			// hist_total += tmp;
			result.at(i) = pointQuery(key, i);
			hist_total += result.at(i);
			if (result.at(i))
				hist_card++;
		}
		return result;
	}

	size_t get_memory_usage() {
		std::cout << "\nHash table:" << table.get_memory_usage() << std::endl;
		std::cout << "CM Sketch:" << lightHist.get_memory_usage() << std::endl;
		std::cout << "Bloom Filter:" << bfHist.get_memory_usage() << std::endl;
		return table.get_memory_usage() + lightHist.get_memory_usage() + bfHist.get_memory_usage();
	}
};

#endif