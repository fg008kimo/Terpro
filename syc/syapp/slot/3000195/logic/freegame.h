#ifndef _FREEGAME_H_
#define _FREEGAME_H_

#include "../utils.h"
#include "../data.h"


// 计算免费游戏局数
int freeGame_calNum(int *normalIcons);

// 生成整局免费游戏
void freeGame_gen(s8 isBuyFreegame, s16 freegameNum, int (*preIcons)[BOARD_ICON_NUM], int (*icons)[BOARD_ICON_NUM], int (*changeWildMode)[HIGH_ICON_NUM]);

#endif
