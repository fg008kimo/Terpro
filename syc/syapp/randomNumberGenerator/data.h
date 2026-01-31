/*
除了个别要做数组下标的宏定义放data.h，其它配置统一放data.c，
*/

#ifndef _ALG_DATA_H_
#define _ALG_DATA_H_

#include <stdint.h>

#define __LINUX__  //关掉__LINUX__：可跨平台编译、可跑主逻辑，但无法运行若干依赖linux模块例如网络&zlog&platform
// #define __DEBUG__
#ifndef __DEBUG__
    #define __ZLOG__
#endif

extern const int ALG_VERSION;//版本号

#endif