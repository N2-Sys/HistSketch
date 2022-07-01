#ifndef _UTILS_H
#define _UTILS_H

#include "common.h"

// uint8_t testKey[CHARKEY_LEN] = {73, 167, 155, 193, 251, 215, 115, 82, 1, 187, 172, 27, 6};
// uint8_t testKeyHist[CHARKEY_LEN+1] = {73, 167, 155, 193, 251, 215, 115, 82, 1, 187, 172, 27, 6, 15};

Key_t testKey;
uint8_t testBid;

bool compareKeyHist(Key_t k, uint8_t bid) {
	return (testKey == k && testBid == bid);
}

bool compareKey(Key_t k) {
	return (k == testKey);
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

void readTraces(const char *path, std::vector<int_data_t>& traces) {
	FILE *inputData = fopen(path, "rb");
	traces.clear();
	char *strData = new char[DATA_T_SIZE];

	printf("Reading in data\n");

	while (fread(strData, DATA_T_SIZE, 1, inputData) == 1) {
		traces.push_back(int_data_t(strData));
	}
	fclose(inputData);
	
	printf("Successfully read in %ld packets\n", traces.size());
}


#endif
