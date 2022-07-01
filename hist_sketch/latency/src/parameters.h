#ifndef _PARAM_H_
#define _PARAM_H_

#define CHARKEY_LEN 13
#define DATA_T_SIZE 15

// hash table
#define SLOT_NUM (1 << 11) + (1 << 12) + (1 << 13)
#define EVICT_THRESHOLD 1
#define CONTROL_PLANE_PRO 0

// bloom filter
#define BLOOM_SIZE (1 << 21)
#define BLOOM_HASH_NUM 11

// histogram
#define BUCKET_WIDTH 50
#define BUCKET_NUM 20
#define H_BUCKET_NUM 8

// cm for histogram
#define CM_DEPTH_HIST 4
#define CM_WIDTH_HIST (1 << 15)

// metrics
#define POINT_ARETHRESHOLD 0.01
#define HISTOGRAM_ARETHRESHOLD 0.01
#define HC_SIZE 50

/*************************************/
/****** Do not edit this part! *******/
#define HIT 0
#define HIT_EVICT 1
#define MISS_EVICT 2
#define MISS_INSERT 3

#define HIST_HIT 0
#define HIST_EVICT 1

#define SOLVED

#endif