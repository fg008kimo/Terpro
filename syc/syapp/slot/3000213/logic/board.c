#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "board.h"
#include "../server.h"
#include "../utils.h"

//#define PRINT_RAND    			1
//#define PRINT_BRD_CALMULTI    	1

/////////////////////////////////【对外接口】////////////////////////////////////////

int board_gen(const uint8_t (*rollerIcon)[ROLLER_ICON_LEN], const uint16_t *rollerIconNum, const uint16_t (*gMulSymbolWeight)[2], int *icons)
{
	int column;
	int i, j, k;
	int multiplier;

	for(column = 0; column < BOARD_COLUMN; column++)
	{
		i = column * BOARD_ROW;
		j = (int)GetRand(rollerIconNum[column]); 
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
	
	multiplier = getRandomValue(gMulSymbolWeight, MUL_NUM, MUL_WEIGHT);
	#ifdef PRINT_BOARD
	printf("multiplier:%d, line:%d in board_gen()\n", multiplier, __LINE__);
	#endif
	return multiplier;
}

int board_traverse_gen(const uint8_t (*rollerIcon)[ROLLER_ICON_LEN], const uint16_t *rollerIconNum, int *icons)//for tranversal use
{
	int column;
	int i, j, k;
	static int cols[BOARD_COLUMN] = {0};
	int multiplier;

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
		}
	}

	for(column = BOARD_COLUMN - 1; column >= 0; column--)
	{
		cols[column]++;
		if(cols[column] >= rollerIconNum[column]){
			cols[column] = 0;
		}else
			break;
	}
	
	multiplier = 1;		
	return multiplier;
}

int board_calMulti(s16 lineNum, int *icons, int (*lineInfos)[3], s16 *awardLineNum, int multiplier)
{
	int line, lineMulti, totalMulti = 0;
	int i, targetIcon, targetWildIsSeq, targetWildSeqNum;
	
	#ifdef PRINT_BRD_CALMULTI
	printf("--start board_calMulti()\n");
	#endif
	if(awardLineNum)
		*awardLineNum = 0;
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
		}else{
			targetWildIsSeq = 0;
			#ifdef PRINT_BRD_CALMULTI
			printf("targetWildIsSeq:%d, %d, %s\n", targetWildIsSeq, __LINE__, __FUNCTION__);
			#endif
		}
		for(i = 1; i < BOARD_COLUMN; i++)
		{
			if(icons[gLinePos[line][i]] == E_ICON_WILD)
			{
				if(targetWildIsSeq){
					targetWildSeqNum++;
					#ifdef PRINT_BRD_CALMULTI
					printf("targetWildSeqNum:%d, targetWildIsSeq:%d, line:%d, i:%d, gLinePos[line][i]:%d, %d, %s\n", targetWildSeqNum, targetWildIsSeq, line, i, gLinePos[line][i], __LINE__, __FUNCTION__);
					#endif
				}
			}
			else 
			{
				targetWildIsSeq = 0;
				if(targetIcon == E_ICON_WILD){
					targetIcon = icons[gLinePos[line][i]];
					#ifdef PRINT_BRD_CALMULTI
					printf(" is E_ICON_WILD, targetIcon:%d, line:%d, i:%d, gLinePos[line][i]:%d, %d, %s\n", targetIcon, line, i, gLinePos[line][i], __LINE__, __FUNCTION__);
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

		if(targetWildSeqNum > 0 && gIconMulti[E_ICON_WILD][targetWildSeqNum] > gIconMulti[targetIcon][i]) 
		{
			targetIcon = E_ICON_WILD;
			i = targetWildSeqNum;
			#ifdef PRINT_BRD_CALMULTI
			printf("targetIcon:%d, i:%d, gIconMulti[E_ICON_WILD][targetWildSeqNum]:%d, gIconMulti[targetIcon][i]:%d, %d, %s\n", targetIcon, i, gIconMulti[E_ICON_WILD][targetWildSeqNum], gIconMulti[targetIcon][i], __LINE__, __FUNCTION__);
			#endif
		}

		lineMulti = gIconMulti[targetIcon][i] * multiplier;
		totalMulti += lineMulti;

		if(lineMulti > 0 && lineInfos && awardLineNum)
		{
			lineInfos[*awardLineNum][0] = line + 1;
			lineInfos[*awardLineNum][1] = targetIcon;
			lineInfos[*awardLineNum][2] = lineMulti;
			(*awardLineNum)++;
			#ifdef PRINT_BRD_CALMULTI
			printf("*awardLineNum:%d, lineInfos[*awardLineNum][0]:%d, lineInfos[*awardLineNum][1]:%d, lineInfos[*awardLineNum][2]:%d, %d, %s\n", *awardLineNum, line+1, targetIcon, lineMulti, __LINE__, __FUNCTION__);
			#endif
		}
	}
	#ifdef PRINT_BRD_CALMULTI
	printf("--end of board_calMulti()\n");
	#endif
	return totalMulti;
}

int getRandomValue(const uint16_t (*weight)[2], int len, int weightValue)
{																	
    int i;
    int ran = GetRand(weightValue) + 1;
    for(i = len-1; i >= 0; i--)
    {
		#ifdef PRINT_RAND
		printf("ran:%d, i:%d, weight[i]:%d, %s\n", ran, i, weight[i], __FUNCTION__);
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

void difficulty_control(s16 lineNum, int diff, int upMulti, int downMulti, const double (*targetRTP)[2], int isSelect_A, s32 totalMulti, int *loopTime)
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
