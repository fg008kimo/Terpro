#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "freegame.h"
#include "board.h"
#include "../server.h"
#include "../Alg2.h"
#include "../data.h"

//#define CAL_PRIZE_NUM_FG    			1
//#define CAL_PRIZE_NUM_DIST_FG    		1
//#define REPEAT_POS    				1
//#define FG_BG_CODE_FLOW	    		1
//#define FREEGAME_NUM	    			1

/////////////////////////////////【对外接口】////////////////////////////////////////

/*
if free game changes to using reel, this function didn't need anymore, but need to be keep anyway
*/
//void freeGame_board_gen(int *icons)
////void freeGame_board_gen(int *icons, int *przFgSymbolDist) //calculate_prize_symbol dist
//{
//	int i, k, j, prize_num, index_prize, prizePos[BOARD_ICON_NUM];
//	//#ifdef CAL_PRIZE_NUM
//	//int n;
//	//#endif
//	
//	const uint32_t *prizeNumWeight = gPrizeNumWeight;
//	const uint32_t *prizeSymbolWeight = gFGPrizeSymbolWeight;
//	//prize_num = getRandomValue(prizeNumWeight, PRIZE_SYMBOL_NUM-1, PRIZE_NUM_PROB_SUM) + 2;
//	prize_num = getRandomValue(prizeNumWeight, PRIZE_SYMBOL_NUM, PRIZE_NUM_PROB_SUM) + 1;//1~10
//	//prize_num = 10; //test pure case
//	//#ifdef CAL_PRIZE_NUM_DIST_FG
//	//printf("prize_num:%d, line:%d in freeGame_board_gen\n", prize_num, __LINE__);
//	//#endif
//	
//	for(i = 0; i < BOARD_ICON_NUM; i++)//generate null icon board
//		icons[i] = E_ICON_NULL;
//	for(k = 0; k < prize_num; k++){//generate board containing prize symbol
//		#ifdef FG_BG_CODE_FLOW
//		printf("stop 2 in freeGame_board_gen\n");//test_flow
//		#endif
//		prizePos[k] = (int)GetRand(BOARD_ICON_NUM);
//		for(j = 0; j < k; j++){//prevent the prize position is same with each other
//			while(prizePos[j] == prizePos[k]){
//				#ifdef REPEAT_POS
//				printf("prizePos[j]:%d, prizePos repeated\n", prizePos[j]);//test_flow
//				#endif
//				prizePos[k] = (int)GetRand(BOARD_ICON_NUM);
//				j = 0;
//			}
//		}
//		#ifdef CAL_PRIZE_NUM_FG
//		printf("k:%d, prizePos[k]:%d, line:%d in freeGame_board_gen\n", k, prizePos[k], __LINE__);
//		#endif
//		index_prize = getRandomValue(prizeSymbolWeight, PRIZE_SYMBOL_NUM, PRIZE_PROB_SUM_FG);
//		//index_prize = RandSelect(gPrizeSymbolWeight, PRIZE_SYMBOL_NUM, PRIZE_PROB_SUM_FG);
//		//index_prize = 9;//0~9  //test pure case
//		#ifdef CAL_PRIZE_NUM_FG
//		printf("index_prize:%d, line:%d in freeGame_board_gen\n", index_prize, __LINE__);
//		#endif
//		#ifdef FG_BG_CODE_FLOW
//		printf("stop 4 in freeGame_board_gen\n");//test_flow
//		#endif
//		
//		//#ifdef CAL_PRIZE_NUM   //need to be remarked if not use
//		//for(n = 0; n <= PRIZE_SYMBOL_NUM; n++){
//		//	if(index_prize == n){
//		//		przFgSymbolDist[n]++;//{8,0},{9,1},{10,2},{11,3},{12,4},{13,5},{14,6},{15,7},{16,8},{17,9}
//		//		printf("n:%d, przFgSymbolDist[n]:%d in freeGame_board_gen\n", n, przFgSymbolDist[n]);
//		//	}
//		//}
//		//#endif
//		
//		if(icons[prizePos[k]] == E_ICON_NULL)
//			icons[prizePos[k]] = E_ICON_PRIZE + index_prize;//8~17
//		else{
//			#ifdef FG_BG_CODE_FLOW
//			printf("the icon is not null\n");
//			#endif
//			continue;
//		}
//		#ifdef FG_BG_CODE_FLOW
//		printf("stop 5 in freeGame_board_gen, line:%d in freeGame_board_gen\n");//test_flow
//		#endif
//	}
//	#ifdef PRINT_BOARD
//	for(i = 0; i < BOARD_ICON_NUM; i++)//print board
//		printf("i:%d, icons[i]:%d, line:%d in freeGame_board_gen\n", i, icons[i], __LINE__);
//	#endif
//}

void freeGame_gen(s16 freegameNum, int (*icons)[BOARD_ICON_NUM], const uint32_t *prizeSymbolWeight ,int PRIZE_PROB_SUM)//(*icons) = freegameIcons[FREE_GAME_NUM]
//void freeGame_gen(s16 freegameNum, int (*icons)[BOARD_ICON_NUM], int (*przFgSymbolDist)[PRIZE_SYMBOL_NUM])  //calculate_prize_symbol dist
//void freeGame_gen(s16 freegameNum, int (*icons)[BOARD_ICON_NUM], int (*przFgSymbolDist)[PRIZE_SYMBOL_NUM], int *przFgNumDist)  //calculate_prize_symbol dist and calculate_prize_num
//void freeGame_gen(s16 freegameNum, int (*icons)[BOARD_ICON_NUM], int *przFgNumDist)  //calculate_prize_num
{
	int i;
	//#ifdef CAL_PRIZE_NUM_DIST_FG
	//int q, prize_num;
	//#endif
	
	for(i = 0; i < freegameNum; i++){
		#ifdef PRINT_BOARD
		printf("===========freegameNum:%d \n",  i, __LINE__);
		#endif
		board_gen(gFreeGameRollerIcon, gFreeGameRollerIconNum, icons[i], prizeSymbolWeight ,PRIZE_PROB_SUM);//ready to use
		//freeGame_board_gen(icons[i]);
		//freeGame_board_gen(icons[i], przFgSymbolDist[i]);  //calculate_prize_symbol dist
		//freeGame_board_gen(icons[i], przFgSymbolDist[i], przFgNumDist[i]);  //calculate_prize_symbol dist and calculate_prize_num 
		
		//#ifdef CAL_PRIZE_NUM_DIST_FG
		//const uint32_t *prizeNumWeight = gPrizeNumWeight;
		//prize_num = getRandomValue(prizeNumWeight, PRIZE_SYMBOL_NUM-1, FREEGAME_PROB_SUM) + 2;
		//printf("prize_num:%d, line:%d in freeGame_gen\n", prize_num, __LINE__);
		//for(q = 0; q <= PRIZE_SYMBOL_NUM; q++){
		//	if(prize_num == q){
		//		przFgNumDist[q]++;//{2 3 4 5 6 7 8 9 10}(要-1才能對到array index)
		//		printf("i:%d, q:%d, prize_num:%d, przFgNumDist[q]:%d, line:%d in freeGame_gen\n", i, q, prize_num, przFgNumDist[q], __LINE__);
		//	}
		//}
		//#endif
	}
}

