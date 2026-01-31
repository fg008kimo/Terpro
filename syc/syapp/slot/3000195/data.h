/*
除了个别要做数组下标的宏定义放data.h，其它配置统一放data.c，
*/

#ifndef _ALG_DATA_H_
#define _ALG_DATA_H_

#include <stdint.h>

extern const int ALG_VERSION;
extern const int ALG_VERSION_BIG_PRIZE;

#define BET_LINE_TOTAL      25               //最多压线数
#define MAX_DIFF_NUM        10               //难度个数
#define MAX_SEND_MAX_VALUE  100000000        //最大炒场
#define ICON_NUM            11               //图标种类
#define HIGH_ICON_NUM       4                //高分图标种类
#define BOARD_ROW           3                //盘面行数
#define BOARD_COLUMN        5                //盘面列数
#define BOARD_ICON_NUM      15               //盘面图标数量
#define ROLLER_ICON_LEN     100              //每条滚轮图标最大数量

extern const int E_ICON_NULL;
extern const int E_ICON_PIPC1;
extern const int E_ICON_PIPC2;
extern const int E_ICON_PIPC3;
extern const int E_ICON_PIPC4;
extern const int E_ICON_SCATTER;
extern const int E_ICON_A;
extern const int E_ICON_K;
extern const int E_ICON_Q;
extern const int E_ICON_J;
extern const int E_ICON_10;
extern const int E_ICON_WILD;

extern const int E_PRIZETYPE_NORMAL;
extern const int E_PRIZETYPE_FREEGAME;

extern const uint8_t gLinePos[BET_LINE_TOTAL][BOARD_COLUMN];

extern const uint16_t gIconMulti[ICON_NUM+1][BOARD_COLUMN+1];//gIconMulti[12][6]

extern const uint16_t gRollerIconNum[BOARD_COLUMN];
extern const uint8_t gRollerIcon[BOARD_COLUMN][ROLLER_ICON_LEN];

#define FREEGAME_TYPE 3                  //免费游戏种类
#define FREEGAME_ROUND_MAX 25            //免费游戏最大局数
#define FREEGAME_CHANGE_WILD_MODE_NUM 15 //免费游戏转WILD模式个数
#define FREEGAME_CHANGE_WILD_MODE_PROB_SUM 10000 //免费游戏转WILD模式的权重总和
extern const int gFreeGameInfo[FREEGAME_TYPE][2];
extern const uint16_t gFreeGameRollerIconNum[BOARD_COLUMN];
extern const uint8_t gFreeGameRollerIcon[BOARD_COLUMN][ROLLER_ICON_LEN];
extern const uint8_t gFreeGameChangeWildMode[FREEGAME_CHANGE_WILD_MODE_NUM][5];
extern const uint32_t gFreeGameChangeWildModeProb[FREEGAME_CHANGE_WILD_MODE_NUM];

extern const uint16_t gBuyFreeGameRollerIconNum[BOARD_COLUMN];
extern const uint8_t gBuyFreeGameRollerIcon[BOARD_COLUMN][ROLLER_ICON_LEN];
extern const uint8_t gBuyFreeGameChangeWildMode[FREEGAME_CHANGE_WILD_MODE_NUM][5];
extern const uint32_t gBuyFreeGameChangeWildModeProb[FREEGAME_CHANGE_WILD_MODE_NUM];

#endif