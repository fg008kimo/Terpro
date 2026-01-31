#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "board.h"
#include "../server.h"
#include "../utils.h"
#include "../data.h"

//#define PRINT_RAND    			1
//#define PRINT_CALPRIZETYPE    	1
//#define CAL_ANIME_TYPE    			1
//#define PRINT_CALPRIZE_NUM    	1
//#define PRINT_CALPRIZE_MULTI    	1
//#define PRINT_BRD_CALMULTI    	1


/////////////////////////////////【对外接口】////////////////////////////////////////

void board_gen(const uint8_t (*rollerIcon)[ROLLER_ICON_LEN], const uint16_t *rollerIconNum, int *icons, const uint32_t *prizeSymbolWeight ,int PRIZE_PROB_SUM)
//void board_gen(const uint8_t (*rollerIcon)[ROLLER_ICON_LEN], const uint16_t *rollerIconNum, int *icons, int prizeSymbolWeight ,int PRIZE_PROB_SUM, int *przSymbolDist)//calculate_prize_symbol dist
{
	int column;
	int i, j, k, index_prize, BOARD_ROW_NEW;
	//const uint32_t *prizeSymbolWeight = gBGPrizeSymbolWeight;
	//#ifdef CAL_PRIZE_NUM
	//int m;
	//#endif

	for(column = 0; column < BOARD_COLUMN; column++)
	{
		BOARD_ROW_NEW = (column % 2) != 0 ? BOARD_ROW + 1 : BOARD_ROW;//if odd column choose(BOARD_ROW + 1) else choose BOARD_ROW
		i = column != (BOARD_COLUMN - 1) ? column * BOARD_ROW : column * BOARD_ROW + 1;//The third row need 1 more element
		j = (int)GetRand(rollerIconNum[column]);
		for(k = 0; k < BOARD_ROW_NEW; k++){
			icons[i+k] = rollerIcon[column][j];
			if(icons[i+k] == E_ICON_PRIZE){ //若選到PRIZE SYMBOL,會依照權重隨機變成不同的PRIZE SYMBOL
				index_prize = getRandomValue(prizeSymbolWeight, PRIZE_SYMBOL_NUM, PRIZE_PROB_SUM);
				//index_prize = RandSelect(gPrizeSymbolWeight, PRIZE_SYMBOL_NUM, PRIZE_PROB_SUM_BG);
				
				//#ifdef CAL_PRIZE_NUM//calculate_prize_symbol num  //need to be remarked if not use
				//for(m = 0; m < PRIZE_SYMBOL_NUM; m++){
				//	if(index_prize == m){
				//		przSymbolDist[m]++;//{8,0},{9,1},{10,2},{11,3},{12,4},{13,5},{14,6},{15,7},{16,8},{17,9}
				//		printf("m:%d, przSymbolDist[m]:%d\n", m, przSymbolDist[m]);
				//	}
				//}
				//#endif
				
				icons[i+k] += index_prize;
			}
			j++;
			if(j >= rollerIconNum[column])
				j = 0;
			#ifdef PRINT_BOARD
			printf("icons[i+k]:%d, column:%d, i:%d, k:%d, j:%d, %d\n", icons[i+k], column, i, k, j, __LINE__);
			#endif
		}
	}
}

