#ifndef _BOARD_H_
#define _BOARD_H_

#include "../utils.h"
#include "../data.h"

// 生成盘面
void board_gen(const uint8_t (*rollerIcon)[ROLLER_ICON_LEN], const uint16_t *rollerIconNum, int *icons);

// 生成盘面（遍历方式）[测试用]
void board_traverse_gen(const uint8_t (*rollerIcon)[ROLLER_ICON_LEN], const uint16_t *rollerIconNum, int *icons, int *mul);

// 计算盘面倍率，并填充得分线信息
int board_calMulti(s16 lineNum, int *icons, int (*lineInfos)[4], s16 *awardLineNum, int *mul);

// 判断盘面类型
int board_calPrizeType(int *icons);

// 统计盘面中某图标个数
int board_calIconNum(int *icons, int targetIcon);

// 盤面VS換成倍率符
void IconsToVSsymbol(int *iconsBefore, int *icons);

// 生成倍率符
void genMultiplier(const uint32_t (*gMulValue)[2], const int WEIGHT_SUM, int *mul);

//產生random函數
int getRandomValue(const uint32_t (*weight)[2], int len, int weightValue);
#endif
