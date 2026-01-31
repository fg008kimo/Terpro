#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "board.h"
#include "../server.h"
#include "../utils.h"
#include "../data.h"

//#define PRINT_BRD_CALMULTI    	1
//#define PRINT_RAND    			1
//#define ICONS_VS_SYMBOL    		1
//#define MULTIPLIER    			1

/////////////////////////////////【对外接口】////////////////////////////////////////

void board_gen(const uint8_t (*rollerIcon)[ROLLER_ICON_LEN], const uint16_t *rollerIconNum, int *icons)
{
	int column;
	int i, j, k;

	for(column = 0; column < BOARD_COLUMN; column++)
	{
		i = column * BOARD_ROW;
		j = (int)GetRand(rollerIconNum[column]);
		//j = 0; //for testing
		for(k = 0; k < BOARD_ROW; k++)
		{
			icons[i+k] = rollerIcon[column][j];
			j++; 
			if(j >= rollerIconNum[column])
				j = 0;
			#ifdef PRINT_BOARD
			printf("icons[i+k]:%d, column:%d, i:%d, k:%d, j:%d, line:%d in board_gen()\n", icons[i+k], column, i, k, j, __LINE__);
			#endif
		}
	}
}

void board_traverse_gen(const uint8_t (*rollerIcon)[ROLLER_ICON_LEN], const uint16_t *rollerIconNum, int *icons, int *mul)
{
	int column;
	int i, j, k;
	static int cols[BOARD_COLUMN] = {0};

	for(column = 0; column < BOARD_COLUMN; column++)
	{
		i = column * BOARD_ROW;
		j = cols[column];
		for(k = 0; k < BOARD_ROW; k++)
		{
			icons[i+k] = rollerIcon[column][j];
			j++;
			if(j >= rollerIconNum[column])
				j = 0;
			#ifdef PRINT_BOARD
			printf("icons[i+k]:%d, column:%d, i:%d, k:%d, j:%d, line:%d in board_traverse_gen()\n", icons[i+k], column, i, k, j, __LINE__);
			#endif	
		}
	}
	
	for(i = 0; i < BOARD_COLUMN; i++){
		mul[i] = 2;
		#ifdef PRINT_BOARD
		printf("i:%d, mul[i]:%d in IconsToVSsymbol()\n", i, mul[i], __LINE__);
		#endif
	}

	//遍历
	for(column = BOARD_COLUMN - 1; column >= 0; column--)
	{
		cols[column]++;
		#ifdef PRINT_BOARD
		printf("cols[column]:%d, line:%d in board_traverse_gen()\n", cols[column], __LINE__);
		#endif			
		if(cols[column] >= rollerIconNum[column])
			cols[column] = 0;
		else
			break;
	}
}

