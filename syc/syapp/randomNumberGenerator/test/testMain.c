#include <stdio.h>

#include "testMain.h"
#include "../utils.h"

#define RANDOM_ROUND 100000000
#define RANDOM_MAX 10000

long randomCnt[RANDOM_MAX];
//统计每个数字命中的次数，拷贝到excel上画图
void random_handle_cnt(uint32_t v)
{
	randomCnt[v]++;
}
void random_print_cnt()
{
	int num;
	for(num = 0; num < RANDOM_MAX; num++)
		printf("%d %ld\n", num, randomCnt[num]);
}

long randomOddEven[2];
//奇数/偶数个数
void random_handle_odd_even(uint32_t v)
{
	randomOddEven[v&1]++;
}
void random_print_odd_even()
{
	printf("odd/even=%f\n", (double)randomOddEven[1] / randomOddEven[0]);
}

long randomUnits[10];
//数值中个位数是0~9的个数
void random_handle_units(uint32_t v)
{
	randomUnits[v%10]++;
}
void random_print_units()
{
	int units;
	for(units = 0; units < 10; units++)
		printf("[%d]=%f\n", units, (double)randomUnits[units] / RANDOM_ROUND);
}

long randomBin[2];
//统计二进制中1和0的个数（应该无限接近1:1?）
void random_handle_bin(uint32_t v)
{
	if(v == 0)
		randomBin[0]++;
	while(v > 0)
	{
		randomBin[v&1]++;
		v = v >> 1;
	}
}
void random_print_bin()
{
	printf("bin_1/bin_0=%f\n", (double)randomBin[1] / randomBin[0]);
}

void random_handle(uint32_t v)
{
	random_handle_cnt(v);
	random_handle_odd_even(v);
	random_handle_units(v);
	random_handle_bin(v);
}
void random_print()
{
	// random_print_cnt();
	printf("【odd-even】========================\n");
	random_print_odd_even();
	printf("【units】========================\n");
	random_print_units();
	printf("【bin】========================\n");
	random_print_bin();
	printf("========================\n");
}

void random_writeFile(double v)
{
	char filePath[1000];
	FILE *fp;

	snprintf(filePath, sizeof(filePath), "./random_num.txt"); //random_num.txt必须先存在
	if((fp = fopen(filePath, "a")) == NULL)
	{
		printf("fopen error 1\n");
		return;
	}

	fprintf(fp, "%.16f\n", v);

	fclose(fp);
}

/////////////////////////////【对外接口 start】////////////////////////////////
void TestMain()
{
	long i;
	for(i = 0; i < RANDOM_ROUND; i++)
	{
		random_handle(GetRand(0, RANDOM_MAX));
	}
	random_print();
}
/////////////////////////////【对外接口 end】//////////////////////////////////