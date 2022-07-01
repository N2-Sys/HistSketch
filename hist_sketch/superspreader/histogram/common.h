#ifndef _COMMON_H
#define _COMMON_H

#include <cassert>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <ctime>
#include <cmath>
#include <limits>
#include <iostream>
#include <algorithm>
#include <vector>
#include <map>
#include <set>
#include <unordered_map>
#include <random>
#include <array>

#include "../../Eigen/Dense"
#include "../../Eigen/IterativeLinearSolvers"
#include "../../Eigen/SparseCore"
#include "AwareHash.h"
#include "parameters.h"

typedef const char * CharKey_t;
typedef uint32_t Key_t;
typedef std::array<uint32_t, BUCKET_NUM> Hist_t;

typedef struct five_tuple {
	char str[CHARKEY_LEN];
	bool operator < (const five_tuple& a) const {
		bool flag = false;
		for (int i = 0; i < CHARKEY_LEN; ++i) {
			if (this->str[i] == a.str[i])
				continue;
			else if (this->str[i] < a.str[i]) {
				flag = 1;
				break;
			}
			else
				return false;
		}
		return flag;
	}
	bool operator == (const five_tuple& a) const {
		for (int i = 0; i < CHARKEY_LEN; ++i) {
			if (this->str[i] == a.str[i])
				continue;
			else
				return false;
		}
		return true;
	}
	bool isEmpty() {
		for (int i = 0; i < CHARKEY_LEN; ++i) {
			if (str[i] == 0)
				continue;
			else
				return false;
		}
		return true;
	}
	five_tuple() {
		memset(str, 0, sizeof(str));
	}
	five_tuple(char * s) {
		memcpy(str, s, CHARKEY_LEN);
	}
	five_tuple(CharKey_t s) {
		memcpy(str, s, CHARKEY_LEN);
	}
} five_tuple;

typedef struct data_t{
	five_tuple key;		//the five tuple
	uint16_t length;	//IP packet length
	data_t(char *s) {
		key = five_tuple(s);
		length = *(uint16_t *)(s + CHARKEY_LEN);
	}
	data_t(CharKey_t s) {
		key = five_tuple(s);
		length = *(uint16_t *)(s + CHARKEY_LEN);
	}
	data_t() {
		length = 0;
	}
} data_t;

typedef struct new_data_t{
	five_tuple key;		//the five tuple
	uint8_t bid;		//bucket id

	new_data_t(char *s) {
		key = five_tuple(s);
		bid = *(uint8_t *)(s + CHARKEY_LEN);
	}
	new_data_t(CharKey_t s) {
		key = five_tuple(s);
		bid = *(uint8_t *)(s + CHARKEY_LEN);
	}

	new_data_t(char *s, uint8_t bucket_id) {
		key = five_tuple(s);
		bid = bucket_id;
	}
	new_data_t(CharKey_t s, uint8_t bucket_id) {
		key = five_tuple(s);
		bid = bucket_id;
	}

	new_data_t() {
		bid = 0;
	}

	CharKey_t str() {
		char *new_key;
		new_key = new char[CHARKEY_LEN + 1];
		memcpy(new_key, key.str, CHARKEY_LEN);
		new_key[CHARKEY_LEN] = bid;
		return (CharKey_t)new_key;
	}

	bool is_zero() {
		if (bid)
			return false;
		for (int i = 0; i < CHARKEY_LEN; ++i)
			if (key.str[i])
				return false;
		return true;
	}

	bool operator < (const new_data_t& a) const {
		if (key < a.key || (key == a.key && bid < a.bid))
			return true;
		else
			return false;
	}

	bool operator == (const new_data_t& a) const {
		if (key == a.key && bid == a.bid)
			return true;
		else
			return false;
	}
} new_data_t;

typedef struct int_data_t{
	uint32_t key;		// source ip
	uint32_t dst;	// destination ip
	int_data_t(char *s) {
		key = *(uint32_t*)s;
		dst = *(uint32_t *)(s + INTKEY_LEN);
	}
	int_data_t(Key_t s, Key_t d) {
		key = s;
		dst = d;
	}
	int_data_t() {
		dst = key = 0;
	}
} int_data_t;

typedef struct new_int_data_t{
	Key_t key;		//the five tuple
	uint16_t bid;		//bucket id

	new_int_data_t(char *s) {
		key = *(uint32_t *)s;
		bid = *(uint16_t *)(s + INTKEY_LEN);
	}
	new_int_data_t(Key_t s, uint16_t id) {
		key = s;
		bid = id;
	}

	new_int_data_t() {
		key = bid = 0;
	}

	CharKey_t str() {
		char *new_key = new char[INTKEY_LEN + 1];
		memcpy(new_key, (void *)&key, INTKEY_LEN);
		new_key[INTKEY_LEN] = bid;
		return (CharKey_t)new_key;
	}

	bool is_zero() {
		if (bid || key)
			return false;
		return true;
	}

	bool operator < (const new_int_data_t& a) const {
		if (key < a.key || (key == a.key && bid < a.bid))
			return true;
		else
			return false;
	}

	bool operator == (const new_int_data_t& a) const {
		if (key == a.key && bid == a.bid)
			return true;
		else
			return false;
	}
} new_int_data_t;

#endif


