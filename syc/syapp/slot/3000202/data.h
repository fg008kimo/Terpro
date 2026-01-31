/*
除了个别要做数组下标的宏定义放data.h，其它配置统一放data.c，
*/

#ifndef _ALG_DATA_H_
#define _ALG_DATA_H_

#include <stdint.h>

extern const int ALG_VERSION;
extern const int ALG_VERSION_BIG_PRIZE;

#define BET_LINE_TOTAL      			15               //压线数
#define MAX_DIFF_NUM        			10               //难度个数
#define MAX_SEND_MAX_VALUE  			100000000        //最大炒场
#define ICON_NUM            			21               //图标种类
#define BOARD_ROW           			5                //盘面行数
#define BOARD_COLUMN        			5                //盘面列数
#define BOARD_ICON_NUM      			25               //盘面图标数量
#define ROLLER_ICON_LEN     			385              //每条滚轮图标最大数量
#define MUL_NUM     					12               //權重数量
#define BASEGAME_WEIGHT_SUM     		483              //BASE GAME 權重總和
#define FREEGAME_WEIGHT_SUM     		165              //FREE GAME 權重總和
#define BUYFREEGAME_WEIGHT_SUM     		229              //BUY FREE GAME 權重總和
#define FREE_GAME_NUM     				10               //免費遊戲次數

//#define disable_freegame			1
//#define PRINT_BOARD    			1
//#define DEBUG_PAYOUT_DIST			1

extern const int E_ICON_NULL;
extern const int E_ICON_PIPC1;
extern const int E_ICON_PIPC2;
extern const int E_ICON_PIPC3;
extern const int E_ICON_PIPC4;
extern const int E_ICON_PIPC5;
extern const int E_ICON_A;
extern const int E_ICON_K;
extern const int E_ICON_Q;
extern const int E_ICON_J;
extern const int E_ICON_10;
extern const int E_ICON_WILD;
extern const int E_ICON_SC1;
extern const int E_ICON_SC2;
extern const int E_ICON_SC3;
extern const int E_ICON_VS;

extern const int E_PRIZETYPE_NORMAL;
extern const int E_PRIZETYPE_FREEGAME;
extern const int E_PRIZETYPE_STICKY_WILD;
extern const int E_PRIZETYPE_BOBUSGAME;

extern const uint8_t gLinePos[BET_LINE_TOTAL][BOARD_COLUMN];

extern const uint16_t gIconMulti[ICON_NUM+1][BOARD_COLUMN+1];

extern const uint16_t gRollerIconNum[BOARD_COLUMN];
extern const uint8_t gRollerIcon[BOARD_COLUMN][ROLLER_ICON_LEN];
extern const uint32_t gBaseGameWeight[MUL_NUM][2];

#define FREEGAME_TYPE 3                  //免费游戏种类
#define FREEGAME_ROUND_MAX 20            //免费游戏最大局数

extern const int gFreeGameInfo[FREEGAME_TYPE][2];
extern const uint16_t gFreeGameRollerIconNum[BOARD_COLUMN];
extern const uint8_t gFreeGameRollerIcon[BOARD_COLUMN][ROLLER_ICON_LEN];
extern const uint32_t gFreeGameWeight[MUL_NUM][2];

extern const uint16_t gStickyGameRollerIconNum[BOARD_COLUMN];
extern const uint8_t gStickyGameRollerIcon[BOARD_COLUMN][ROLLER_ICON_LEN];

extern const uint16_t gBuyFreeGameRollerIconNum[BOARD_COLUMN];
extern const uint8_t gBuyFreeGameRollerIcon[BOARD_COLUMN][ROLLER_ICON_LEN];
extern const uint32_t gBuyFreeGameWeight[MUL_NUM][2];

#endif