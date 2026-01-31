#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "board.h"
#include "../server.h"
#include "../utils.h"
#include "../data.h"

//#define PRINT_RAND    			1
//#define PRINT_BRD_CALMULTI    	1
//#define WILD_GET_MULTIPLIER    	1

/////////////////////////////////【对外接口】////////////////////////////////////////
#ifdef GET_RAND
void cal_rand(s16 *rand_num)
{
	int j;
	j = (int)GetRand(TEST_RAND_NUM);
	*rand_num = j;
	//printf("j:%d, *rand_num:%d, line:%d\n", j, *rand_num, __LINE__);
}
#endif

#ifdef SRAND
int randint(int n){
	if((n - 1) == RAND_MAX)
		return rand();
	else{
		/* 計算可以被整除的長度 */
		long end = RAND_MAX / n;
		end *= n;
	
		/* 將尾端會造成偏差的幾個亂數去除，
		若產生的亂數超過 limit，則將其捨去 */
		int r;
		while ((r = rand()) >= end);
	
		return r % n;
	}
}
void cal_rand_inbuild(s16 *rand_num1)/* C inbuild funciton test */
{
	int k, min = 0, max = TEST_RAND_NUM;	
	k = randint(max - min) + min;
	*rand_num1 = k;
	//printf("k:%d, *rand_num1:%d, line:%d\n", k, *rand_num1, __LINE__);
}
#endif

