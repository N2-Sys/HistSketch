#include "sketch.h"
#include "LossRadar.h"

using namespace std;

const uint32_t flow_number = 100000;
const char *path = "../../../data/test-4s.dat";

vector<data_t> traces;
map<five_tuple, map<uint8_t, uint32_t>> ground_truth_all;
map<five_tuple, map<uint8_t, uint32_t>> ground_truth_large;
map<five_tuple, map<uint8_t, uint32_t>> ground_truth_small;
map<five_tuple, map<uint8_t, uint32_t>> ground_truth_random;
map<five_tuple, map<uint8_t, uint32_t>> ground_truth_loss_large;
map<five_tuple, map<uint8_t, uint32_t>> ground_truth_loss_small;
map<five_tuple, map<uint8_t, uint32_t>> ground_truth_loss_random;


sketch<uint32_t, H_BUCKET_NUM, SLOT_NUM, CM_DEPTH_HIST, CM_WIDTH_HIST, BLOOM_SIZE, BLOOM_HASH_NUM> hist;
sketch<uint32_t, H_BUCKET_NUM, SLOT_NUM, CM_DEPTH_HIST, CM_WIDTH_HIST, BLOOM_SIZE, BLOOM_HASH_NUM> hist_loss;
LossRadar<uint32_t, LR_WIDTH, LR_HASH> lr;
LossRadar<uint32_t, LR_WIDTH, LR_HASH> lr_loss;


