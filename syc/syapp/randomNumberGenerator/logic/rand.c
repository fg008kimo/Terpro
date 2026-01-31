/*
随机数
*/

#include <stdio.h>
#include "rand.h"
#include "mt.h"

static int randType = E_RAND_MERSENNE_TWISTER;

void RDInit()
{
	switch(randType)
	{
		case E_RAND_MERSENNE_TWISTER:
			MTInit();
			break;
		default:
			printf("RDInit: randType[%d] error\n", randType);
			break;
	}
}

double RDGetRandom()
{
	switch(randType)
	{
		case E_RAND_MERSENNE_TWISTER:
			return MTGetRandom();
		default:
			printf("RDGetRandom: randType[%d] error\n", randType);
			return 0;
	}
}

uint32_t RDGetRand(uint32_t minValue,uint32_t maxValue)
{
	switch(randType)
	{
		case E_RAND_MERSENNE_TWISTER:
			return MTGetRand(minValue, maxValue);
		default:
			printf("RDGetRand: randType[%d] error\n", randType);
			return 0;
	}
}