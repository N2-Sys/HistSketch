#ifndef _UTILS_H
#define _UTILS_H

#include "common.h"

uint8_t testKey[CHARKEY_LEN] = {73, 167, 155, 193, 251, 215, 115, 82, 1, 187, 172};
uint8_t testKeyHist[CHARKEY_LEN+1] = {73, 167, 155, 193, 251, 215, 115, 82, 1, 187, 172, 27};

bool compareKeyHist(Key_t k, uint8_t bid) {
	bool f = true;
	for (int i = 0; i < CHARKEY_LEN; ++i) {
		if ((uint8_t)k[i] != testKeyHist[i])
			f = false;
	}
	if (bid != testKeyHist[CHARKEY_LEN])
		f = false;
	return f;
}

bool compareKey(Key_t k) {
	bool f = true;
	for (int i = 0; i < CHARKEY_LEN; ++i) {
		if ((uint8_t)k[i] != testKey[i]) {
			f = false;
			break;
		}
	}
	return f;
}

bool isPrime(uint32_t num) {
	uint32_t border = (uint32_t)ceil(sqrt((double)num));
	for (uint32_t i = 2; i <= border; ++i) {
		if ((num % i) == 0)
			return false;
	}
	return true;
}

uint32_t calNextPrime(uint32_t num) {
	while (!isPrime(num)) {
		num++;
	}
	return num;
}

void readTraces(const char *path, std::vector<data_t>& traces) {
	FILE *inputData = fopen(path, "rb");
	traces.clear();
	char *strData = new char[DATA_T_SIZE];

	printf("Reading in data\n");

	while (fread(strData, DATA_T_SIZE, 1, inputData) == 1) {
		traces.push_back(data_t(strData));
	}
	fclose(inputData);
	
	printf("Successfully read in %ld packets\n", traces.size());
}

void get_distribution(std::map<five_tuple, std::map<uint8_t, uint32_t>> &ground_truth, std::map<uint32_t, uint32_t> &distribution) {
	for (auto iter = ground_truth.begin(); iter != ground_truth.end(); iter++) {
		for (auto iter2 = iter->second.begin(); iter2 != iter->second.end(); iter2++) {
			if (distribution.find(iter2->second) == distribution.end())
				distribution.emplace(iter2->second, 1);
			else
				distribution[iter2->second] += 1;
		}
	}
}

double get_entropy(std::map<five_tuple, std::map<uint8_t, uint32_t>> & ground_truth) {
	std::map<uint32_t, uint32_t> distribution;
	get_distribution(ground_truth, distribution);

	uint32_t total = 0;
	double entropy = 0;

	for (auto iter = distribution.begin(); iter != distribution.end(); iter++) {
		total += iter->first * iter->second;
		entropy += iter->first * iter->second * log2(iter->first);
	}
	
	return -entropy / total + log2(total);

}

uint32_t get_cardinality(std::map<five_tuple, std::map<uint8_t, uint32_t>> & ground_truth) {
	uint32_t sum = 0;
	for (auto iter = ground_truth.begin(); iter != ground_truth.end(); iter++) {
		sum += iter->second.size();
	}
	return sum;
}

uint8_t transform_bid(uint16_t x) {
	if (x == 0)
		return 0;
	else if (x == 56)
		return 1;
	else if (x == 97)
		return 2;
	else if (x == 99)
		return 3;
	else if (x == 102)
		return 4;
	else if (x == 104)
		return 5;
	else if (x == 105)
		return 6;
	else if (x == 107)
		return 7;
	else if (x == 109)
		return 8;
	else if (x == 110)
		return 9;
	else if (x == 112)
		return 10;
	else if (x == 114)
		return 11;
	else if (x == 115)
		return 12;
	else if (x == 117)
		return 13;
	else if (x == 119)
		return 14;
	else
		return 15;
}

#endif
