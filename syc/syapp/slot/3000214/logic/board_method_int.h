#ifndef _BOARD_H_
#define _BOARD_H_

#include "../utils.h"
#include "../data.h"


// 生成盘面
void board_gen(const int (*tableWeight)[TABLE_NUM][2], const int tableLen, const int (*symbolWeight)[TABLE_NUM][BOARD_COLUMN][ICON_NUM][2], const int (*symbolLen)[TABLE_NUM][BOARD_COLUMN], const int (*mulWeight)[MUL_NUM][2], int *icons, int respinIcon, int option);

// 计算盘面倍率，并填充得分线信息
double board_calMulti(int *icons);

//產生random函數
int getRandomValue(const int (*weight)[2], int len, int weightValue);
#endif