int board_calMulti(s16 lineNum, int *icons, int (*lineInfos)[3], s16 *awardLineNum)
{
	int line, lineMulti;
	int i, targetIcon, targetWildIsSeq, targetWildSeqNum;
	//double totalMulti = 0.0;
	int totalMulti = 0;

	#ifdef PRINT_BRD_CALMULTI
	printf("--start board_calMulti()\n");
	#endif
	if(awardLineNum)
		*awardLineNum = 0;
	if(lineNum > 0){
		for(line = 0; line < lineNum; line++)
		{
			targetWildIsSeq = 1;
			targetWildSeqNum = 0;
		
			targetIcon = icons[gLinePos[line][0]];
			#ifdef PRINT_BRD_CALMULTI
			printf("line:%d, gLinePos[line][0]:%d, targetIcon:%d, %d, %s\n", line, gLinePos[line][0], targetIcon, __LINE__, __FUNCTION__);
			#endif
			if(targetIcon == E_ICON_WILD){
				targetWildSeqNum++;
				#ifdef PRINT_BRD_CALMULTI
				printf("targetWildSeqNum:%d, %d, %s\n", targetWildSeqNum, __LINE__, __FUNCTION__);
				#endif
			}else if(targetIcon >= E_ICON_PRIZE){
				#ifdef PRINT_BRD_CALMULTI
				printf("E_ICON_PRIZE case, %d, %s\n", __LINE__, __FUNCTION__);
				#endif
				continue;
			}else{
				targetWildIsSeq = 0;
				#ifdef PRINT_BRD_CALMULTI
				printf("targetWildIsSeq:%d, %d, %s\n", targetWildIsSeq, __LINE__, __FUNCTION__);
				#endif
			}
			for(i = 1; i < BOARD_COLUMN; i++)
			{
				#ifdef PRINT_BRD_CALMULTI
				printf("targetIcon:%d, icons[gLinePos[line][i]]:%d, line:%d, i:%d, gLinePos[line][i]:%d, targetWildSeqNum:%d, targetWildIsSeq:%d, %d, %s\n", targetIcon, icons[gLinePos[line][i]], line, i, gLinePos[line][i], targetWildSeqNum, targetWildIsSeq, __LINE__, __FUNCTION__);
				#endif
				if(icons[gLinePos[line][i]] == E_ICON_WILD){
					if(targetWildIsSeq){
						targetWildSeqNum++;
						#ifdef PRINT_BRD_CALMULTI
						printf("targetWildSeqNum:%d, targetWildIsSeq:%d, line:%d, i:%d, gLinePos[line][i]:%d, %d, %s\n", targetWildSeqNum, targetWildIsSeq, line, i, gLinePos[line][i], __LINE__, __FUNCTION__);
						#endif
					}
				}else if(icons[gLinePos[line][i]] >= E_ICON_PRIZE){
					#ifdef PRINT_BRD_CALMULTI
					printf("icons[gLinePos[line][i]]:%d, line:%d, i:%d, gLinePos[line][i]:%d, %d, %s\n", icons[gLinePos[line][i]], line, i, gLinePos[line][i], __LINE__, __FUNCTION__);
					#endif
					break;
				}else{
					targetWildIsSeq = 0;
					if(targetIcon == E_ICON_WILD){
						targetIcon = icons[gLinePos[line][i]];
						#ifdef PRINT_BRD_CALMULTI
						printf("targetIcon is E_ICON_WILD in column 0, targetIcon:%d, line:%d, i:%d, gLinePos[line][i]:%d, %d, %s\n", targetIcon, line, i, gLinePos[line][i], __LINE__, __FUNCTION__);
						#endif
					}
					if(targetIcon != icons[gLinePos[line][i]]){
						#ifdef PRINT_BRD_CALMULTI
						printf("targetIcon:%d, icons[gLinePos[line][i]]:%d, line:%d, i:%d, gLinePos[line][i]:%d, %d, %s\n", targetIcon, icons[gLinePos[line][i]], line, i, gLinePos[line][i], __LINE__, __FUNCTION__);
						#endif
						break;
					}
				}
			}
		
			//比较wild图标倍率
			if(targetWildSeqNum > 0 && gIconMulti[E_ICON_WILD][targetWildSeqNum] > gIconMulti[targetIcon][i])
			{
				targetIcon = E_ICON_WILD;
				i = targetWildSeqNum;
				#ifdef PRINT_BRD_CALMULTI
				printf("targetIcon:%d, i:%d, gIconMulti[E_ICON_WILD][targetWildSeqNum]:%d, gIconMulti[targetIcon][i]:%d, %d, %s\n", targetIcon, i, gIconMulti[E_ICON_WILD][targetWildSeqNum], gIconMulti[targetIcon][i], __LINE__, __FUNCTION__);
				#endif
			}
		
			lineMulti = gIconMulti[targetIcon][i];
			//totalMulti += (double)lineMulti;
			totalMulti += lineMulti;
			#ifdef DEBUG_PAYOUT_DIST //這邊不要改成 PRINT_BRD_CALMULTI,因為想看算分
			printf("targetIcon:%d, i:%d, totalMulti:%d, lineMulti:%d, %d, %s\n", targetIcon, i, totalMulti, lineMulti, __LINE__, __FUNCTION__);
			#endif
		
			if(lineMulti > 0 && lineInfos && awardLineNum)
			{
				//lineInfos[*awardLineNum][0] = (double)(line + 1);
				//lineInfos[*awardLineNum][1] = (double)targetIcon;
				//lineInfos[*awardLineNum][2] = (double)lineMulti;
				lineInfos[*awardLineNum][0] = line + 1;
				lineInfos[*awardLineNum][1] = targetIcon;
				lineInfos[*awardLineNum][2] = lineMulti;
				(*awardLineNum)++;
				#ifdef DEBUG_PAYOUT_DIST
				printf("*awardLineNum:%d, lineInfos[*awardLineNum][0]:%d, lineInfos[*awardLineNum][1]:%d, lineInfos[*awardLineNum][2]:%d, line:%d, %s\n", *awardLineNum, line + 1, targetIcon, lineMulti, __LINE__, __FUNCTION__);
				#endif
			}
		}
	}
	#ifdef PRINT_BRD_CALMULTI
	printf("start to calculate the prize in board_calMulti()\n");
	#endif
	if(board_calPrizeNum(icons, E_ICON_PRIZE) >= PRIZE_SYMBOL_THRESHOLD){
		int PrizeMulti;
		PrizeMulti = board_calPrizeMulti(icons);
		totalMulti += PrizeMulti;
		#ifdef DEBUG_PAYOUT_DIST
		printf("PrizeMulti:%d, totalMulti:%d, %d, %s\n", PrizeMulti, totalMulti, __LINE__, __FUNCTION__);
		#endif
		if(PrizeMulti > 0 && lineInfos && awardLineNum)//若是free game, lineInfos放在 gResult.freegameLineInfos[round]
		{
			//lineInfos[*awardLineNum][0] = (double)lineNum;
			//lineInfos[*awardLineNum][1] = (double)E_ICON_PRIZE;
			//lineInfos[*awardLineNum][2] = (double)PrizeMulti;
			lineInfos[*awardLineNum][0] = lineNum;
			lineInfos[*awardLineNum][1] = E_ICON_PRIZE;
			lineInfos[*awardLineNum][2] = PrizeMulti;
			(*awardLineNum)++;
			#ifdef DEBUG_PAYOUT_DIST
			printf("*awardLineNum:%d, lineInfos[*awardLineNum][0]:%d, lineInfos[*awardLineNum][1]:%d, lineInfos[*awardLineNum][2]:%d, line:%d, %s\n", *awardLineNum, lineNum, E_ICON_PRIZE, PrizeMulti, __LINE__, __FUNCTION__);
			//直接放 lineInfos[*awardLineNum][0~2] 會print到壞掉 查不出原因 => print的msg太多的話printer的memory會掛掉
			#endif
		}
	}
	
	#ifdef PRINT_BRD_CALMULTI
	printf("--end of board_calMulti()\n");
	#endif
	return totalMulti;
}

