#ifndef UTILS_H
#define UTILS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdio.h>

#include "server.h"
#include "tools/cJSON.h"

typedef signed char		s8;		//[-128,127]
typedef signed short	s16;    //[-32768,32767]
typedef signed int		s32;    //[-2147483648,2147483647]用途：默认使用
typedef signed long		s64;	//[非常大]用途：进出分
typedef unsigned char	u8;		//[0,255]
typedef unsigned short	u16;	//[0,65535]
typedef unsigned int	u32;	//[0,4294967295]
typedef unsigned long	u64;	//[非常大]

#define MIN_CLIENT_ID    100000000
#define RANDOM_ENC_KEY_LEN    16
#define IS_EMPTY(str)  ((str == NULL) || (strlen(str) == 0)) ? 1 : 0

enum alg_client_rpc_method {
	rpc_login = 0,
	rpc_auth = 1,
	rpc_init = 2,
	rpc_hb = 3,
};

enum alg_response_msgtype {
	srv_rm_login = 0,
	srv_rm_auth = 1,
	srv_rm_hbpong = 3,       //heartbeat response
};

// 获取当前时间
char *get_current_time(char *time_now);

// 申请动态内存
void *alg_malloc(size_t size);
// 申请动态内存
void *alg_calloc(size_t nmemb, size_t size);
// 销毁动态内存
void alg_free(void *mem);

// 随机数模块初始化
void InitRandom(int algPort);
// 返回[0,1)随机浮点数
double GetRandom();
// 返回[0,maxValue)随机整数
uint32_t GetRand(uint32_t maxValue);
// 返回[minValue,maxValue)随机整数
uint32_t GetRand_2(uint32_t minValue,uint32_t maxValue);

// 随机选择probArray[]，返回数组下标
int RandSelect(const uint32_t* probArray, uint16_t probNum, uint32_t probSum);

// 生成随机字符串
void generate_random_str(char *str, int length);

// 打印cJSON对象
void print_cjson(cJSON* dataJson);

/// <summary>
/// 把字符串转换为整型数组
/// </summary>
/// <param name="str">传入的字符串（例如[10,20,30]）</param>
/// <param name="intArr">返回的整型数组</param>
/// <param name="count">返回的数字个数</param>
void parse_str_2_intArr(char* str, int* intArr, int* count);

// 拷贝字符串
void cpy_char_arr(char* target_arr, char* source_arr, int count);
// 拷贝整型数组
void cpy_int_arr(int* target_arr, int* source_arr, int count);

// 四个字符转换为整型
int parse_charArr_2_int(char* c_arr);
// 整型转换为四个字符
void parse_int_2_charArr(int num, char* c_arr);

// 把浮点数转换为整数（浮点数精度问题。例子：浮点数14可能存储为13.999999强转换为13，但我们需要14）
u64 convert_double_2_ulong(double fValue);

// 交换数值
void swap(int* pNum1, int* pNum2);

// 洗牌
void shuffle(int tableId, int* arr, int len);

#ifdef __cplusplus
}
#endif

#endif

