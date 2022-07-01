#ifndef _PARAMETERS_H
#define _PARAMETERS_H

#define CHARKEY_LEN 11
#define DATA_T_SIZE 13
#define BUCKET_WIDTH 100	// change if test latency
#define BUCKET_NUM 16
#define POINT_ARETHRESHOLD 0.01
#define HISTOGRAM_ARETHRESHOLD 0.01
#define HC_SIZE 10

// CM Sketch
#define CM_DEPTH 4
#define CM_WIDTH (1 << 16)

// CU Sketch
#define CU_DEPTH 4
#define CU_WIDTH (1 << 16)

// Count Sketch
#define CS_DEPTH 4
#define CS_WIDTH (1 << 16)

// FlowRadar
#define BF_WIDTH (1 << 21)
#define BF_HASHNUM 11
#define IBLT_WIDTH (1 << 16)
#define IBLT_HASHNUM 3

// SuMaxSketch
#define SM_DEPTH 4
#define SM_WIDTH (1 << 16)

// Elastic Sketch
#define SLOT_NUM (1 << 12)
#define LIGHT_DEPTH 4
#define LIGHT_WIDTH (1 << 15)
#define COUNTER_PER_BUCKET 8

// CBF
#define CBF_WIDTH (1 << 18)
#define CBF_HASHNUM 5

// NitroSketch
#define NS_PROBABILITY 0.9
#define NS_DEPTH 4
#define NS_WIDTH (1 << 16)

// UnivMon
#define UM_LAYER 4
#define UM_DEPTH 4
#define UM_WIDTH (1 << 14)
#define TOPK 100000

// CoCoSketch
#define CCS_DEPTH 2
#define CCS_WIDTH (29127)

#endif
