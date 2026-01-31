/*
除了个别要做数组下标的宏定义放data.h，其它配置统一放data.c，
*/

#ifndef _ALG_DATA_H_
#define _ALG_DATA_H_

#include <stdint.h>

extern const int ALG_VERSION;
extern const int ALG_VERSION_BIG_PRIZE;

#define BET_LINE_TOTAL        			100              //最大全线
#define MAX_DIFF_NUM        			10               //难度个数
#define MAX_SEND_MAX_VALUE  			100000000        //最大炒场(限制最大虧分)
#define ICON_NUM            			16               //图标种类
#define OPTION_NUM            			5                //組數數量
#define BOARD_ROW           			3                //盘面行数
#define BOARD_COLUMN        			4                //盘面列数
#define BOARD_ICON_NUM      			12               //盘面图标数量
#define ROLLER_ICON_LEN      			100              //輪帶長度
#define TABLE_NUM     					4                //table数量
#define TABLE_WEIGHT_SUM     			100              //table權重總和
#define MUL_NUM     					3                //倍率符数量
#define MUL_WEIGHT_SUM     				100              //倍率符權重總和
#define SC_NUM     						3                //SCATTER数量(array index num, not realistic num)
#define ROTARY_ICON_NUM     			8                //轉盤獎項数量
#define ROTARY_WEIGHT_SUM     			1000             //轉盤權重總和
#define PERFORM_NUM     				2             	 //表演數量
#define PERFORM_WEIGHT_SUM     			100              //表演權重總和

#define DEBUG_MODE						1

#ifdef DEBUG_MODE
	#define DEBUG_PAYOUT_DIST			1
#endif	
//#define NORMAL_WIN_NUM    			1

extern const int E_ICON_NULL;     
extern const int E_ICON_PIPC500;  
extern const int E_ICON_PIPC000;   
extern const int E_ICON_PIPC100;   
extern const int E_ICON_PIPC50;   
extern const int E_ICON_PIPC10;    
extern const int E_ICON_PIPC5;     
extern const int E_ICON_PIPC1;    
extern const int E_ICON_PIPC00;    
extern const int E_ICON_PIPC0;     
extern const int E_ICON_PIPCDOT; 
extern const int E_ICON_PIPCEMPTY; 
extern const int E_ICON_MUL;       
extern const int E_ICON_RE;     
extern const int E_ICON_SC1;    
extern const int E_ICON_SC2; 
extern const int E_ICON_MUL2;     
extern const int E_ICON_MUL5;      
extern const int E_ICON_MUL10;     

extern const int E_PRIZETYPE_NORMAL;
extern const int E_PRIZETYPE_FREEGAME;
extern const int E_PRIZETYPE_RESPINGAME;

extern const int NORMAL_DIFF;

extern const uint16_t gRollerIconNum1[OPTION_NUM][BOARD_COLUMN];
extern const uint8_t gRollerIcon1[OPTION_NUM][BOARD_COLUMN][ROLLER_ICON_LEN];
extern const uint32_t gTableWeight1[OPTION_NUM][TABLE_NUM][2]; 
extern const uint32_t gBaseSymbolWeight1[OPTION_NUM][TABLE_NUM][BOARD_COLUMN][ICON_NUM][2];
extern const uint32_t gBaseSymbolWeightNum1[OPTION_NUM][TABLE_NUM][BOARD_COLUMN];
extern const uint32_t gMulWeight1[OPTION_NUM][MUL_NUM][2];
extern const uint32_t gRotaryIcon1[SC_NUM][ROTARY_ICON_NUM][2];
extern const double gTargetRTP1[OPTION_NUM][MAX_DIFF_NUM][2];

extern const uint16_t gRollerIconNum2[OPTION_NUM][BOARD_COLUMN];
extern const uint8_t gRollerIcon2[OPTION_NUM][BOARD_COLUMN][ROLLER_ICON_LEN];
extern const uint32_t gTableWeight2[OPTION_NUM][TABLE_NUM][2]; 
extern const uint32_t gBaseSymbolWeight2[OPTION_NUM][TABLE_NUM][BOARD_COLUMN][ICON_NUM][2];
extern const uint32_t gBaseSymbolWeightNum2[OPTION_NUM][TABLE_NUM][BOARD_COLUMN];
extern const uint32_t gMulWeight2[OPTION_NUM][MUL_NUM][2];
extern const uint32_t gRotaryIcon2[SC_NUM][ROTARY_ICON_NUM][2];
extern const double gTargetRTP2[OPTION_NUM][MAX_DIFF_NUM][2];

extern const uint16_t gRollerIconNum3[OPTION_NUM][BOARD_COLUMN];
extern const uint8_t gRollerIcon3[OPTION_NUM][BOARD_COLUMN][ROLLER_ICON_LEN];
extern const uint32_t gTableWeight3[OPTION_NUM][TABLE_NUM][2]; 
extern const uint32_t gBaseSymbolWeight3[OPTION_NUM][TABLE_NUM][BOARD_COLUMN][ICON_NUM][2];
extern const uint32_t gBaseSymbolWeightNum3[OPTION_NUM][TABLE_NUM][BOARD_COLUMN];
extern const uint32_t gMulWeight3[OPTION_NUM][MUL_NUM][2];
extern const uint32_t gRotaryIcon3[SC_NUM][ROTARY_ICON_NUM][2];
extern const double gTargetRTP3[OPTION_NUM][MAX_DIFF_NUM][2];

extern const uint16_t gRollerIconNum4[OPTION_NUM][BOARD_COLUMN];
extern const uint8_t gRollerIcon4[OPTION_NUM][BOARD_COLUMN][ROLLER_ICON_LEN];
extern const uint32_t gTableWeight4[OPTION_NUM][TABLE_NUM][2]; 
extern const uint32_t gBaseSymbolWeight4[OPTION_NUM][TABLE_NUM][BOARD_COLUMN][ICON_NUM][2];
extern const uint32_t gBaseSymbolWeightNum4[OPTION_NUM][TABLE_NUM][BOARD_COLUMN];
extern const uint32_t gMulWeight4[OPTION_NUM][MUL_NUM][2];
extern const uint32_t gRotaryIcon4[SC_NUM][ROTARY_ICON_NUM][2];
extern const double gTargetRTP4[OPTION_NUM][MAX_DIFF_NUM][2];

extern const uint32_t gPerformAction[PERFORM_NUM][2];
extern const uint32_t gPerformPosition[PERFORM_NUM][2];


#endif