int board_calMulti(s16 lineNum, int *icons, int (*lineInfos)[4], s16 *awardLineNum, int *mul)
{
	int line, lineMulti, lineMultiMul, totalMulti = 0;
	int i, j, targetIcon, targetWildIsSeq, targetWildSeqNum, multiplier;
	 
	#ifdef PRINT_BOARD
	printf("PRINT_BOARD start, line:%d in board_calMulti()\n", __LINE__);
    for (i = 0; i < BOARD_ICON_NUM; i++)//print final board
		printf("i:%d, icons[i]:%d, line:%d in board_calMulti()\n", i, icons[i], __LINE__);
	#endif

	#ifdef PRINT_BRD_CALMULTI
	printf("--start board_calMulti()\n");
	#endif
	if(awardLineNum)
		*awardLineNum = 0;
	for(line = 0; line < lineNum; line++)
	{
		multiplier = 0;
		targetWildIsSeq = 1;
		targetWildSeqNum = 0;
		if((icons[gLinePos[line][0]] == E_ICON_SC1) ||
		   (icons[gLinePos[line][0]] == E_ICON_SC2) ||
		   (icons[gLinePos[line][0]] == E_ICON_SC3)){
			#ifdef PRINT_BRD_CALMULTI
			printf("line:%d, gLinePos[line][0]:%d, icons[gLinePos[line][0]]:%d, __LINE__:%d, %s\n", line, gLinePos[line][0], icons[gLinePos[line][0]], __LINE__, __FUNCTION__);
			#endif
			continue;
		}

		targetIcon = icons[gLinePos[line][0]];
		#ifdef PRINT_BRD_CALMULTI
		printf("line:%d, gLinePos[line][0]:%d, targetIcon:%d, __LINE__:%d, %s\n", line, gLinePos[line][0], targetIcon, __LINE__, __FUNCTION__);
		#endif
		if(targetIcon == E_ICON_WILD){
			targetWildSeqNum++;
			#ifdef PRINT_BRD_CALMULTI
			printf("targetWildSeqNum:%d, __LINE__:%d, %s\n", targetWildSeqNum, __LINE__, __FUNCTION__);
			#endif
		}else if(targetIcon == E_ICON_VS){
			targetWildSeqNum++;
			multiplier += mul[0];
			#ifdef PRINT_BRD_CALMULTI
			printf("targetWildSeqNum:%d, mul[0]:%d, multiplier:%d, __LINE__:%d, %s\n", targetWildSeqNum, mul[0], multiplier, __LINE__, __FUNCTION__);
			#endif
		}else{
			targetWildIsSeq = 0;
			#ifdef PRINT_BRD_CALMULTI
			printf("targetWildIsSeq:%d, __LINE__:%d, %s\n", targetWildIsSeq, __LINE__, __FUNCTION__);
			#endif
		}
		for(i = 1; i < BOARD_COLUMN; i++)
		{
			if((icons[gLinePos[line][i]] == E_ICON_SC1) ||
			   (icons[gLinePos[line][i]] == E_ICON_SC2) ||
			   (icons[gLinePos[line][i]] == E_ICON_SC3)){
				#ifdef PRINT_BRD_CALMULTI
				printf("icons[gLinePos[line][i]]:%d, i:%d, gLinePos[line][i]:%d, __LINE__:%d, %s\n", icons[gLinePos[line][i]], i, gLinePos[line][i], __LINE__, __FUNCTION__);
				#endif
				break;
			}else if(icons[gLinePos[line][i]] == E_ICON_WILD){
				if(targetWildIsSeq){
					targetWildSeqNum++;
					#ifdef PRINT_BRD_CALMULTI
					printf("i:%d, targetWildSeqNum:%d, targetWildIsSeq:%d, gLinePos[line][i]:%d, __LINE__:%d, %s\n", i, targetWildSeqNum, targetWildIsSeq, gLinePos[line][i], __LINE__, __FUNCTION__);
					#endif
				}else{
					#ifdef PRINT_BRD_CALMULTI
					printf("i:%d, targetWildSeqNum:%d, targetWildIsSeq:%d, gLinePos[line][i]:%d, __LINE__:%d, %s\n", i, targetWildSeqNum, targetWildIsSeq, gLinePos[line][i], __LINE__, __FUNCTION__);
					#endif
				}
			}else if(icons[gLinePos[line][i]] == E_ICON_VS){
				if(targetWildIsSeq){
					targetWildSeqNum++;
					#ifdef PRINT_BRD_CALMULTI
					printf("i:%d, targetWildSeqNum:%d, targetWildIsSeq:%d, __LINE__:%d, %s\n", i, targetWildSeqNum, targetWildIsSeq, __LINE__, __FUNCTION__);
					#endif
				}
				multiplier += mul[i];
				#ifdef PRINT_BRD_CALMULTI
				printf("targetWildSeqNum:%d, targetWildIsSeq:%d, mul[%d]:%d, multiplier:%d, gLinePos[line][i]:%d, __LINE__:%d, %s\n", targetWildSeqNum, targetWildIsSeq, i, mul[i], multiplier, gLinePos[line][i], __LINE__, __FUNCTION__);
				#endif
			}else{
				targetWildIsSeq = 0;
				if((targetIcon == E_ICON_WILD) || (targetIcon == E_ICON_VS)){ 
					targetIcon = icons[gLinePos[line][i]];
					#ifdef PRINT_BRD_CALMULTI
					printf("i:%d, targetIcon:%d, gLinePos[line][i]:%d, line:%d, %s if E_ICON_WILD || E_ICON_VS \n", i, targetIcon, gLinePos[line][i], __LINE__, __FUNCTION__);
					#endif
				}
				if(targetIcon != icons[gLinePos[line][i]]){
					#ifdef PRINT_BRD_CALMULTI
					printf("i:%d, targetIcon:%d, icons[gLinePos[line][i]]:%d, gLinePos[line][i]:%d, __LINE__:%d, %s\n", i, targetIcon, icons[gLinePos[line][i]], gLinePos[line][i], __LINE__, __FUNCTION__);
					#endif
					break;
				}
				#ifdef PRINT_BRD_CALMULTI
				printf("i:%d, targetIcon:%d, icons[gLinePos[line][i]]:%d, gLinePos[line][i]:%d, targetIcon_continue, __LINE__:%d, %s\n", i, targetIcon, icons[gLinePos[line][i]], gLinePos[line][i], __LINE__, __FUNCTION__);
				#endif
			}
		}

		if(targetWildSeqNum > 0 && gIconMulti[E_ICON_WILD][targetWildSeqNum] > gIconMulti[targetIcon][i]){
			targetIcon = E_ICON_WILD;
			i = targetWildSeqNum;
			#ifdef PRINT_BRD_CALMULTI
			printf("targetIcon:%d, i:%d, gIconMulti[E_ICON_WILD][targetWildSeqNum]:%d, gIconMulti[targetIcon][i]:%d, __LINE__:%d, %s\n", targetIcon, i, gIconMulti[E_ICON_WILD][targetWildSeqNum], gIconMulti[targetIcon][i], __LINE__, __FUNCTION__);
			#endif
		}
		lineMulti = gIconMulti[targetIcon][i];
		#ifdef PRINT_BRD_CALMULTI 
		printf("targetIcon:%d, i:%d, lineMulti:%d, __LINE__:%d, %s\n", targetIcon, i, lineMulti, __LINE__, __FUNCTION__);
		#endif
		if(multiplier != 0){
			totalMulti += lineMulti * multiplier;
			#ifdef PRINT_BRD_CALMULTI 
			printf("totalMulti:%d, lineMulti:%d, multiplier:%d, __LINE__:%d, %s\n", totalMulti, lineMulti, multiplier, __LINE__, __FUNCTION__);
			#endif
		}else{
			totalMulti += lineMulti;
			#ifdef PRINT_BRD_CALMULTI 
			printf("totalMulti:%d, lineMulti:%d, __LINE__:%d, %s\n", totalMulti, lineMulti, __LINE__, __FUNCTION__);
			#endif
		}

		if(lineMulti > 0 && lineInfos && awardLineNum){
			lineInfos[*awardLineNum][0] = line + 1;
			lineInfos[*awardLineNum][1] = targetIcon;
			lineInfos[*awardLineNum][2] = lineMulti;
			lineInfos[*awardLineNum][3] = multiplier;
			(*awardLineNum)++;
			#ifdef PRINT_BRD_CALMULTI 
			printf("*awardLineNum:%d, lineInfos[*awardLineNum][0]:%d, lineInfos[*awardLineNum][1]:%d, lineInfos[*awardLineNum][2]:%d, lineInfos[*awardLineNum][3]:%d, __LINE__:%d, %s\n", *awardLineNum, line+1, targetIcon, lineMulti, multiplier, __LINE__, __FUNCTION__);
			#endif
		}
	}
	return totalMulti;
}

