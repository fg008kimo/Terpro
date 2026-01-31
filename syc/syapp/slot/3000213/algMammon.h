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
	s32 times;                 // 
	s64 lineBet;			   // 每条线的押分（协议字段名bet）
	s16 lineNum;               // 有押分的线数（协议字段名betLineNum）
	s32 tableId;
	s8 blackDiff;              // 算法难度值
	s8 blackDiffType;          // 算法难度类型
	s8 uid;
	int playerDiff;            // 用户难度	
}STR_AlgKillFish;

typedef struct{
	u32 totalMulti;            				  // 总倍率（等于normalMulti）

	// 基础牌面，所有中奖类型都有
	int *normalIcons;      	  				  // 图标列表（一维数组，长度9）
	int multiplier;							  // 倍率
	int (*normalLineInfos)[3];	  			  // 得分线（二维数组[[线id,得分图标id,倍率]]）

	s16 normalLineNum;         // 算法专用
}STR_AlgKillFishResult;

typedef struct{
	u32 totalMulti;            				  

	int normalIcons[BOARD_ICON_NUM];
	int multiplier;
	int normalLineInfos[BET_LINE_TOTAL][3];

	s16 normalLineNum;
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



