#ifndef RNG_H
#define RNG_H

#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <pthread.h>

#include "../uthash.h"

#define RNG_READ_LEN_MAX 10240
#define RNG_WRITE_LEN_MAX 64
#define RNG_RANDOM_NUM 1000  //必须小于等于randomNumberGenerator工程的RNG_RANDOM_NUM

enum rng_pack_state {
	eRngPackStateHeadLenInComplete = 0, // 包头的长度字段不完整
	eRngPackStateContentInComplete,     // 包内容不完整
	eRngPackStateComplete,              // 完整的包
};

struct RngConnectClient {
    int sock;

    int rState;
    uint32_t rHeadLen;
    uint32_t rHeadId;
    uint32_t rHeadMinValue;
    uint32_t rHeadMaxValue;
    char rBuf[RNG_READ_LEN_MAX];
    int rBufUsefulInfoLen;

	UT_hash_handle hh_rng_sock;   
};

struct RngNetDb {
	struct RngConnectClient client;
    pthread_mutex_t requestMutex;
	int epollfd;
};

struct RngDoubleBufData {
    double data[RNG_RANDOM_NUM];
    int index;
};

struct RngBufData {
    long key;

    uint32_t data[RNG_RANDOM_NUM];
    int index;

    UT_hash_handle hh_rng_buf; 
};

// 初始化
void RngInit(int algPort);

// 返回[0,1)随机浮点数
double RngGetRandom();

// 返回[minValue,maxValue)随机整数
uint32_t RngGetRand(uint32_t minValue,uint32_t maxValue);

#endif

