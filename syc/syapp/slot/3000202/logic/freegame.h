#ifndef _FREEGAME_H_
#define _FREEGAME_H_

#include "../utils.h"
#include "../data.h"

//紀錄WILD位置
void RecordWild(int *icons, int *iconsWildTemp);

// 盤面WILD黏住
void StickWild(int *icons, int *iconsWildTemp);

// 计算免费游戏局数
int freeGame_calNum(int *normalIcons);

// 生成(十)局免费游戏(VS)
//void freeGame_gen(s8 isBuyFreegame, s16 freegameNum, int (*iconsBefore)[BOARD_ICON_NUM], int (*icons)[BOARD_ICON_NUM], int (*multiplier)[BOARD_COLUMN]);//No need to record iconsBefore
void freeGame_gen(s8 isBuyFreegame, s16 freegameNum, int (*icons)[BOARD_ICON_NUM], int (*multiplier)[BOARD_COLUMN]);

// 生成(十)局免费游戏(Sticky)
void freeGame_genSticky(s8 isBuyFreegame, s16 freegameNum, int (*icons)[BOARD_ICON_NUM]);

#endif
