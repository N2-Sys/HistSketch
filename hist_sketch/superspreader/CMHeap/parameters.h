#ifndef _PARAM_H_
#define _PARAM_H_

#define CHARKEY_LEN 13
#define INTKEY_LEN 4
#define DATA_T_SIZE 15

// metrics
#define SS_TRUE_THRESHOLD 50
#define SS_ESTIMATE_THRESHOLD 50

#define CMH_B 457
#define CMH_BB 457 * 2
#define CMH_C 3
#define CMH_DEPTH 4
#define CMH_WIDTH (1 << 13) + (1 << 10)
#define HEAP_COUNT 100

#endif