int board_calPrizeType(int FREEGAME_THRESHOLD)
{
	//int index;
	uint32_t index;
	uint32_t RAND_NUM = (1000000000);
	//unsigned long long int RAND_NUM = (100000000000000000);
	index =  GetRand(RAND_NUM) + 1;   				// 	 909913450423647           9099134
													//100000000000000000        1000000000
	
	//return E_PRIZETYPE_FREEGAME; //test only free game 
	//return E_PRIZETYPE_NORMAL; //test only base game 
	
	if(index <= FREEGAME_THRESHOLD){
		#ifdef PRINT_CALPRIZETYPE
		printf("index:%d, %s\n", index, __FUNCTION__);
		#endif
		return E_PRIZETYPE_FREEGAME;
	}else
		return E_PRIZETYPE_NORMAL;
}

int calAnimeType(int ANIME_THRESHOLD)
{
	uint32_t index;
	uint32_t RAND_NUM = (1000000);
	index =  GetRand(RAND_NUM) + 1;
	
	if(index <= ANIME_THRESHOLD){
		#ifdef CAL_ANIME_TYPE
		printf("index:%d, %s\n", index, __FUNCTION__);
		#endif
		return E_PRIZETYPE_FREEGAME;
	}else
		return E_PRIZETYPE_NORMAL;
}

