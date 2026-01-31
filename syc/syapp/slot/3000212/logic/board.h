#ifndef _BOARD_H_
#define _BOARD_H_

#include "../utils.h"
#include "../data.h"

// 生成盘面
void board_gen(const uint8_t (*rollerIcon)[ROLLER_ICON_LEN], const uint16_t *rollerIconNum, int *icons, const uint32_t *prizeSymbolWeight, int PRIZE_PROB_SUM);

// 计算盘面倍率，并填充得分线信息
int board_calMulti(s16 lineNum, int *icons, int (*lineInfos)[3], s16 *awardLineNum);

// 判断盘面类型
int board_calPrizeType(int FREEGAME_THRESHOLD);

// 判断動畫类型
int calAnimeType(int ANIME_THRESHOLD);

//// 统计盘面中某图标个数
//int board_calIconNum(int *icons, int targetIcon);

// 统计盘面中寶藏图标个数
int board_calPrizeNum(int *icons, int targetIcon);

// 统计盘面中寶藏图标倍率
//double board_calPrizeMulti(int *icons);
int board_calPrizeMulti(int *icons);

//获取权重的随机数
int getRandomValue(const uint32_t* weight, int len, int weightValue);

//选择AB权重的版本号
int selectWeight(s8 uid);

//难度调控
void difficulty_control(s16 lineNum, int diff, int upMulti, int downMulti, const double (*targetRTP)[3], int isSelect_A, s32 totalMulti, int *loopTime);

#endif
