/*
随机数
*/

#ifndef RAND_H
#define RAND_H

#include <unistd.h>
#include <stdint.h>

enum rand_type {
	E_RAND_MERSENNE_TWISTER, //梅森旋转法
};

// 随机数初始化
void RDInit();

// 返回[0,1)随机浮点数
double RDGetRandom();

// 返回[minValue,maxValue)随机整数
uint32_t RDGetRand(uint32_t minValue,uint32_t maxValue);

#endif

