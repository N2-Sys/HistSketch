#include <iostream>
#include <algorithm>
#include <cmath>
#include <map>

using namespace std;

#define BUCKET_NUM 20
#define DATA_T_SIZE 13

map<uint32_t, uint32_t> freqMap;
uint16_t rk = 1;

void zipf(double c, double a, int size, int num) {
	double sum = 0;
	int partialSum = 0;
	for (int i = 1; i <= size; ++i) {
		sum += c / pow(i, a);
	}
	for (int i = 1; i <= size; ++i) {
		freqMap[i] = ceil((c / pow(i, a) / sum) * num);
		// cout << (c / pow(i, a) / sum) << endl;
		// calculate the border
		partialSum += freqMap[i];
		if (partialSum >= num / 20) {
			cout << "border:" << i << endl;
			partialSum = 0;
		}
		// cout << freqMap[i] << endl;
	}

}

uint16_t getZipf() {
	if (freqMap[rk] == 0) {
		rk++;
	}
	freqMap[rk]--;
	return rk + 89;
}

void readTraces(const char *inPath, const char *outPath) {
	FILE *inputData = fopen(inPath, "rb");
	FILE *outputData = fopen(outPath, "wb");
	char *strData = new char[DATA_T_SIZE + 2];
	int cnt = 0;

	printf("Reading in data\n");

	while (fread(strData, DATA_T_SIZE, 1, inputData) == 1) {
		cnt++;
	}

	printf("Successfully read in %d packets\n", cnt);

	fseek(inputData, 0, 0);
	zipf(1, 1, 1000, cnt);
	while (fread(strData, DATA_T_SIZE, 1, inputData) == 1) {
		uint16_t latency = getZipf();
		// cout << latency << endl;
		*((uint16_t *)(strData + DATA_T_SIZE)) = latency;
		fwrite(strData, DATA_T_SIZE + 2, 1, outputData);
	}

	fclose(inputData);
	fclose(outputData);
}

int main() {
	const char *inPath = "../../../data/test-4s.dat";
	const char *outPath = "../../../data/test-4s-lat.dat";
	readTraces(inPath, outPath);
	return 0;
}