#ifndef RNG_H
#define RNG_H

#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <pthread.h>

#include "tools/uthash.h"

#define RNG_READ_LEN_MAX 64
#define RNG_WRITE_LEN_MAX 10240

enum rng_pack_state {
	eRngPackStateHeadLenInComplete = 0, // 包头的长度字段不完整
	eRngPackStateContentInComplete,     // 包内容不完整
	eRngPackStateComplete,              // 完整的包
};

struct RngConnectClient {
    int sock;
    time_t last;

    int rState;
    uint32_t rHeadLen;
    uint32_t rHeadId;
    uint32_t rHeadMinValue;
    uint32_t rHeadMaxValue;
    char rBuf[RNG_READ_LEN_MAX];
    int rBufUsefulInfoLen;

	UT_hash_handle hh_rng_sock;   
};

struct RngRemoveSock {
    int sock;
    UT_hash_handle hh_rng_remove_sock; 
};

struct RngNetDb {
	int listenSock;
	struct RngConnectClient *clients;
    struct RngRemoveSock *rmSocks;
    pthread_mutex_t rmSocksMutex;
	int epollfd;
};

// 随机数发生器主循环(仿制_mammon_main_loop)
void RngMainLoop(int port);

#endif

