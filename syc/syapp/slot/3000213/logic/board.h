#ifndef _BOARD_H_
#define _BOARD_H_

#include "../utils.h"
#include "../data.h"

// 生成盘面
int board_gen(const uint8_t (*rollerIcon)[ROLLER_ICON_LEN], const uint16_t *rollerIconNum, const uint16_t (*gMulSymbolWeight)[2], int *icons);

// 生成盘面（遍历方式）[测试用]
int board_traverse_gen(const uint8_t (*rollerIcon)[ROLLER_ICON_LEN], const uint16_t *rollerIconNum, int *icons);

// 计算盘面倍率，并填充得分线信息
int board_calMulti(s16 lineNum, int *icons, int (*lineInfos)[3], s16 *awardLineNum, int multiplier);

//获取权重的随机数
int getRandomValue(const uint16_t (*weight)[2], int len, int weightValue);

//难度调控
void difficulty_control(s16 lineNum, int diff, int upMulti, int downMulti, const double (*targetRTP)[2], int isSelect_A, s32 totalMulti, int *loopTime);

#endif
