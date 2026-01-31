#ifndef _FREEGAME_H_
#define _FREEGAME_H_

#include "../utils.h"
#include "../data.h"


// 生成整局免费游戏
void freeGame_gen(s8 isBuyFreegame, int (*freegameIcons)[MAX_CASCADE_NUM][BOARD_ICON_NUM], int (*freegameMultis)[MAX_CASCADE_NUM],
    int (*freegameRewardInfos)[MAX_CASCADE_NUM][MAX_REWARD_ICON_TYPE][3], int (*freegameMulIconMultis)[BOARD_ICON_NUM], 
    s16 *freegameNum, s16 *freegameCascadeNums, s16 (*freegameRewardIconTypes)[MAX_CASCADE_NUM], s16 *freegameMulIconNums);

#endif
