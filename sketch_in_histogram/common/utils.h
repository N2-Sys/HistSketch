#ifndef _UTILS_H
#define _UTILS_H

#include "common.h"

uint8_t testKey[CHARKEY_LEN] = {133, 250, 69, 221, 215, 19, 187, 104, 238, 111, 234, 11, 6};
uint8_t testKeyHist[CHARKEY_LEN+1] = {124, 101, 52, 192, 135, 79, 171, 146, 1, 187, 86, 98, 6, 7};

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

#endif
