#ifndef _LIGHT_PART_H_
#define _LIGHT_PART_H_

#include "parameters.h"
#include "BloomFilter.h"
#include "Eigen/Dense"
#include "Eigen/IterativeLinearSolvers"
#include "Eigen/SparseCore"

uint32_t bandwidth = 0;
double total_insert_time = 0;

template<typename T, uint32_t w, uint32_t d>
class PRSketch {
	private:
		uint32_t width = calNextPrime(w), depth = d;
		uint64_t h[d], s[d], n[d];
		BloomFilter<PR_BF_WIDTH, PR_BF_HASH> bf;
		std::vector<new_data_t> flowkeyStorage;
		std::map<new_data_t, T> mp;

		uint32_t hash(unsigned char *key, int idx) {
			return (uint32_t)(AwareHash(key, CHARKEY_LEN + 1, h[idx], s[idx], n[idx]) % width);
		}

	public:
		T** matrix;
	
		PRSketch() {
			matrix = new T*[d];
			matrix[0] = new T[width * d];
			memset(matrix[0], 0, width * d * sizeof(T));
			for (int i = 1; i < d; ++i) {
				matrix[i] = matrix[i - 1] + width;
			}

			

			int index = 0;
			for (int i = 0; i < d; ++i) {
				h[i] = GenHashSeed(index++);
				s[i] = GenHashSeed(index++);
				n[i] = GenHashSeed(index++);
			}

			mp.clear();
		}

		void insert(Key_t key, uint32_t val = 1) {	//note: flow ID + bucket ID
			uint32_t pos;
			// auto t_a = std::chrono::high_resolution_clock::now();
			if (!bf.getbit(key)) {
				bf.setbit(key);
				flowkeyStorage.push_back(new_data_t((char *)key));
				bandwidth += CHARKEY_LEN + 1;
			}
			for (int i = 0; i < depth; ++i) {
				pos = (uint32_t)(AwareHash((unsigned char *)key, CHARKEY_LEN + 1, h[i], s[i], n[i]) % width);
				matrix[i][pos] += val;
			}
			// auto t_b = std::chrono::high_resolution_clock::now();
			// total_insert_time += std::chrono::duration_cast<std::chrono::microseconds>(t_b - t_a).count();
		}

		void fastInsert(Key_t key, T f = 1, bool flag = false) {			//note: flow ID + bucket ID
			uint32_t pos;
			T result = query(key);
			for (int i = 0; i < depth; ++i) {
				pos = (uint32_t)(AwareHash((unsigned char *)key, CHARKEY_LEN + 1, h[i], s[i], n[i]) % width);
				matrix[i][pos] += f;
			}
			if (result > FAST_THRESHOLD)
				return;
			if (!bf.getbit(key)) {
				bf.setbit(key);
				flowkeyStorage.push_back(new_data_t((char *)key));
			}
		}

		T onlineQuery(Key_t key) {
			T result = std::numeric_limits<T>::max();
			uint32_t pos;
			for (int i = 0; i < depth; ++i) {
				pos = (uint32_t)(AwareHash((unsigned char *)key, CHARKEY_LEN + 1, h[i], s[i], n[i]) % width);
				result = std::min(matrix[i][pos], result);
			}
			return result;
		}

		void solve_equations(std::map<new_data_t, T> & mp) {
			if (!mp.empty())
				return;

			std::cout << "Constructing the equations..." << std::endl;

			int M = depth * width, N = flowkeyStorage.size();

			Eigen::VectorXd X(N), b(M);
			Eigen::SparseMatrix<double> A(M, N);
			std::vector<Eigen::Triplet<double>> tripletlist;

			auto t_a = std::chrono::high_resolution_clock::now();

			for (int i = 0, j; i < depth; i++) {
				j = 0;
				for (auto it = flowkeyStorage.begin(); it != flowkeyStorage.end(); ++it, ++j) {
					int idx = i * width + this->hash((unsigned char *)it->c_str, i);
					//按Triplet方式填充，速度快
					tripletlist.push_back(Eigen::Triplet<double>(idx, j, 1));
				}
				for (j = 0; j < width; ++j) {
					b(i * width + j) = this->matrix[i][j];
				}
			}
			
			A.setFromTriplets(tripletlist.begin(), tripletlist.end());

			 // 压缩优化矩阵

			// std::cout << "Solving the equations..." << std::endl;

			A.makeCompressed();
			Eigen::LeastSquaresConjugateGradient<Eigen::SparseMatrix<double>> Solver_sparse;
			Solver_sparse.compute(A);
			X = Solver_sparse.solve(b);

			auto t_b = std::chrono::high_resolution_clock::now();
			std::cout << "Solved time: " << std::chrono::duration_cast<std::chrono::microseconds>(t_b - t_a).count() << "us" << std::endl;


			std::cout << "Dumping the result" << std::endl;

			for (int i = 0; i < N; ++i) {
				int ans = (int)(X(i) + 0.5);
				ans = (ans <= 0)?1:ans;
				// std::cout << ans << std::endl;
				mp.emplace(flowkeyStorage[i], ans);
			}
		}

		T query(Key_t key) {
			T result = 0;
			new_data_t tmp_key((char *)key);

			solve_equations(mp);
			if (mp.find(tmp_key) != mp.end())
				result = mp[tmp_key];
			// else
			// 	result = onlineQuery(key);
			
			return result;
		}

		uint32_t get_memory_usage() {
			return width * d * sizeof(T) + bf.get_memory_usage();
		}
};

#endif