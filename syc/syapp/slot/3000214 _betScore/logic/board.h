#ifndef _BOARD_H_
#define _BOARD_H_

#include "../utils.h"
#include "../data.h"


// 生成盘面
void board_gen(const uint32_t (*tableWeight)[TABLE_NUM][2], const int tableLen, const uint32_t (*symbolWeight)[TABLE_NUM][BOARD_COLUMN][ICON_NUM][2], const uint32_t (*symbolLen)[TABLE_NUM][BOARD_COLUMN], const uint32_t (*mulWeight)[MUL_NUM][2], const uint8_t (*rollerIcon)[BOARD_COLUMN][ROLLER_ICON_LEN], const uint16_t (*rollerIconNum)[BOARD_COLUMN], int *icons, int respinIcon, int option);

// 计算盘面倍率，并填充得分线信息
s64 board_calMulti(int *icons, int times);

// 生成表演盘面
void board_genPerform(const uint32_t (*performActWeight)[2], const uint32_t (*performPosWeight)[2], const int performLen, const uint8_t (*rollerIcon)[BOARD_COLUMN][ROLLER_ICON_LEN], const uint16_t (*rollerIconNum)[BOARD_COLUMN], int *iconsBefore, int *icons, int option, s64 normalMulti);

//產生random函數
int getRandomValue(const uint32_t (*weight)[2], int len, int weightValue);

//难度调控
void difficulty_control(int diff, int upMulti, int downMulti, const double (*targetRTP)[MAX_DIFF_NUM][2], int isSelect_A, s32 totalMulti, int *loopTime, int option);

#endif