#ifdef REEL_DIST
void board_gen_dist(const uint8_t (*rollerIcon)[ROLLER_ICON_LEN], const uint16_t *rollerIconNum, int *icons, s16 (*rollerDist)[ROLLER_ICON_LEN], s16 *rollerDist1, s16 *rollerDist2, s16 *rollerDist3, s16 *rollerDist4, s16 *rollerDist5, s16 *rollerDist6, s16 *rollerDist7, s16 *rollerDist8)
{
	int column;
	int i, j = 0, k;
	memset(rollerDist, 0, sizeof(s16)*BOARD_COLUMN*ROLLER_ICON_LEN);
	int rollerDist10, rollerDist11, rollerDist12, rollerDist13, rollerDist20, rollerDist21, rollerDist22, rollerDist23;
	int rollerDist30, rollerDist31, rollerDist32, rollerDist33, rollerDist40, rollerDist41, rollerDist42, rollerDist43;
	int rollerDist50, rollerDist51, rollerDist52, rollerDist53, rollerDist60, rollerDist61, rollerDist62, rollerDist63;
	int rollerDist70, rollerDist71, rollerDist72, rollerDist73, rollerDist80, rollerDist81, rollerDist82, rollerDist83;
	rollerDist10 = rollerDist11 = rollerDist12 = rollerDist13 = rollerDist20 = rollerDist21 = rollerDist22 = rollerDist23 = *rollerDist1 = *rollerDist2 = 0;
	rollerDist30 = rollerDist31 = rollerDist32 = rollerDist33 = rollerDist40 = rollerDist41 = rollerDist42 = rollerDist43 = *rollerDist3 = *rollerDist4 = 0;
	rollerDist50 = rollerDist51 = rollerDist52 = rollerDist53 = rollerDist60 = rollerDist61 = rollerDist62 = rollerDist63 = *rollerDist5 = *rollerDist6 = 0;
	rollerDist70 = rollerDist71 = rollerDist72 = rollerDist73 = rollerDist80 = rollerDist81 = rollerDist82 = rollerDist83 = *rollerDist7 = *rollerDist8 = 0;

	for(column = 0; column < BOARD_COLUMN; column++)
	{
		i = column * BOARD_ROW;
		#ifdef SRAND
		j = randint(rollerIconNum[column]);
		#else
		j = (int)GetRand(rollerIconNum[column]);//j = 0~55(reel(1))
		#endif
		//j = 0; //test fixed board
		//REEL_DIST test
		rollerDist[column][j]++;
		if(column == 0 && j == 0){
			rollerDist10 = 1;
			rollerDist20 = 1;
			//printf("column:%d, j:%d, rollerDist10:%d, rollerDist20:%d, line:%d\n", column, j, rollerDist10, rollerDist20, __LINE__);
		}
		else if(column == 1 && j == 0){
			rollerDist11 = 1;
			rollerDist21 = 1;
			//printf("column:%d, j:%d, rollerDist11:%d, rollerDist21:%d, line:%d\n", column, j, rollerDist11, rollerDist21, __LINE__);
		}
		else if(column == 2 && j == 0){
			rollerDist12 = 1;
			//printf("column:%d, j:%d, rollerDist12:%d, line:%d\n", column, j, rollerDist12, __LINE__);
		}
		else if(column == 2 && j == 4){
			rollerDist22 = 1;
			//printf("column:%d, j:%d, rollerDist22:%d, line:%d\n", column, j, rollerDist22, __LINE__);
		}
		else if(column == 3 && j != 0){
			rollerDist13 = 1; 
			rollerDist23 = 1;
			//printf("column:%d, j:%d, rollerDist13:%d, rollerDist23:%d, line:%d\n", column, j, rollerDist13, rollerDist23, __LINE__);
		}	
		if(column == 0 && j == 1){
			rollerDist30 = 1;
			rollerDist40 = 1;
			//printf("column:%d, j:%d, rollerDist30:%d, rollerDist40:%d, line:%d\n", column, j, rollerDist30, rollerDist40, __LINE__);
		}
		else if(column == 1 && j == 1){
			rollerDist31 = 1;
			rollerDist41 = 1;
			//printf("column:%d, j:%d, rollerDist31:%d, rollerDist41:%d, line:%d\n", column, j, rollerDist31, rollerDist41, __LINE__);
		}
		else if(column == 2 && j == 1){
			rollerDist32 = 1;
			//printf("column:%d, j:%d, rollerDist32:%d, line:%d\n", column, j, rollerDist32, __LINE__);
		}
		else if(column == 2 && j == 4){
			rollerDist42 = 1;
			//printf("column:%d, j:%d, rollerDist42:%d, line:%d\n", column, j, rollerDist42, __LINE__);
		}
		else if(column == 3 && j != 1){
			rollerDist33 = 1; 
			rollerDist43 = 1;
			//printf("column:%d, j:%d, rollerDist33:%d, rollerDist43:%d, line:%d\n", column, j, rollerDist33, rollerDist43, __LINE__);
		}
		if(column == 0 && j == 2){
			rollerDist50 = 1;
			rollerDist60 = 1;
			//printf("column:%d, j:%d, rollerDist50:%d, rollerDist60:%d, line:%d\n", column, j, rollerDist50, rollerDist60, __LINE__);
		}
		else if(column == 1 && j == 2){
			rollerDist51 = 1;
			rollerDist61 = 1;
			//printf("column:%d, j:%d, rollerDist51:%d, rollerDist61:%d, line:%d\n", column, j, rollerDist51, rollerDist61, __LINE__);
		}
		else if(column == 2 && j == 2){
			rollerDist52 = 1;
			//printf("column:%d, j:%d, rollerDist52:%d, line:%d\n", column, j, rollerDist52, __LINE__);
		}
		else if(column == 2 && j == 4){
			rollerDist62 = 1;
			//printf("column:%d, j:%d, rollerDist62:%d, line:%d\n", column, j, rollerDist62, __LINE__);
		}
		else if(column == 3 && j != 2){
			rollerDist53 = 1; 
			rollerDist63 = 1;
			//printf("column:%d, j:%d, rollerDist53:%d, rollerDist63:%d, line:%d\n", column, j, rollerDist53, rollerDist63, __LINE__);
		}
		if(column == 0 && j == 3){
			rollerDist70 = 1;
			rollerDist80 = 1;
			//printf("column:%d, j:%d, rollerDist70:%d, rollerDist80:%d, line:%d\n", column, j, rollerDist70, rollerDist80, __LINE__);
		}
		else if(column == 1 && j == 3){
			rollerDist71 = 1;
			rollerDist81 = 1;
			//printf("column:%d, j:%d, rollerDist71:%d, rollerDist81:%d, line:%d\n", column, j, rollerDist71, rollerDist81, __LINE__);
		}
		else if(column == 2 && j == 3){
			rollerDist72 = 1;
			//printf("column:%d, j:%d, rollerDist72:%d, line:%d\n", column, j, rollerDist72, __LINE__);
		}
		else if(column == 2 && j == 4){
			rollerDist82 = 1;
			//printf("column:%d, j:%d, rollerDist82:%d, line:%d\n", column, j, rollerDist82, __LINE__);
		}
		else if(column == 3 && j != 3){
			rollerDist73 = 1; 
			rollerDist83 = 1;
			//printf("column:%d, j:%d, rollerDist73:%d, rollerDist83:%d, line:%d\n", column, j, rollerDist73, rollerDist83, __LINE__);
		}

		//printf("column:%d, j:%d, rollerDist[column][j]:%d, line:%d in board_gen_dist()\n", column, j, rollerDist[column][j], __LINE__);
		//REEL_DIST test end
		for(k = 0; k < BOARD_ROW; k++)
		{
			icons[i+k] = rollerIcon[column][j];
			#ifdef PRINT_BOARD  //要等盤面產生正確無誤才註解掉
			printf("icons[i+k]:%d, column:%d, i:%d, k:%d, j:%d, line:%d in board_gen_dist()\n", icons[i+k], column, i, k, j, __LINE__);
			#endif
			j++;
			if(j >= rollerIconNum[column])
				j = 0;
		}
	}
	if((rollerDist10 == 1) && (rollerDist11 == 1) && (rollerDist12 == 1) && (rollerDist13 == 1)){
		*rollerDist1 = 1;   
		//printf("*rollerDist1:%d, line:%d\n", *rollerDist1, __LINE__);
	}		
	if((rollerDist20 == 1) && (rollerDist21 == 1) && (rollerDist22 == 1) && (rollerDist23 == 1)){
		*rollerDist2 = 1;
		//printf("*rollerDist2:%d, line:%d\n", *rollerDist2, __LINE__);
	}
	if((rollerDist30 == 1) && (rollerDist31 == 1) && (rollerDist32 == 1) && (rollerDist33 == 1)){
		*rollerDist3 = 1;   
		//printf("*rollerDist3:%d, line:%d\n", *rollerDist3, __LINE__);
	}		
	if((rollerDist40 == 1) && (rollerDist41 == 1) && (rollerDist42 == 1) && (rollerDist43 == 1)){
		*rollerDist4 = 1;
		//printf("*rollerDist4:%d, line:%d\n", *rollerDist4, __LINE__);
	}
	if((rollerDist50 == 1) && (rollerDist51 == 1) && (rollerDist52 == 1) && (rollerDist53 == 1)){
		*rollerDist5 = 1;   
		//printf("*rollerDist5:%d, line:%d\n", *rollerDist5, __LINE__);
	}		
	if((rollerDist60 == 1) && (rollerDist61 == 1) && (rollerDist62 == 1) && (rollerDist63 == 1)){
		*rollerDist6 = 1;
		//printf("*rollerDist6:%d, line:%d\n", *rollerDist6, __LINE__);
	}
	if((rollerDist70 == 1) && (rollerDist71 == 1) && (rollerDist72 == 1) && (rollerDist73 == 1)){
		*rollerDist7 = 1;   
		//printf("*rollerDist7:%d, line:%d\n", *rollerDist7, __LINE__);
	}		
	if((rollerDist80 == 1) && (rollerDist81 == 1) && (rollerDist82 == 1) && (rollerDist83 == 1)){
		*rollerDist8 = 1;
		//printf("*rollerDist8:%d, line:%d\n", *rollerDist8, __LINE__);
	}
}
#endif

