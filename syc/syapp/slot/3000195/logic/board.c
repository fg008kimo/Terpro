#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "board.h"
#include "../server.h"
#include "../utils.h"


/////////////////////////////////【对外接口】////////////////////////////////////////

void board_gen(const uint8_t (*rollerIcon)[ROLLER_ICON_LEN], const uint16_t *rollerIconNum, int *icons)//ROLLER_ICON_LEN=100(每個reel icon數量上限), rollerIcon是背景輪帶[{47,49,53,63,60}]的矩陣(base game), rollerIconNum是每條輪帶上的icon數量
{
	int column;
	int i, j, k;

	for(column = 0; column < BOARD_COLUMN; column++)//BOARD_COLUMN=5
	{
		i = column * BOARD_ROW;// BOARD_ROW=3
		j = (int)GetRand(rollerIconNum[column]); //g0RollerIconNum = reel上icon矩陣, column代表第幾個reel, rollerIconNum[column]是第column條reel上的icon數量, j是reel上的icon。GetRand() = return RngGetRand()
		for(k = 0; k < BOARD_ROW; k++)//BOARD_ROW = 3, so k =0,1,2
		{
			icons[i+k] = rollerIcon[column][j];//icons[0~14] = rollerIcon[0~4][j] (rollerIcon是5條所有的背景輪帶) (column=第幾個reel) (j是背景輪帶上的第幾個icon) 參考data.c的const uint8_t gRollerIcon[BOARD_COLUMN][ROLLER_ICON_LEN] 
			//icons把3x5盤面變成一維矩陣 [0,1,2,3,4,5,6,7,8,9,10,11,12,13,14]
			j++; //j為連續3正整數,因為board要呈現連續三整數在每個輪帶呈現面的column
			if(j >= rollerIconNum[column])//若j > 那條reel上的數字(每條reel數目不一), 則 j=0 (so j<={47,49,53,63,60} for base game)
				j = 0;
			printf("icons[i+k]:%d, column:%d, i:%d, k:%d, j:%d, %d\n", icons[i+k], column, i, k, j, __LINE__);
		}
	}
}

void board_traverse_gen(const uint8_t (*rollerIcon)[ROLLER_ICON_LEN], const uint16_t *rollerIconNum, int *icons)
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
		}
	}

	//遍历
	for(column = BOARD_COLUMN - 1; column >= 0; column--)
	{
		cols[column]++;
		if(cols[column] >= rollerIconNum[column])
			cols[column] = 0;
		else
			break;
	}
}

int board_isHighIcon(int icon)
{
	return icon >= E_ICON_PIPC1 && icon <= E_ICON_PIPC4;
}

