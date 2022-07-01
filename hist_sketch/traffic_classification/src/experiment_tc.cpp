#include "sketch.h"

using namespace std;

const char *path1 = "../../../data/chat1a.dat";
const char *path2 = "../../../data/chat1b.dat";

const char *path3 = "../../../data/file1.dat";
const char *path4 = "../../../data/file2.dat";
const char *path5 = "../../../data/file3.dat";
const char *path6 = "../../../data/file4.dat";
const char *path7 = "../../../data/file5.dat";
const char *path8 = "../../../data/file6.dat";
const char *path9 = "../../../data/file7.dat";
const char *path10 = "../../../data/file8.dat";

const char *path11 = "../../../data/chat2.dat";
const char *path12 = "../../../data/chat3.dat";
const char *path13 = "../../../data/chat4.dat";
const char *path14 = "../../../data/chat5.dat";

vector<data_t> chat_traces;
vector<data_t> file_traces;

map<five_tuple, map<uint8_t, uint32_t>> ground_truth_all_chat;
map<five_tuple, map<uint8_t, uint32_t>> ground_truth_all_file;
map<uint32_t, uint32_t> chat_distribution, file_distribution, chat_distribution_estimate, file_distribution_estimate;

sketch<uint32_t, H_BUCKET_NUM, SLOT_NUM, CM_DEPTH_HIST, CM_WIDTH_HIST, BLOOM_SIZE, BLOOM_HASH_NUM>  chat_hist;
sketch<uint32_t, H_BUCKET_NUM, SLOT_NUM, CM_DEPTH_HIST, CM_WIDTH_HIST, BLOOM_SIZE, BLOOM_HASH_NUM> file_hist;

int main() {
	readTraces(path1, chat_traces);
	readTraces(path2, chat_traces);
	readTraces(path11, chat_traces);
	readTraces(path12, chat_traces);
	readTraces(path13, chat_traces);
	readTraces(path14, chat_traces);

	readTraces(path3, file_traces);
	readTraces(path4, file_traces);
	readTraces(path5, file_traces);
	readTraces(path6, file_traces);
	readTraces(path7, file_traces);
	readTraces(path8, file_traces);
	readTraces(path9, file_traces);
	readTraces(path10, file_traces);

	// insert chat data
	uint16_t max_length = 0, min_length = 0xffff;
	uint32_t cnt_chat = 0;
	for (auto it = chat_traces.begin(); it != chat_traces.end(); it++) {
		uint8_t bid = it->length / BUCKET_WIDTH;
		if (bid > 15)
			bid = 15;
		chat_hist.insert((Key_t)it->key.str, bid);
		//decode the ground truth
		ground_truth_all_chat[it->key][bid]++;
		chat_distribution[bid] += 1;
		max_length = max(max_length, it->length);
		min_length = min(min_length, it->length);
		cnt_chat++;
	}
	printf("Flow number:%d\nPacket number:%d\nmax length in the data:%d\nmin length in the data:%d\n",
		(int)ground_truth_all_chat.size(), cnt_chat, max_length, min_length);
	for (auto it = chat_distribution.begin(); it != chat_distribution.end(); it++) {
		cout << it->second << endl;
	}

	// insert file data
	max_length = 0, min_length = 0xffff;
	uint32_t cnt_file = 0;
	for (auto it = file_traces.begin(); it != file_traces.end(); it++) {
		uint8_t bid = it->length / BUCKET_WIDTH;
		if (bid > 15)
			bid = 15;
		file_hist.insert((Key_t)it->key.str, bid);
		//decode the ground truth
		ground_truth_all_file[it->key][bid]++;
		file_distribution[bid] += 1;
		max_length = max(max_length, it->length);
		min_length = min(min_length, it->length);
		cnt_file++;
		if (cnt_file == cnt_chat) {
			break;
		}
	}
	printf("Flow number:%d\nPacket number:%d\nmax length in the data:%d\nmin length in the data:%d\n",
		(int)ground_truth_all_file.size(), cnt_file, max_length, min_length);
	for (auto it = file_distribution.begin(); it != file_distribution.end(); it++) {
		cout << it->second << endl;
	}

	// histogram query of all chat flows
	double chat_entropy_are = 0;
	for (auto it = ground_truth_all_chat.begin(); it != ground_truth_all_chat.end(); ++it) {
		Hist_t result = chat_hist.histogramQuery((Key_t)it->first.str);
		for (int i = 0; i < BUCKET_NUM; ++i) {
			chat_distribution_estimate[i] += result.at(i);
		}
		double entropy = get_per_flow_entropy(it->second);
		uint32_t total = 0;
		double entropy_estimate = 0;
		for (int i = 0; i < BUCKET_NUM; ++i) {
			if (result.at(i) == 0) continue;
			total += result.at(i);
			entropy_estimate += result.at(i) * log2(result.at(i));
		}
		entropy_estimate = -entropy_estimate / total + log2(total);
		if (entropy != 0)
			chat_entropy_are += fabs(entropy - entropy_estimate) / entropy;
	}
	cout << "Histogram quer for chat flows:" << endl;
	for (auto it = chat_distribution_estimate.begin(); it != chat_distribution_estimate.end(); it++) {
		cout << it->second << endl;
	}

	// histogram query of all file flows
	double file_entropy_are = 0;
	for (auto it = ground_truth_all_file.begin(); it != ground_truth_all_file.end(); ++it) {
		Hist_t result = file_hist.histogramQuery((Key_t)it->first.str);
		for (int i = 0; i < BUCKET_NUM; ++i) {
			file_distribution_estimate[i] += result.at(i);
		}
		double entropy = get_per_flow_entropy(it->second);
		uint32_t total = 0;
		double entropy_estimate = 0;
		for (int i = 0; i < BUCKET_NUM; ++i) {
			if (result.at(i) == 0) continue;
			total += result.at(i);
			entropy_estimate += result.at(i) * log2(result.at(i));
		}
		entropy_estimate = -entropy_estimate / total + log2(total);
		if (entropy != 0)
			file_entropy_are += fabs(entropy - entropy_estimate) / entropy;
	}
	cout << "Histogram quer for file flows:" << endl;
	for (auto it = file_distribution_estimate.begin(); it != file_distribution_estimate.end(); it++) {
		cout << it->second << endl;
	}

	cout << "Chat entropy ARE:" << chat_entropy_are / ground_truth_all_chat.size() << endl;
	cout << "File entropy ARE:" << file_entropy_are / ground_truth_all_file.size() << endl;
	cout << "Memory usage(chat):" << chat_hist.get_memory_usage() / (double)1024 / 1024 << "MB" << endl;
	cout << "Memory usage(file):" << file_hist.get_memory_usage() / (double)1024 / 1024 << "MB" << endl;

	return 0;
}




