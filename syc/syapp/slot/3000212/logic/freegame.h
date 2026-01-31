#ifndef _FREEGAME_H_
#define _FREEGAME_H_

#include "../utils.h"
#include "../data.h"

// 生成免费游戏盤面
//void freeGame_board_gen(int *icons);
//void freeGame_board_gen(int *icons, int *przFgSymbolDist);  //calculate_prize_symbol dist

//// 計算免费游戏不同寶箱的RTP
//int freegame_calNumMulti(int *icons);

// 生成整局免费游戏
void freeGame_gen(s16 freegameNum, int (*icons)[BOARD_ICON_NUM], const uint32_t *prizeSymbolWeight ,int PRIZE_PROB_SUM);
//void freeGame_gen(s16 freegameNum, int (*icons)[BOARD_ICON_NUM], int (*przFgSymbolDist)[PRIZE_SYMBOL_NUM]);  //calculate_prize_symbol dist
//void freeGame_gen(s16 freegameNum, int (*icons)[BOARD_ICON_NUM], int (*przFgSymbolDist)[PRIZE_SYMBOL_NUM], int *przFgSNumDist);  //calculate_prize_symbol dist and calculate_prize_num
//void freeGame_gen(s16 freegameNum, int (*icons)[BOARD_ICON_NUM], int *przFgSNumDist);  //calculate_prize_num

#endif
