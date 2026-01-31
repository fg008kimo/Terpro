#ifndef FISHERMAN_H_
#define FISHERMAN_H_

#include "../data.h"
#include "../utils.h"

int gPayIconScore[GAMETYPE][ICON_NUM][BOARD_COLUMN];

//生成初始盘面
int gen_board(const uint8_t (*rollerIcon)[BOARD_COLUMN][ROLLER_ICON_LEN], const uint16_t (*rollerIconNum)[BOARD_COLUMN], const int (*reelWeight)[2], int len, int* icons);

//计算某个图标的个数
int calIconNum(int *icons, int icon);

//计算盘面的得分，并记录中奖线的信息
void cal_boardMulti(int prizeType, int *icons, int *gameMulti, int scatType);

//根据倍率转换图标类型，以便发给后端
void changeIcon(int prizeType, int* icons, int icon, int *gameMulti);

//将一种普通-稀有图标转换成牛头符
void normalIconsToBuff(int reelPos, int *icons);

//将免费游戏中满足条件的符号转换为牛头符
void freeIconsToBuff(int *icons, int buffCounter);

//判断图标是不是百搭图标
int isWildIcon(int icon);

//获取游戏类型
int getPrizeType(int *icons);

//获取权重的随机数
int getRandomValue(const int (*weight)[2], int len, int weightValue);

#endif