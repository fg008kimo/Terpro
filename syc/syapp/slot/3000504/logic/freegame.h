#ifndef _FREEGAME_H_
#define _FREEGAME_H_

#include "../utils.h"
#include "../data.h"

//SC計算
int calScatterNum(int *icons, int targetIcon);

//SC產生
void genScatter(int *scTemp, s8 isBuyFreegame);

//紀錄WILD位置
void RecordWild(int *icons, int *iconsTemp);

// 盤面WILD黏住
void StickWild(int *icons, int *iconsTemp);

void wildGetMultiplier1(int *icons, const uint32_t (*gMulValue)[2], const int WEIGHT_SUM);

// 生成免费游戏
#ifdef FG_TEST
void freeGame_gen(s8 isBuyFreegame, int (*icons)[BOARD_ICON_NUM], int (*scArray)[BOARD_ICON_NUM], s16 *freegameNum, s16 *freegameSCcal, s16 *freegameSCNum, s16 *freegameAddNum, s16 *freegameWildNum, s16 *freegameWildFinalNum);//, s16 *freegameAddRound
//void freeGame_gen(s8 isBuyFreegame, int (*icons)[BOARD_ICON_NUM], s16 *freegameNum, s16 *freegameAddRound);
#else
void freeGame_gen(s8 isBuyFreegame, int (*icons)[BOARD_ICON_NUM], int (*scArray)[BOARD_ICON_NUM], s16 *freegameNum);
//void freeGame_gen(s8 isBuyFreegame, int (*icons)[BOARD_ICON_NUM], s16 *freegameNum);
#endif

#endif
