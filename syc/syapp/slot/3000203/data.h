/*
除了个别要做数组下标的宏定义放data.h，其它配置统一放data.c，
*/

#ifndef _ALG_DATA_H_
#define _ALG_DATA_H_

#include <stdint.h>

extern const int ALG_VERSION;
extern const int ALG_VERSION_BIG_PRIZE;

#define GAMETYPE            2                //游戏类型
#define BET_LINE_TOTAL      19               //最多压线数
#define MAX_DIFF_NUM        10               //难度个数
#define MAX_SEND_MAX_VALUE  100000000        //最大炒场
#define ICON_NUM            13               //图标种类
#define WILD_TYPE_NUM       15               //百搭图标种类
#define BOARD_ROW           5                //盘面行数
#define BOARD_COLUMN        5                //盘面列数
#define BOARD_ICON_NUM      25               //盘面图标数量
#define SELECT_VERSION_NUM  9                //可供选择的版本
#define VERSION_WEIGHT      10000            //可供选择版本的总权重
#define MAX_ADD_ROUND       5                //免费又免费的最大次数
#define ROLLER_ICON_LEN     300              //每条滚轮图标最大数量
#define WILD_WEIGHT         2000             //百搭权重
#define REEL_WEIGHT         1000             //滚轴权重
#define MAX_WIN_MULTI       12500            //最大的赢倍

extern const int E_ICON_PIC1;
extern const int E_ICON_PIC2;
extern const int E_ICON_PIC3;
extern const int E_ICON_PIC4;
extern const int E_ICON_PIC5;
extern const int E_ICON_A;
extern const int E_ICON_K;
extern const int E_ICON_Q;
extern const int E_ICON_J;
extern const int E_ICON_10;
extern const int E_ICON_WILD;
extern const int E_ICON_CAT;
extern const int E_ICON_SCAT;

extern const int E_PRIZETYPE_NORMAL;
extern const int E_PRIZETYPE_FREEGAME;

extern const int NORMAL_DIFF;

extern const double gTargetRTP[10][3];
extern const double gBuyTargetRTP[10][3];
extern const int gSelectVersion[SELECT_VERSION_NUM][2];

extern const uint8_t gLinePos[BET_LINE_TOTAL][BOARD_COLUMN];
extern const int gIconMultis[ICON_NUM][BOARD_COLUMN+1];
extern const uint16_t gRollerIconNum_A[BOARD_COLUMN];
extern const uint8_t gRollerIcon_A[BOARD_COLUMN][ROLLER_ICON_LEN];
extern const uint16_t gRollerIconNum_B[BOARD_COLUMN];
extern const uint8_t gRollerIcon_B[BOARD_COLUMN][ROLLER_ICON_LEN];
extern const int gWildMultis[WILD_TYPE_NUM][2];

#define FREEGAME_TYPE 2            //免费游戏种类
#define FREEGAME_ROUND_MAX 30      //免费游戏最大局数
#define ADD_ROUND    4             //增加免费游戏的局数

extern const int gFreeGameInfo[FREEGAME_TYPE][2];
extern const int gScatInFreeRell[3];
extern const int gScatAppearInRow[BOARD_ROW][2];
extern const uint16_t gUnactivatedRollerIconNum_A[BOARD_COLUMN];
extern const uint8_t gUnactivatedRollerIcon_A[BOARD_COLUMN][ROLLER_ICON_LEN];
extern const uint16_t gActivatedRollerIconNum_A[BOARD_COLUMN];
extern const uint8_t gActivatedRollerIcon_A[BOARD_COLUMN][ROLLER_ICON_LEN];

extern const uint16_t gUnactivatedRollerIconNum_B[BOARD_COLUMN];
extern const uint8_t gUnactivatedRollerIcon_B[BOARD_COLUMN][ROLLER_ICON_LEN];
extern const uint16_t gActivatedRollerIconNum_B[BOARD_COLUMN];
extern const uint8_t gActivatedRollerIcon_B[BOARD_COLUMN][ROLLER_ICON_LEN];

extern const uint16_t gBuyFreegameRollerIconNum[BOARD_COLUMN];
extern const uint8_t gBuyFreegameRollerIcon[BOARD_COLUMN][ROLLER_ICON_LEN];


#endif