/*
除了个别要做数组下标的宏定义放data.h，其它配置统一放data.c，
*/

#ifndef _ALG_DATA_H_
#define _ALG_DATA_H_

#include <stdint.h>

extern const int ALG_VERSION;
extern const int ALG_VERSION_BIG_PRIZE;

#define MAX_DIFF_NUM        10               //难度个数
#define MAX_SEND_MAX_VALUE  100000000        //最大炒场
#define ICON_NUM            11               //图标种类
#define ICON_MULTI_MAGNIFY  100              //图标中奖倍率放大倍数
#define MUL_ICON_TYPE       14               //倍率图种类
#define BOARD_ROW           5                //盘面行数
#define BOARD_COLUMN        6                //盘面列数
#define BOARD_ICON_NUM      30               //盘面图标数量
#define ROLLER_ICON_LEN     250              //每条滚轮图标最大数量
#define MAX_CASCADE_NUM     100              //每局最大倾泻(刷新盘面)次数
#define MAX_REWARD_ICON_TYPE  4              //盘面奖励图标的最大种类
#define PROB_SUM            10000            //权重总和

extern const int E_ICON_NULL;
extern const int E_ICON_PIPC1;
extern const int E_ICON_PIPC2;
extern const int E_ICON_PIPC3;
extern const int E_ICON_PIPC4;
extern const int E_ICON_APPLE;
extern const int E_ICON_PEACH;
extern const int E_ICON_WATERMELON;
extern const int E_ICON_GRAPE;
extern const int E_ICON_BANANA;
extern const int E_ICON_MUL_BASE;
extern const int E_ICON_SCATTER;

extern const int E_PRIZETYPE_NORMAL;
extern const int E_PRIZETYPE_FREEGAME;

extern const uint16_t gIconMulti[ICON_NUM+1][BOARD_ICON_NUM+1];

extern const uint16_t gMainRollerIconNum[BOARD_COLUMN];
extern const uint8_t gMainRollerIcon[BOARD_COLUMN][ROLLER_ICON_LEN];

extern const uint16_t gSecRollerIconNum[BOARD_COLUMN];
extern const uint8_t gSecRollerIcon[BOARD_COLUMN][ROLLER_ICON_LEN];

extern const uint16_t gMulIconMulti[MUL_ICON_TYPE];

#define FREEGAME_ROUND_MAX 100           //免费游戏最大局数
extern const int gFreeGameInfo[2][2];
extern const uint16_t gFreeGameMainRollerIconNum[BOARD_COLUMN];
extern const uint8_t gFreeGameMainRollerIcon[BOARD_COLUMN][ROLLER_ICON_LEN];
extern const uint16_t gFreeGameSecRollerIconNum[BOARD_COLUMN];
extern const uint8_t gFreeGameSecRollerIcon[BOARD_COLUMN][ROLLER_ICON_LEN];
extern const uint32_t gFreeGameMulIconProb1[3];
extern const uint32_t gFreeGameMulIconProb2[3][MUL_ICON_TYPE];

extern const uint16_t gBuyFreeGameBaseRollerIconNum[BOARD_COLUMN];
extern const uint8_t gBuyFreeGameBaseRollerIcon[BOARD_COLUMN][ROLLER_ICON_LEN];
extern const uint16_t gBuyFreeGameMainRollerIconNum[BOARD_COLUMN];
extern const uint8_t gBuyFreeGameMainRollerIcon[BOARD_COLUMN][ROLLER_ICON_LEN];
extern const uint16_t gBuyFreeGameSecRollerIconNum[BOARD_COLUMN];
extern const uint8_t gBuyFreeGameSecRollerIcon[BOARD_COLUMN][ROLLER_ICON_LEN];
extern const uint32_t gBuyFreeGameMulIconProb1[3];
extern const uint32_t gBuyFreeGameMulIconProb2[3][MUL_ICON_TYPE];

#endif