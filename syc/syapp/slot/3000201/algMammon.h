#ifndef ALG_MAMMON_AAA_H
#define ALG_MAMMON_AAA_H

#ifdef __cplusplus
extern "C" {
#endif

#include <time.h>

#include "utils.h"
#include "data.h"

#define ALG_MAMMON_VERSION     "S19052901"

#define ALG_MAMMON_PORT        19522
#define ALG_MAMMON_PASSWORD    "abcd1234" 


#define MAX_TABLES_NUM 10000
#define DEFAULT_TABLE_ID 0

enum mammon_method {
	mammonKillFish = 0,          // 获取开奖结果(STR_AlgKillFish,STR_AlgKillFishResult)
	mammonInit = 1,              // 初始化(STR_AlgInit,STR_AlgInitResult)
	mammonGetData = 2,           // 获取算法数据(STR_AlgTestData,STR_AlgTestDataResult)
	mammonReset = 3,             // 算法重置(STR_AlgReset,STR_AlgResetResult)
	mammonRevise = 4,            // in、out修正(STR_AlgInOutRevise,STR_AlgInOutReviseResult)
	mammonSetMaxSend = 5,        // 调整炒场额度(STR_AlgSetMaxSend,STR_AlgSetMaxSendResult)
	mammonClear = 6,             // 清除算法数据(STR_ClearAllAlgData,STR_ClearAllAlgDataResult)
	mammonSetDiff = 7,           // 设置主游戏难度(STR_AlgSetDiff,STR_AlgSetDiffResult)
	mammonSetBiBeiDiff = 8,      // 设置比倍难度(STR_AlgSetBiBeiDiff,STR_AlgSetBiBeiDiffResult)
	mammonBiBei = 9,             // 获取比倍结果（按倍率）(STR_AlgBiBei,STR_AlgBiBeiResult)
	mammonSetJackpotRate = 10,   // 设置彩金累积率(STR_AlgSetJackpotRate,STR_AlgSetJackpotRateResult)
	mammonAlgSetting = 11,       // 设置算法投币比例、最大负分、难度(STR_AlgSetting,STR_AlgSettingResult)
	mammonSetConfig = 12,        // 设置配置信息(STR_AlgSetConfig,STR_AlgSetConfigResult)
};

typedef struct{
	double gRealInOutCoins[2];	 //总进总出（单位：币）
	s32 diff;
	s32 max_send;
	s32 coin_rate;
}STR_AlgData;


typedef struct{
	s32 diff;
	s32 max_send;
	s32 coin_rate;
	s32 begin_table_id;
	s32 end_table_id;
}STR_AlgSetting;

typedef struct{
	s8 result;    //0-fail, 1-success
}STR_AlgSettingResult;


typedef struct{
	s64 bet; // 押分
	s32 times;   // bet被放大了多少倍
	s32 multi; // 倍率
	s32 tableId; 
}STR_AlgBiBei;

typedef struct{
	u8 result;    //0-fail, 1-success
}STR_AlgBiBeiResult;


typedef struct{
	s32 times;                 // lineBet被放大了times倍
	s64 lineBet;			   // 每条线的押分（协议字段名bet）
	s16 lineNum;               // 有押分的线数（协议字段名betLineNum）
	s32 tableId;
}STR_AlgKillFish;

typedef struct{
	s8 prizeType;			   			   // 中奖类型：0-普通游戏、1-免费游戏
	u32 totalMulti;            			   // 总倍率（等于normalMulti+freegameMultis[]）

	// 基础牌面，所有中奖类型都有
	int *normalIconsAgo;      	           // 符号替换前图标列表（一维数组）
	int *normalIcons;      	  			   // 符号替换后图标列表（一维数组）
	int normalMulti;                       // 倍率
	
	// prizeType=1
	int (*freegameIconsAgo)[BOARD_ICON_NUM];  // 符号替换前图标列表（二维数组）
	int (*freegameIcons)[BOARD_ICON_NUM];     // 符号替换后图标列表（二维数组）
	int *freegameMultis;                      // 每局的倍率（一维数组）

	//转盘
	int buffCounter;                              //金牛计数器
	int *rotaryMultis;                            //转盘倍率
	int *hitTarget;                               //转盘抽中的结果
	int (*rotaryData)[ROTARY_ICON_NUM];           //转盘数据

	s16 freegameNum;		   // 算法专用
	s16 rotaryNum;        	   // 算法专用
}STR_AlgKillFishResult;

typedef struct{
	s8 prizeType;
	u32 totalMulti;

	int normalIconsAgo[BOARD_ICON_NUM];
	int normalIcons[BOARD_ICON_NUM];
	int normalMulti;

	int freegameIconsAgo[FREEGAME_ROUND_MAX][BOARD_ICON_NUM];//FREEGAME_ROUND_MAX = 100, BOARD_ICON_NUM = 20
	int freegameIcons[FREEGAME_ROUND_MAX][BOARD_ICON_NUM];
	int freegameMultis[FREEGAME_ROUND_MAX];

	int buffCounter;
	int rotaryIcons[ROTARY_INIT_ROUND][ROTARY_ICON_NUM];
	int rotaryMultis[ROTARY_INIT_ROUND];
	int hitTarget[ROTARY_INIT_ROUND];
	int rotaryData[ROTARY_INIT_ROUND][ROTARY_ICON_NUM];

	s16 freegameNum;
	s16 rotaryNum;
}STR_AlgKillFishResultTemp;

typedef struct{
	s32 level;        // 一个码，会有指定的范围，稍后提供
	s32 begin_table_id;
	s32 end_table_id;
}STR_AlgSetMaxSend;

typedef struct{
	s32 result;
}STR_AlgSetMaxSendResult;


typedef struct{
	s64 inScore; //要修正的in值
	s64 outScore; //要修正的out值
	s32 times;    // inScore, outScore被放大了多少倍
	s32 tableId;
}STR_AlgInOutRevise;

typedef struct{
	s8 reviseResult;
}STR_AlgInOutReviseResult;


typedef struct{
	s32 tableId;
}STR_AlgInit;

typedef struct{
	u32 algVerNum; // 算法版本号
	s32 result; // 初始化结果
}STR_AlgInitResult;


typedef struct{
	s32 coin_rate; // 投币比例
	s32 begin_table_id;
	s32 end_table_id;
}STR_AlgReset;

typedef struct{
	s8 result;
}STR_AlgResetResult;


typedef struct{
	s32 tableId;
}STR_AlgTestData;

typedef struct{
	s32 coin_rate;
	s32 max_send;
	s32 real_diff;
	u64 random;

	u64 gRejustIn;
	u64 gRejustOut;
}STR_AlgTestDataResult;


typedef struct{
	s32 begin_table_id;
	s32 end_table_id;
}STR_ClearAllAlgData;

typedef struct{
	s8 result;
}STR_ClearAllAlgDataResult;


typedef struct{
	s32 diff;   // 难度
	s32 begin_table_id;
	s32 end_table_id;
}STR_AlgSetDiff;

typedef struct{
	s8 result; // 1:成功; 0:失败
}STR_AlgSetDiffResult;


typedef struct{
	s32 diff;  // 难度
	s32 tableId;
}STR_AlgSetBiBeiDiff;

typedef struct{
	s8 result; // 1:成功; 0:失败
}STR_AlgSetBiBeiDiffResult;


typedef struct{
	s32 jackpot_rate; // X10000
	s32 idx;
}STR_AlgSetJackpotRate;

typedef struct{
	s8 result;    //0-fail, 1-success
}STR_AlgSetJackpotRateResult;


typedef struct{
	char *content; //file content
}STR_AlgSetConfig;

typedef struct{
	s8 result;    //0-fail, 1-success
	char *config;  //see if the same with STR_AlgSetConfig.content
}STR_AlgSetConfigResult;

#ifdef __cplusplus
}
#endif

#endif