//int board_calIconNum(int *icons, int targetIcon)
//{
//	int i, num = 0;
//	for(i = 0; i < BOARD_ICON_NUM; i++)
//		if(icons[i] == targetIcon)
//			num++;
//	return num;
//}


int board_calPrizeNum(int *icons, int targetIcon)
{
	int i, num = 0;
	for(i = 0; i < BOARD_ICON_NUM; i++){
		if(icons[i] >= targetIcon){
			num++;
			#ifdef PRINT_CALPRIZE_NUM
			printf("num:%d, %s\n", num, __FUNCTION__);
			#endif
		}
	}
	return num;
}

int board_calPrizeMulti(int *icons)
{
	int i;
	int prizeMulti = 0;
	for(i = 0; i < BOARD_ICON_NUM; i++){//board prize multi summation
		if(icons[i] >= E_ICON_PRIZE){
			prizeMulti += gPrizeSymbol[icons[i] - E_ICON_PRIZE][1];
			//prizeMulti += (double)gPrizeSymbol[icons[i] - E_ICON_PRIZE][1];
			#ifdef PRINT_CALPRIZE_MULTI
			printf("i:%d, icons[i]:%d, gPrizeSymbol[icons[i] - E_ICON_PRIZE][1]:%d, prizeMulti:%d, %d, %s\n", i, icons[i], gPrizeSymbol[icons[i] - E_ICON_PRIZE][1], prizeMulti, __LINE__, __FUNCTION__);
			#endif
		}
	}
	return prizeMulti;
}

int getRandomValue(const uint32_t* weight, int len, int weightValue)
{																	
    int i;
    int ran = GetRand(weightValue) + 1;//若把+1拿掉, ran有可能出現0的情況,下面for loop會return權重矩陣的極值(最大or最小),所以會造成幾百次以上實驗會有單一峰值
    for(i = len-1; i >= 0; i--)
    {
		#ifdef PRINT_RAND
		printf("ran:%d, i:%d, weight[i]:%d, %s\n", ran, i, weight[i], __FUNCTION__);
		#endif
		if(ran <= weight[i]){
			#ifdef PRINT_RAND
			printf("in if ran:%d, i:%d, %s\n", ran, i, __FUNCTION__);
			#endif
			return i;
		}	
        ran -= weight[i];
    }
    return 0;
}

int selectWeight(s8 uid)
{
	int selectVersion = uid % 2 != 0 ? 0 : 8;   //uid奇数对应v0版本，偶数对应v8版本
	int isSelect_A = GetRand(VERSION_WEIGHT) < gSelectVersion[selectVersion][0];
	return isSelect_A;
} 

void difficulty_control(s16 lineNum, int diff, int upMulti, int downMulti, const double (*targetRTP)[3], int isSelect_A, s32 totalMulti, int *loopTime)
{
	double ranPro = GetRandom();
	if(diff < NORMAL_DIFF)
	{
		if(totalMulti <= downMulti*lineNum && ranPro < (double)targetRTP[diff][2-isSelect_A]/100)   //上限位的rtp
			(*loopTime)--;
		else
			(*loopTime) = 0;
	}
	else
	{
		if(totalMulti > upMulti*lineNum && ranPro < (double)targetRTP[diff][2-isSelect_A]/100)   //下限位的rtp
			(*loopTime)--;
		else
			(*loopTime) = 0;
	}
}
