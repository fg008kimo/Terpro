#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>
#include <stdio.h>

#include "data.h"

#ifdef __LINUX__
#include "zlog.h"
#endif

#if (defined(__LINUX__) && defined(__ZLOG__)) 
    #define LOGD dzlog_debug
    #define LOGI dzlog_info
    #define LOGW dzlog_warn
    #define LOGE dzlog_error
#else
    #define __STR(R)    #R
    #define STR(R)  __STR(R)
    #define LOGD(...)   do{printf("\n");printf(__VA_ARGS__);}while(0)
    #define LOGI(...)   do{printf("\n");printf(__VA_ARGS__);}while(0)
    #define LOGW(...)   do{printf("\n");printf(__VA_ARGS__);}while(0)
    #define LOGE(...)   do{printf("\n");printf(__VA_ARGS__);}while(0)
#endif

//跨平台编译但不支持实际运行
#ifndef __LINUX__
/* epoll */
#define EPOLL_CTL_ADD 1
#define EPOLL_CTL_DEL 2
#define EPOLL_CTL_MOD 3
#define EPOLLIN 1
#define EPOLLPRI 1
#define EPOLLOUT 1
typedef union epoll_data
{
    void *ptr;
    int fd;
    uint32_t u32;
    uint64_t u64;
} epoll_data_t;
struct epoll_event {
    uint32_t events;	/* Epoll events */
    epoll_data_t data;	/* User data variable */
};
int epoll_create (int __size);
int epoll_ctl (int __epfd, int __op, int __fd,struct epoll_event *__event);
int epoll_wait (int __epfd, struct epoll_event *__events, int __maxevents, int __timeout);
/* zlog */
int dzlog_init(const char *confpath, const char *cname);
#endif

// 申请动态内存
void *alg_malloc(size_t size);
// 申请动态内存
void *alg_calloc(size_t nmemb, size_t size);
// 销毁动态内存
void alg_free(void *mem);

// 随机数初始化
void RandInit();

// 返回[0,1)随机浮点数
double GetRandom();

// 返回[minValue,maxValue)随机整数
uint32_t GetRand(uint32_t minValue,uint32_t maxValue);

#endif