int board_calPrizeType(int *icons)
{
	if(board_calIconNum(icons, E_ICON_SC2) >= gFreeGameInfo[0][0]) 
		return E_PRIZETYPE_FREEGAME;
	if(board_calIconNum(icons, E_ICON_SC1) >= gFreeGameInfo[0][0]) 
		return E_PRIZETYPE_STICKY_WILD;
	//else if(board_calIconNum(icons, E_ICON_SC3) >= gFreeGameInfo[0][0]) 
	//	return E_PRIZETYPE_BOBUSGAME;
	else
		return E_PRIZETYPE_NORMAL;
	
	//return E_PRIZETYPE_FREEGAME; //test freegame only
	//return E_PRIZETYPE_STICKY_WILD; //test sticky game only
}

int board_calIconNum(int *icons, int targetIcon)
{
	int i, num = 0;
	for(i = 0; i < BOARD_ICON_NUM; i++)
		if(icons[i] == targetIcon)
			num++;
	return num;
}

void IconsToVSsymbol(int *iconsBefore, int *icons)
{
    int i;
    for (i = 0; i < BOARD_ICON_NUM; i++){
        if(iconsBefore[i] == E_ICON_VS){
			int j, k;
			
			j = i / BOARD_COLUMN;//cal the column j, all of elements should be change to VS symbol 
			for(k = 0; k < BOARD_ROW; k ++){
				icons[j * BOARD_COLUMN + k] = E_ICON_VS;//all emlements in same column substituted to E_ICON_VS
				#ifdef ICONS_VS_SYMBOL
				printf("i:%d, j:%d, icons[j * BOARD_COLUMN + k]:%d, j*BOARD_COLUMN+k:%d, line:%d in IconsToVSsymbol()\n", i, j, icons[j * BOARD_COLUMN + k], j*BOARD_COLUMN+k, __LINE__);
				#endif
			}
			i += (j + 1) * BOARD_COLUMN - i - 1;//No need to substituted to E_ICON_VS in the same column again
			#ifdef ICONS_VS_SYMBOL
			printf("i:%d, line:%d in IconsToVSsymbol()\n", i, __LINE__);
			#endif
		}
    }
}

void genMultiplier(const uint32_t (*gMulValue)[2], const int WEIGHT_SUM, int *mul)
{
	int i;
	
	for(i = 0; i < BOARD_COLUMN; i++){
		mul[i] = getRandomValue(gMulValue, MUL_NUM, WEIGHT_SUM);
		#ifdef MULTIPLIER
		printf("i:%d, mul[i]:%d in genMultiplier()\n", i, mul[i], __LINE__);
		#endif
	}
}

int getRandomValue(const uint32_t (*weight)[2], int len, int weightValue)
{																	
    int i;
    int ran = GetRand(weightValue) + 1;
    for(i = len-1; i >= 0; i--)
    {
		#ifdef PRINT_RAND
		printf("ran:%d, i:%d, weight[i][1]:%d, %s\n", ran, i, weight[i][1], __FUNCTION__);
		#endif
		if(ran <= weight[i][1]){
			#ifdef PRINT_RAND
			printf("in if ran:%d, i:%d, %s\n", ran, i, __FUNCTION__);
			#endif
			return weight[i][0];
		}	
        ran -= weight[i][1];
    }
    return 0;
}