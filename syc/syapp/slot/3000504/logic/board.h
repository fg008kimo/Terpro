#ifndef _BOARD_H_
#define _BOARD_H_

#include "../utils.h"
#include "../data.h"


//原始隨機函數
#ifdef GET_RAND
void cal_rand(s16 *rand_num);
#endif

#ifdef SRAND
int randint(int n);
//DY隨機函數
void cal_rand_inbuild(s16 *rand_num1);
#endif

// 生成盘面
#ifdef REEL_DIST
void board_gen_dist(const uint8_t (*rollerIcon)[ROLLER_ICON_LEN], const uint16_t *rollerIconNum, int *icons, s16 (*rollerDist)[ROLLER_ICON_LEN], s16 *rollerDist1, s16 *rollerDist2, s16 *rollerDist3, s16 *rollerDist4, s16 *rollerDist5, s16 *rollerDist6, s16 *rollerDist7, s16 *rollerDist8);
#else
void board_gen(const uint8_t (*rollerIcon)[ROLLER_ICON_LEN], const uint16_t *rollerIconNum, int *icons);
#endif

// 生成盘面（遍历方式）[测试用]
void board_traverse_gen(const uint8_t (*rollerIcon)[ROLLER_ICON_LEN], const uint16_t *rollerIconNum, int *icons);

// 计算盘面倍率，并填充得分线信息
int board_calMulti(s16 lineNum, int *icons, int (*lineInfos)[4], s16 *awardLineNum);

// 判断盘面类型
int board_calPrizeType(int *icons);

// 统计盘面中某图标个数
int board_calIconNum(int *icons, int targetIcon);

// 盤面VS換成倍率符
void IconsToVSsymbol(int *iconsBefore, int *icons);

// 生成倍率
#ifdef WILD_MUL_DIST
void wildGetMultiplier(int *icons, const uint32_t (*gMulValue)[2], const int WEIGHT_SUM, s16 *wildMul);
#else
void wildGetMultiplier(int *icons, const uint32_t (*gMulValue)[2], const int WEIGHT_SUM);
#endif

//產生random函數
int getRandomValue(const uint32_t (*weight)[2], int len, int weightValue);
#endif