int main() {
	readTraces(path, traces);

	// insert data
	uint16_t max_length = 0, min_length = 0xffff;
	uint32_t packet_cnt = 0;
	double total_insert_time = 0;
	int N = traces.size();
	for (auto it = traces.begin(); it != traces.end(); it++) {
		uint8_t bid = it->length / BUCKET_WIDTH;
		new_data_t tmp_key((Key_t)it->key.str, bid);
		hist.insert((Key_t)it->key.str, bid, tmp_key);

		// decode the ground truth
		ground_truth_all[it->key][bid]++;
		max_length = max(max_length, it->length);
		min_length = min(min_length, it->length);
		packet_cnt++;

		// insert into LR
		CustomizedKey tmp_ckey(tmp_key, ground_truth_all[it->key][bid]);
		lr.insert(tmp_ckey);

		// if (ground_truth_size.size() == flow_number) {
		// 	break;
		// }
	}

	cout << "Total: " << ground_truth_all.size() << " flows and " << packet_cnt << " packets" << endl;

	// // Big loss
	// int big_loss_cnt = 0;
	// ground_truth_large = ground_truth_all;
	// for (auto it = ground_truth_all.begin(); it != ground_truth_all.end(); it++) {
	// 	for (auto it2 = it->second.rbegin(); it2 != it->second.rend(); it2++) {
	// 		if (it2->first >= BIG_BORDER) {
	// 			big_loss_cnt += it2->second;
	// 			ground_truth_loss_large[it->first][it2->first] += it2->second;
	// 			ground_truth_large[it->first].erase(it2->first);
	// 		}
	// 		else
	// 			break;
	// 	}
	// 	if (big_loss_cnt >= packet_cnt * 0.01)
	// 		break;
	// }
	// cout << big_loss_cnt << " large packets & lost " << ground_truth_loss_large.size() << " flows" << endl;

	// for (auto it = traces.begin(); it != traces.end(); ++it) {
	// 	uint8_t bid = it->length / BUCKET_WIDTH;
	// 	new_data_t tmp_key((Key_t)it->key.str, bid);
	// 	if (ground_truth_large.find(it->key) != ground_truth_large.end() &&
	// 		ground_truth_large[it->key].find(bid) != ground_truth_large[it->key].end() &&
	// 		ground_truth_large[it->key][bid] != 0) {
	// 			hist_loss.insert((Key_t)it->key.str, bid, tmp_key);

	// 			CustomizedKey tmp_ckey(tmp_key, ground_truth_large[it->key][bid]);
	// 			lr_loss.insert(tmp_ckey);

	// 			ground_truth_large[it->key][bid]--;
	// 		}
			
	// }
	// cout << "Total: " << ground_truth_large.size() << " flows and " << packet_cnt - big_loss_cnt << " packets" << endl;

	// map<five_tuple, array<uint32_t, BUCKET_NUM>> result;
	// hist.get_packet_loss(result, hist_loss);

	// lr ^= lr_loss;
	// lr.dump();

	// map<five_tuple, array<uint32_t, BUCKET_NUM>> lr_result;
	// for (auto it = lr.lost_packet.begin(); it != lr.lost_packet.end(); ++it) {
	// 	lr_result[it->flowID.key][it->flowID.bid] += 1;
	// }

	// double are_hs = 0;
	// double are_lr = 0;
	// uint32_t bucket_cnt = 0;
	// for (auto it = ground_truth_loss_large.begin(); it != ground_truth_loss_large.end(); ++it) {
	// 	for (auto it2 = it->second.begin(); it2 != it->second.end(); it2++) {
	// 		if (it2->second != result[it->first].at(it2->first)) {
	// 			are_hs += fabs((double)it2->second - (double)result[it->first].at(it2->first)) / it2->second;
	// 		}

	// 		if (it2->second != lr_result[it->first].at(it2->first)) {
	// 			are_lr += fabs((double)it2->second - (double)lr_result[it->first].at(it2->first)) / it2->second;
	// 		}
	// 		bucket_cnt++;
	// 	}
	// }

	// double F1_HS = packet_loss_large(result, ground_truth_loss_large);
	// double F1_LR = packet_loss_large(lr_result, ground_truth_loss_large);


	// // Small losss
	// int small_loss_cnt = 0;
	// ground_truth_small = ground_truth_all;
	// for (auto it = ground_truth_all.begin(); it != ground_truth_all.end(); it++) {
	// 	for (auto it2 = it->second.begin(); it2 != it->second.end(); it2++) {
	// 		if (it2->first <= SMALL_BORDER) {
	// 			small_loss_cnt += it2->second;
	// 			ground_truth_loss_small[it->first][it2->first] += it2->second;
	// 			ground_truth_small[it->first].erase(it2->first);
	// 		}
	// 		else
	// 			break;
	// 	}
	// 	if (small_loss_cnt >= packet_cnt * 0.01)
	// 		break;
	// }
	// cout << small_loss_cnt << " small packets lost" << endl;

	// for (auto it = traces.begin(); it != traces.end(); ++it) {
	// 	uint8_t bid = it->length / BUCKET_WIDTH;
	// 	new_data_t tmp_key((Key_t)it->key.str, bid);
	// 	if (ground_truth_small.find(it->key) != ground_truth_small.end() &&
	// 		ground_truth_small[it->key].find(bid) != ground_truth_small[it->key].end() &&
	// 		ground_truth_small[it->key][bid] != 0) {
	// 			hist_loss.insert((Key_t)it->key.str, bid, tmp_key);

	// 			CustomizedKey tmp_ckey(tmp_key, ground_truth_small[it->key][bid]);
	// 			lr_loss.insert(tmp_ckey);

	// 			ground_truth_small[it->key][bid]--;
	// 		}
			
	// }
	// cout << "Total: " << ground_truth_small.size() << " flows and " << packet_cnt - small_loss_cnt << " packets" << endl;

	// map<five_tuple, array<uint32_t, BUCKET_NUM>> result;
	// hist.get_packet_loss(result, hist_loss);

	// lr ^= lr_loss;
	// lr.dump();

	// map<five_tuple, array<uint32_t, BUCKET_NUM>> lr_result;
	// for (auto it = lr.lost_packet.begin(); it != lr.lost_packet.end(); ++it) {
	// 	lr_result[it->flowID.key][it->flowID.bid] += 1;
	// }

	// double are_hs = 0;
	// double are_lr = 0;
	// uint32_t bucket_cnt = 0;
	// for (auto it = ground_truth_loss_small.begin(); it != ground_truth_loss_small.end(); ++it) {
	// 	for (auto it2 = it->second.begin(); it2 != it->second.end(); it2++) {
	// 		if (it2->second != result[it->first].at(it2->first)) {
	// 			are_hs += fabs((double)it2->second - (double)result[it->first].at(it2->first)) / it2->second;
	// 		}

	// 		if (it2->second != lr_result[it->first].at(it2->first)) {
	// 			are_lr += fabs((double)it2->second - (double)lr_result[it->first].at(it2->first)) / it2->second;
	// 		}
	// 		bucket_cnt++;
	// 	}
	// }

	// double F1_HS = packet_loss_small(result, ground_truth_loss_small);
	// double F1_LR = packet_loss_small(lr_result, ground_truth_loss_small);


	// Random loss
	int random_loss_cnt = 0;
	ground_truth_random = ground_truth_all;
	for (auto it = ground_truth_all.begin(); it != ground_truth_all.end(); it++) {
		if (it->second.size() >= RANDOM_BORDER) {
			for (auto it2 = it->second.begin(); it2 != it->second.end(); it2++) {
				random_loss_cnt += it2->second;
				ground_truth_loss_random[it->first][it2->first] += it2->second;
				// ground_truth_random[it->first].erase(it2->first);
			}
			ground_truth_random.erase(it->first);
			if (random_loss_cnt >= packet_cnt * 0.01)
				break;
		}
	}
	cout << random_loss_cnt << " random packets lost" << endl;

	for (auto it = traces.begin(); it != traces.end(); ++it) {
		uint8_t bid = it->length / BUCKET_WIDTH;
		new_data_t tmp_key((Key_t)it->key.str, bid);
		// if (ground_truth_random.find(it->key) != ground_truth_random.end() &&
		// 	ground_truth_random[it->key].find(bid) != ground_truth_random[it->key].end() &&
		// 	ground_truth_random[it->key][bid] != 0) {
		if (ground_truth_random.find(it->key) != ground_truth_random.end()) {
			hist_loss.insert((Key_t)it->key.str, bid, tmp_key);

			CustomizedKey tmp_ckey(tmp_key, ground_truth_random[it->key][bid]);
			lr_loss.insert(tmp_ckey);

			ground_truth_random[it->key][bid]--;
		}
	}
	cout << "Total: " << ground_truth_random.size() << " flows and " << packet_cnt - random_loss_cnt << " packets" << endl;

	map<five_tuple, array<uint32_t, BUCKET_NUM>> result;
	hist.get_packet_loss(result, hist_loss);

	lr ^= lr_loss;
	lr.dump();

	map<five_tuple, array<uint32_t, BUCKET_NUM>> lr_result;
	for (auto it = lr.lost_packet.begin(); it != lr.lost_packet.end(); ++it) {
		lr_result[it->flowID.key][it->flowID.bid] += 1;
	}

	double are_hs = 0;
	double are_lr = 0;
	uint32_t bucket_cnt = 0;
	for (auto it = ground_truth_loss_random.begin(); it != ground_truth_loss_random.end(); ++it) {
		for (auto it2 = it->second.begin(); it2 != it->second.end(); it2++) {
			if (it2->second != result[it->first].at(it2->first)) {
				are_hs += fabs((double)it2->second - (double)result[it->first].at(it2->first)) / it2->second;
			}

			if (it2->second != lr_result[it->first].at(it2->first)) {
				are_lr += fabs((double)it2->second - (double)lr_result[it->first].at(it2->first)) / it2->second;
			}
			bucket_cnt++;
		}
	}
	double F1_HS = packet_loss_random(result, ground_truth_loss_random);
	double F1_LR = packet_loss_random(lr_result, ground_truth_loss_random);


	// output the result
	cout << bucket_cnt << " buckets" << endl;

	cout << "ARE for HS:" << are_hs / bucket_cnt << endl;
	cout << "ARE for LR:" << are_lr / bucket_cnt << endl;

	cout << "F1 for HS:" << F1_HS << endl;
	cout << "F1 for LR:" << F1_LR << endl;

	cout << "Memory usage for HS: " << (double)hist.get_memory_usage() / (1 << 20) << " MB" << endl;
	cout << "Memory usage for LR: " << (double)lr.get_memory_usage() / (1 << 20) << " MB" << endl;	
	return 0;
}