void board_gen(const uint8_t (*rollerIcon)[ROLLER_ICON_LEN], const uint16_t *rollerIconNum, int *icons)
{
	int column;
	int i, j, k;
	i = j = 0;

	for(column = 0; column < BOARD_COLUMN; column++)
	{
		i = column * BOARD_ROW;

		#ifdef SRAND
		j = randint(rollerIconNum[column]);
		#else
		j = (int)GetRand(rollerIconNum[column]);
		#endif
		//j = 0; //test fixed board
		for(k = 0; k < BOARD_ROW; k++)
		{
			icons[i+k] = rollerIcon[column][j];
			#ifdef PRINT_BOARD  //要等盤面產生正確無誤才註解掉
			printf("icons[i+k]:%d, column:%d, i:%d, k:%d, j:%d, line:%d in board_gen()\n", icons[i+k], column, i, k, j, __LINE__);
			#endif			
			j++; 
			if(j >= rollerIconNum[column])
				j = 0;
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
			#ifdef PRINT_BOARD
			printf("icons[i+k]:%d, column:%d, i:%d, k:%d, j:%d, line:%d in board_traverse_gen()\n", icons[i+k], column, i, k, j, __LINE__);
			#endif	
		}
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

int board_calMulti(s16 lineNum, int *icons, int (*lineInfos)[4], s16 *awardLineNum)
{
	int line, lineMulti, lineMultiMul, totalMulti = 0;
	int i, j, targetIcon, targetWildIsSeq, targetWildSeqNum, multiplier, mul;
	 
	#ifdef PRINT_BOARD
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
		mul = 0;
		targetWildIsSeq = 0;
		targetWildSeqNum = 0;
		if(icons[gLinePos[line][0]] == E_ICON_SC){
			#ifdef PRINT_BRD_CALMULTI
			printf("line:%d, gLinePos[line][0]:%d, icons[gLinePos[line][0]]:%d, __LINE__:%d, %s\n", line, gLinePos[line][0], icons[gLinePos[line][0]], __LINE__, __FUNCTION__);
			#endif
			continue;
		}

		targetIcon = icons[gLinePos[line][0]];
		#ifdef PRINT_BRD_CALMULTI
		printf("line:%d, gLinePos[line][0]:%d, targetIcon:%d, __LINE__:%d, %s\n", line, gLinePos[line][0], targetIcon, __LINE__, __FUNCTION__);
		#endif
		
		for(i = 1; i < BOARD_COLUMN; i++)
		{
			if(icons[gLinePos[line][i]] == E_ICON_SC){
				#ifdef PRINT_BRD_CALMULTI
				printf("icons[gLinePos[line][i]]:%d, i:%d, gLinePos[line][i]:%d, __LINE__:%d, %s\n", icons[gLinePos[line][i]], i, gLinePos[line][i], __LINE__, __FUNCTION__);
				#endif
				break;
			}else if(icons[gLinePos[line][i]] >= E_ICON_WILD){
				targetWildIsSeq = 1;
				targetWildSeqNum++;
				mul = icons[gLinePos[line][i]] - E_ICON_WILD;
				multiplier += mul;
				#ifdef PRINT_BRD_CALMULTI
				printf("i:%d, targetWildSeqNum:%d, targetWildIsSeq:%d, gLinePos[line][i]:%d, mul:%d, multiplier:%d, __LINE__:%d, %s\n", i, targetWildSeqNum, targetWildIsSeq, gLinePos[line][i], mul, multiplier, __LINE__, __FUNCTION__);
				#endif
			}else{
				targetWildIsSeq = 0;
				if(targetIcon >= E_ICON_WILD){ 
					mul = icons[gLinePos[line][i]] - E_ICON_WILD;
					multiplier += mul;
					targetIcon = icons[gLinePos[line][i]];
					#ifdef PRINT_BRD_CALMULTI
					printf("i:%d, targetIcon:%d, gLinePos[line][i]:%d, mul:%d, multiplier:%d, line:%d, %s in if >= E_ICON_WILD\n", i, targetIcon, gLinePos[line][i], mul, multiplier, __LINE__, __FUNCTION__);
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

		lineMulti = gIconMulti[targetIcon][i];
		#ifdef PRINT_BRD_CALMULTI 
		printf("i:%d, targetIcon:%d, lineMulti:%d, gIconMulti[targetIcon][i]:%d, __LINE__:%d, %s\n", i, targetIcon, lineMulti, gIconMulti[targetIcon][i], __LINE__, __FUNCTION__);
		#endif
		
		if(multiplier != 0){
			totalMulti += lineMulti * multiplier;
			#ifdef PRINT_BRD_CALMULTI 
			printf("i:%d, targetIcon:%d, totalMulti:%d, lineMulti:%d, multiplier:%d, __LINE__:%d, %s\n", i, targetIcon, totalMulti, lineMulti, multiplier, __LINE__, __FUNCTION__);
			#endif
		}else{
			totalMulti += lineMulti;
			#ifdef PRINT_BRD_CALMULTI 
			printf("i:%d, targetIcon:%d, totalMulti:%d, lineMulti:%d, __LINE__:%d, %s\n", i, targetIcon, totalMulti, lineMulti, __LINE__, __FUNCTION__);
			#endif
		}

		if(lineMulti > 0 && lineInfos && awardLineNum){
			lineInfos[*awardLineNum][0] = line + 1;
			lineInfos[*awardLineNum][1] = targetIcon;
			lineInfos[*awardLineNum][2] = lineMulti;
			lineInfos[*awardLineNum][3] = multiplier;
			(*awardLineNum)++;
			#ifdef DEBUG_PAYOUT_DIST 
			printf("*awardLineNum:%d, lineInfos[*awardLineNum][0]:%d, lineInfos[*awardLineNum][1]:%d, lineInfos[*awardLineNum][2]:%d, lineInfos[*awardLineNum][3]:%d, __LINE__:%d, %s\n", *awardLineNum, line+1, targetIcon, lineMulti, multiplier, __LINE__, __FUNCTION__);
			#endif
		}
	}
	return totalMulti;
}

int board_calPrizeType(int *icons)
{
	if(board_calIconNum(icons, E_ICON_SC) >= FREEGAME_THRESHOLD) 
		return E_PRIZETYPE_FREEGAME;
	else
		return E_PRIZETYPE_NORMAL;

	//return E_PRIZETYPE_FREEGAME; //test freegame only
	//return E_PRIZETYPE_NORMAL; //test basegame only
}

int board_calIconNum(int *icons, int targetIcon)
{
	int i, num = 0;
	for(i = 0; i < BOARD_ICON_NUM; i++){
		if(icons[i] == targetIcon)
			num++;
	}
	return num;
}

int board_calWildIconNum(int *icons, int targetIcon)
{
	int i, num = 0;
	for(i = 0; i < BOARD_ICON_NUM; i++){
		if(icons[i] >= targetIcon)
			num++;
	}
	return num;
}

#ifdef WILD_MUL_DIST
void wildGetMultiplier(int *icons, const uint32_t (*gMulValue)[2], const int WEIGHT_SUM, s16 *wildMul)
#else
void wildGetMultiplier(int *icons, const uint32_t (*gMulValue)[2], const int WEIGHT_SUM)
#endif
{
	int i, mul;
	#ifdef WILD_MUL_DIST
	memset(wildMul, 0,sizeof(wildMul));
	#endif
	for(i = 0; i < BOARD_ICON_NUM; i++){
		if(icons[i] == E_ICON_WILD){
			#ifdef TEST_TRANVERSAL
			mul = 2;
			//printf("test tranversal in wildGetMultiplier()\n");
			#else
			//mul = 2;  //test case
			mul = getRandomValue(gMulValue, MUL_NUM, WEIGHT_SUM);
			#ifdef WILD_MUL_DIST
			wildMul[mul]++;
			printf("mul:%d, wildMul[mul]:%d in wildGetMultiplier()\n", mul, wildMul[mul], __LINE__);
			#endif//WILD_MUL_DIST
			#endif// TEST_TRANVERSAL
			
			icons[i] += mul;//mul = 2 3 5, icons[i]= 15 16 18
			#ifdef WILD_GET_MULTIPLIER
			printf("i:%d, icons[i]:%d, mul:%d in wildGetMultiplier()\n", i, icons[i], mul, __LINE__);
			#endif
		}	
	}
	#ifdef PRINT_BOARD
    for (i = 0; i < BOARD_ICON_NUM; i++)//print after wild get mul board
		printf("i:%d, icons[i]:%d, line:%d in wildGetMultiplier()\n", i, icons[i], __LINE__);
	#endif
}

int getRandomValue(const uint32_t (*weight)[2], int len, int weightValue)
{																	
    int i;
	#ifdef SRAND
    int ran = randint(weightValue) + 1;
	#else
	int ran = GetRand(weightValue) + 1;
	#endif
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