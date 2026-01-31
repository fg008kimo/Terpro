/*
梅森旋转法
*/

#ifndef _MT_H_
#define _MT_H_

#include <unistd.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif
// 随机数初始化
void MTInit();

// 返回[0,1)随机浮点数
double MTGetRandom();

// 返回[minValue,maxValue)随机整数
uint32_t MTGetRand(uint32_t minValue,uint32_t maxValue);
#ifdef __cplusplus
}
#endif

#endif