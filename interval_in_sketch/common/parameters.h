#ifndef _PARAMETERS_H
#define _PARAMETERS_H

#define CHARKEY_LEN 13
#define DATA_T_SIZE 15
#define BUCKET_WIDTH 94	// change if test latency
#define BUCKET_NUM 16
#define POINT_ARETHRESHOLD 0.01
#define HISTOGRAM_ARETHRESHOLD 0.01
#define HC_SIZE 50

// CM Sketch
#define CM_DEPTH 4
#define CM_WIDTH ((1 << 16)*2)

// CU Sketch
#define CU_DEPTH 4
#define CU_WIDTH (1 << 19)

// Count Sketch
#define CS_DEPTH 4
#define CS_WIDTH ((1 << 16)*2)

// FlowRadar
#define BF_WIDTH ((1 << 20)*2)
#define BF_HASHNUM 11
#define IBLT_WIDTH (48289*2)
#define IBLT_HASHNUM 3

// SuMaxSketch
#define SM_DEPTH 4
#define SM_WIDTH ((1 << 16)*2)

// Elastic Sketch
#define SLOT_NUM ((1 << 12)*15)
#define LIGHT_DEPTH 4
#define LIGHT_WIDTH (((1 << 15))*16)
#define COUNTER_PER_BUCKET 8

// CBF
#define CBF_WIDTH ((1 << 18)*2)
#define CBF_HASHNUM 5

// NitroSketch
#define NS_PROBABILITY 0.9
#define NS_DEPTH 4
#define NS_WIDTH ((1 << 16)*2)

// UnivMon
#define UM_LAYER 4
#define UM_DEPTH 4
#define UM_WIDTH ((1 << 14)*4)
#define TOPK 100000

// CoCoSketch
#define CCS_DEPTH 2
#define CCS_WIDTH (29127*3)

// PRSketch
#define PR_WIDTH (57344*2)
#define PR_DEPTH 4
#define PR_BF_WIDTH ((1 << 20)*2)
#define PR_BF_HASH 11
#define FAST_THRESHOLD 10

#endif
