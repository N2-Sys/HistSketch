#ifndef _PARAMETERS_H
#define _PARAMETERS_H

#define CHARKEY_LEN 13
#define DATA_T_SIZE 15
#define BUCKET_WIDTH 94
#define BUCKET_NUM 16
#define POINT_ARETHRESHOLD 0.01
#define HISTOGRAM_ARETHRESHOLD 0.01

// CM Sketch
#define CM_DEPTH 4
#define CM_WIDTH (1 << 15)

// CU Sketch
#define CU_DEPTH 4
#define CU_WIDTH (1 << 15)

// Count Sketch
#define CS_DEPTH 4
#define CS_WIDTH (1 << 15)

// FlowRadar
#define BF_WIDTH (1 << 19)
#define BF_HASHNUM 11
#define IBLT_WIDTH (1 << 15) - (1 << 13)
#define IBLT_HASHNUM 3

// SuMaxSketch
#define SM_DEPTH 4
#define SM_WIDTH (1 << 15)

// Elastic Sketch
#define SLOT_NUM (1 << 8)
#define LIGHT_DEPTH 4
#define LIGHT_WIDTH (1 << 11)
#define COUNTER_PER_BUCKET 8

// CBF
#define CBF_WIDTH (1 << 17)
#define CBF_HASHNUM 5

// NitroSketch
#define NS_PROBABILITY 0.9
#define NS_DEPTH 4
#define NS_WIDTH (1 << 15)

// UnivMon
#define UM_LAYER 4
#define UM_DEPTH 4
#define UM_WIDTH (1 << 13)

// CoCoSketch
#define CCS_DEPTH 2
#define CCS_WIDTH (1927 * 8)

#endif