int board_calMulti(s16 lineNum, int *icons, int (*lineInfos)[3], s16 *awardLineNum)//計算盤面分數, lineNum=有押分的线数(excel上的payline數目),awardLineNum=有得分的話,那條線的得分
{
	int line, lineMulti, totalMulti = 0;
	int i, targetIcon, targetWildIsSeq, targetWildSeqNum;

	printf("--start board_calMulti()\n");
	if(awardLineNum)
		*awardLineNum = 0;
	for(line = 0; line < lineNum; line++)//25條lineNum(pattern)都要計算一遍
	{
		targetWildIsSeq = 1;//flag used to check if there is wild series
		targetWildSeqNum = 0;//wild的累積個數
		if(icons[gLinePos[line][0]] == E_ICON_SCATTER){ //gLinePos 是excel上的得分pattern
			printf("line:%d, gLinePos[line][0]:%d, icons[gLinePos[line][0]]:%d, %d, %s\n", line, gLinePos[line][0], icons[gLinePos[line][0]], __LINE__, __FUNCTION__);
			continue;
		}

		targetIcon = icons[gLinePos[line][0]];//targetIcon 是存中獎線路的第一格(盤面上第一條),mapping到此生成後盤面的那個symbol,表示用此symbol來往第2~5條看能不能形成連線,這會在後面累積倍率用到
		printf("line:%d, gLinePos[line][0]:%d, targetIcon:%d, %d, %s\n", line, gLinePos[line][0], targetIcon, __LINE__, __FUNCTION__);
		if(targetIcon == E_ICON_WILD){
			targetWildSeqNum++;
			printf("targetWildSeqNum:%d, %d, %s\n", targetWildSeqNum, __LINE__, __FUNCTION__);
		}
		else{
			targetWildIsSeq = 0;
			printf("targetWildIsSeq:%d, %d, %s\n", targetWildIsSeq, __LINE__, __FUNCTION__);
		}
		for(i = 1; i < BOARD_COLUMN; i++)//BOARD_COLUMN=5 ,i=1~4,所以是計算盤面第2~5條
		{
			if(icons[gLinePos[line][i]] == E_ICON_SCATTER)//若在中獎路線上,盤面第2~5條是scatter,則沒有辦法形成中獎連線,跳出迴圈
			{
				printf("icons[gLinePos[line][i]]:%d, line:%d, i:%d, gLinePos[line][i]:%d, %d, %s\n", icons[gLinePos[line][i]], line, i, gLinePos[line][i], __LINE__, __FUNCTION__);
				break;
			}
			else if(icons[gLinePos[line][i]] == E_ICON_WILD)//若在盤面第2~5條是wild,則wild個數累加
			{
				if(targetWildIsSeq){
					targetWildSeqNum++;
					printf("targetWildSeqNum:%d, targetWildIsSeq:%d, line:%d, i:%d, gLinePos[line][i]:%d, %d, %s\n", targetWildSeqNum, targetWildIsSeq, line, i, gLinePos[line][i], __LINE__, __FUNCTION__);
				}//如果有個情況是icons[gLinePos[line][i=1~4]]才變成非WILD的targetIcon,而下一個中獎格為WILD時(icons[gLinePos[line][i+1] = E_ICON_WILD),
				 //不會顯示在此,這裡for loop不會break的話, i會持續累加直到變成最大值(BOARD_COLUMN-1)(當然也可能在中途break),然後會出現在line 132行直接歸到lineMulti去算分
			}
			else //表示盤面上2~5條,既不是SCATTER也不是WILD的情況
			{
				targetWildIsSeq = 0;//wild不是seq => wild沒有變成連線
				if(targetIcon == E_ICON_WILD){ //若中獎路線的第1個輪帶是wild就進來此condition
					targetIcon = icons[gLinePos[line][i]];//但第2~5個輪帶上個icon不是WILD了,則targetIcon會被換成那個新的symbol
					printf(" is E_ICON_WILD, targetIcon:%d, line:%d, i:%d, gLinePos[line][i]:%d, %d, %s\n", targetIcon, line, i, gLinePos[line][i], __LINE__, __FUNCTION__);
				}
				if(targetIcon != icons[gLinePos[line][i]]){
					printf("targetIcon:%d, icons[gLinePos[line][i]]:%d, line:%d, i:%d, gLinePos[line][i]:%d, %d, %s\n", targetIcon, icons[gLinePos[line][i]], line, i, gLinePos[line][i], __LINE__, __FUNCTION__);
					break;
				}
			}
		}

		//比较wild图标倍率
		if(targetWildSeqNum > 0 && gIconMulti[E_ICON_WILD][targetWildSeqNum] > gIconMulti[targetIcon][i]) //gIconMulti是PAYTABLE(連線倍率表), E_ICON_WILD=11, gIconMulti[targetIcon][i] = gIconMulti[哪個symbol][最大倍率i=4]
		{
			targetIcon = E_ICON_WILD;
			i = targetWildSeqNum;
			printf("targetIcon:%d, i:%d, gIconMulti[E_ICON_WILD][targetWildSeqNum]:%d, gIconMulti[targetIcon][i]:%d, %d, %s\n", targetIcon, i, gIconMulti[E_ICON_WILD][targetWildSeqNum], gIconMulti[targetIcon][i], __LINE__, __FUNCTION__);
		}

		lineMulti = gIconMulti[targetIcon][i];//單一icon的倍率
		totalMulti += lineMulti;//總倍率(盤面上所有得分的線路的倍率加總)
		printf("targetIcon:%d, i:%d, totalMulti:%d, lineMulti:%d, %d, %s\n", targetIcon, i, totalMulti, lineMulti, __LINE__, __FUNCTION__);

		if(lineMulti > 0 && lineInfos && awardLineNum)
		{
			lineInfos[*awardLineNum][0] = line + 1;
			lineInfos[*awardLineNum][1] = targetIcon;
			lineInfos[*awardLineNum][2] = lineMulti;
			(*awardLineNum)++;
			printf("*awardLineNum:%d, lineInfos[*awardLineNum][0]:%d, lineInfos[*awardLineNum][1]:%d, lineInfos[*awardLineNum][2]:%d, %d, %s\n", *awardLineNum, line+1, targetIcon, lineMulti, __LINE__, __FUNCTION__);
		}
	}
	printf("--end of board_calMulti()\n");
	return totalMulti;
}

int board_calPrizeType(int *icons)
{
	if(board_calIconNum(icons, E_ICON_SCATTER) >= gFreeGameInfo[0][0]) //E_ICON_SCATTER(橄欖枝)是進入free game的scatter, gFreeGameInfo[0][0] = 3,所以SC >= 3就進入free game
		return E_PRIZETYPE_FREEGAME;
	else
		return E_PRIZETYPE_NORMAL;
}

int board_calIconNum(int *icons, int targetIcon)
{
	int i, num = 0;
	for(i = 0; i < BOARD_ICON_NUM; i++)//BOARD_ICON_NUM = 15
		if(icons[i] == targetIcon)
			num++;
	return num;
}
