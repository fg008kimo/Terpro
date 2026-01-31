/*
除了个别要做数组下标的宏定义放data.h，其它配置统一放data.c，
*/

#ifndef _ALG_DATA_H_
#define _ALG_DATA_H_

#include <stdint.h>

extern const int ALG_VERSION;
extern const int ALG_VERSION_BIG_PRIZE;

#define GAMETYPE            2                //游戏类型
#define BET_LINE_TOTAL      75               //全线游戏
#define MAX_DIFF_NUM        10               //难度个数
#define MAX_SEND_MAX_VALUE  100000000        //最大炒场
#define ICON_NUM            17               //图标种类
#define ROTARY_ICON_NUM     12               //转盘图标数量
#define BOARD_ROW           4                //盘面行数
#define BOARD_COLUMN        5                //盘面列数
#define BOARD_ICON_NUM      20               //盘面图标数量
#define ROLLER_ICON_LEN     60               //每条滚轮图标最大数量
#define WILD_WEIGHT         100              //百搭权重
#define REEL_WEIGHT         10000            //滚轴权重
#define MAX_WIN_MULTI       10000            //最大的赢倍
#define BUFF_BONUS_BOUNDARY 6                //bonus和buff的分界线
#define BASE_REEL_NUM       8                //滚轴数量
#define BONUS_NUM           5                //转盘奖金的个数

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
extern const int E_ICON_9;
extern const int E_ICON_WILD;
extern const int E_ICON_NORMAL_SCAT;
extern const int E_ICON_FREE_SCAT;
extern const int E_ICON_BUFF;

extern const int E_PRIZETYPE_NORMAL;
extern const int E_PRIZETYPE_FREEGAME;

extern const int gBaseReelWeight[BASE_REEL_NUM+4][2];
extern const int gIconMulti[ICON_NUM][BOARD_COLUMN+1];
extern const uint16_t gRollerIconNum[BASE_REEL_NUM][BOARD_COLUMN];
extern const uint8_t gRollerIcon[BASE_REEL_NUM][BOARD_COLUMN][ROLLER_ICON_LEN];
extern const int gNormalWildWeight[3][2];

#define FREEGAME_TYPE 4                  //免费游戏种类
#define FREEGAME_ROUND_MAX 100           //免费游戏最大局数

extern const int gFreeReelWeight[BASE_REEL_NUM][2];
extern const int gFreeGameInfo[FREEGAME_TYPE][2];
extern const uint16_t gFreeGameRollerIconNum[BASE_REEL_NUM][BOARD_COLUMN];
extern const uint8_t gFreeGameRollerIcon[BASE_REEL_NUM][BOARD_COLUMN][ROLLER_ICON_LEN];
extern const int gWildMulti[3][2];
extern const int gFreeWildWeight[3][2];
extern const int gPic1ToGoldWeight[3];
extern const uint16_t gBuyFreeGameRollerIconNum[BASE_REEL_NUM][BOARD_COLUMN];
extern const uint8_t gBuyFreeGameRollerIcon[BASE_REEL_NUM][BOARD_COLUMN][ROLLER_ICON_LEN];

#define ROTARY_INIT_ROUND  5
extern const int gBuffChangeIcons[4][2];
extern const int gRotaryIconNum[3];
extern const int gRotaryIcon[3][BONUS_NUM];
extern const int hitWeight[ROTARY_ICON_NUM][2];

// #define PRINT   

#endif