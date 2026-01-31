#ifndef _BOARD_H_
#define _BOARD_H_

#include "../utils.h"
#include "../data.h"

// 生成盘面
void board_gen(const uint8_t (*rollerIcon)[ROLLER_ICON_LEN], const uint16_t *rollerIconNum, int *icons);
// 生成盘面（遍历方式）[测试用]
void board_traverse_gen(const uint8_t (*rollerIcon)[ROLLER_ICON_LEN], const uint16_t *rollerIconNum, int *icons);

// 判断是否高分图标
int board_isHighIcon(int icon);

// 计算盘面倍率，并填充得分线信息
int board_calMulti(s16 lineNum, int *icons, int (*lineInfos)[3], s16 *awardLineNum);

// 判断盘面类型
int board_calPrizeType(int *icons);

// 统计盘面中某图标个数
int board_calIconNum(int *icons, int targetIcon);

#endif
