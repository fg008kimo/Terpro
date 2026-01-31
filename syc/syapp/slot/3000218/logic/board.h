#ifndef _BOARD_H_
#define _BOARD_H_

#include "../utils.h"
#include "../data.h"

// 生成一局的所有信息（包括：若干盘面）
void board_gen(const uint8_t (*mainRollerIcon)[ROLLER_ICON_LEN], const uint16_t *mainRollerIconNum, 
    const uint8_t (*secRollerIcon)[ROLLER_ICON_LEN], const uint16_t *secRollerIconNum, const uint32_t *mulIconProb, 
    int (*icons)[BOARD_ICON_NUM], int *multis, int (*rewardInfos)[MAX_REWARD_ICON_TYPE][3], int *mulIconMultis, 
    s16 *cascadeNum, s16 *rewardIconTypes, s16 *mulIconNum);

// 判断盘面类型
int board_calPrizeType(int *icons);

// 统计盘面中某图标个数
int board_calIconNum(int *icons, int targetIcon);

#endif
