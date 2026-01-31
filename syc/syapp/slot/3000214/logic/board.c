#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#include "board.h"
#include "../server.h"
#include "../utils.h"
#include "../data.h"

#ifdef DEBUG_MODE
	#define PRINT_BOARD    				1
	#define PRINT_BRD_CALMULTI    		1
	#define PRINT_PERFORM    			1
#endif	
//#define PRINT_RAND    			1

/////////////////////////////////【对外接口】////////////////////////////////////////

void board_gen(const uint32_t (*tableWeight)[TABLE_NUM][2], const int tableLen, const uint32_t (*symbolWeight)[TABLE_NUM][BOARD_COLUMN][ICON_NUM][2], const uint32_t (*symbolLen)[TABLE_NUM][BOARD_COLUMN], const uint32_t (*mulWeight)[MUL_NUM][2], const uint8_t (*rollerIcon)[BOARD_COLUMN][ROLLER_ICON_LEN], const uint16_t (*rollerIconNum)[BOARD_COLUMN], int *icons, int respinIcon, int option)
{
	int column, i, j;
	int tablePos;
	
	//table decision
	if(respinIcon == 1)
		tablePos = 3;
	else
		tablePos = getRandomValue(tableWeight[option], tableLen, TABLE_WEIGHT_SUM);
	#ifdef PRINT_BOARD
	printf("tablePos:%d, line:%d in board_gen()\n", tablePos, __LINE__);
	//for(column = 0; column < BOARD_COLUMN; column++){
	//	printf("symbolLen[option][tablePos][column]:%d, line:%d in board_gen()\n", symbolLen[option][tablePos][column], __LINE__);
	//	for(j = 0; j < ICON_NUM; j++)
	//		printf("column:%d, j:%d, symbolWeight[option][tablePos][column][j][0]:%d, symbolWeight[option][tablePos][column][j][1]:%d, line:%d in board_gen()\n", column, j, symbolWeight[option][tablePos][column][j][0], symbolWeight[option][tablePos][column][j][1], __LINE__);
	//}
	#endif
	
	//generate the position 1 4 7 10 on board first, then if 1 4 7 10 are nonempty, 0 3 6 9 and 2 5 8 11 are empty, vice versa.
	for(column = 0; column < BOARD_COLUMN; column++){
		i = column * BOARD_ROW;
		icons[i + 1] = getRandomValue(symbolWeight[option][tablePos][column], ICON_NUM, symbolLen[option][tablePos][column]);
		if(icons[10] == E_ICON_MUL){
			icons[10] = getRandomValue(mulWeight[option], MUL_NUM, MUL_WEIGHT_SUM);
			if(icons[10] == 2)
				icons[10] = E_ICON_MUL2;
			if(icons[10] == 5)
				icons[10] = E_ICON_MUL5;			
			if(icons[10] == 10)
				icons[10] = E_ICON_MUL10;			
		}
		if(icons[i + 1] != E_ICON_PIPCEMPTY){//side row (main row = 1 4 7 10)
			icons[i] = E_ICON_PIPCEMPTY;
			icons[i + 2] = E_ICON_PIPCEMPTY;
		}else{
			j = (int)GetRand(rollerIconNum[option][column]);
			//#ifdef PRINT_BOARD
			//printf("j:%d, line:%d in board_gen()\n", j, __LINE__);
			//#endif
			icons[i] = rollerIcon[option][column][j];
			if(j + 1 >= rollerIconNum[option][column]) 
				j = -1;
			icons[i + 2] = rollerIcon[option][column][j + 1];
		}
		#ifdef PRINT_BOARD
		printf("icons[%d]:%d, icons[%d]:%d, icons[%d]:%d, line:%d in board_gen()\n", i, icons[i], i+1, icons[i+1], i+2, icons[i+2], __LINE__);
		#endif
	}
}

s64 board_calMulti(int *icons, int times)
{
	int i;
	char *totalBuf = (char *)malloc(sizeof(char)*4*BOARD_COLUMN);
	memset(totalBuf, 0, sizeof(totalBuf));
	double totalMultiTemp = 0;
	s64 totalMulti = 0;
		
	for(i = 1; i < BOARD_ICON_NUM; i += 3){
		if(icons[i] == E_ICON_PIPCEMPTY){
			#ifdef PRINT_BRD_CALMULTI
			printf("icons[%d]:%d, it is NULL, cotinue, totalBuf:%s in board_calMulti()\n", i, icons[i], totalBuf, __LINE__);
			#endif
			continue;
		}else if(icons[i] == E_ICON_PIPCDOT){
			strncat(totalBuf, ".", 1);
			#ifdef PRINT_BRD_CALMULTI
			printf("icons[%d]:%d, totalBuf:%s line:%d in board_calMulti()\n", i, icons[i], totalBuf, __LINE__);
			#endif
		}else if(icons[i] == E_ICON_PIPC000){
			strncat(totalBuf, "000", 3);
			#ifdef PRINT_BRD_CALMULTI
			printf("icons[%d]:%d, totalBuf:%s line:%d in board_calMulti()\n", i, icons[i], totalBuf, __LINE__);
			#endif
		}else if(icons[i] == E_ICON_PIPC00){
			strncat(totalBuf, "00", 2);
			#ifdef PRINT_BRD_CALMULTI
			printf("icons[%d]:%d, totalBuf:%s, line:%d in board_calMulti()\n", i, icons[i], totalBuf, __LINE__);
			#endif
		}else if(icons[i] == E_ICON_PIPC0){
			strncat(totalBuf, "0", 1);
			#ifdef PRINT_BRD_CALMULTI
			printf("icons[%d]:%d, totalBuf:%s, line:%d in board_calMulti()\n", i, icons[i], totalBuf, __LINE__);
			#endif
		}else if(icons[i] == E_ICON_PIPC1){
			strncat(totalBuf, "1", 1);
			#ifdef PRINT_BRD_CALMULTI
			printf("icons[%d]:%d, totalBuf:%s, line:%d in board_calMulti()\n", i, icons[i], totalBuf, __LINE__);
			#endif
		}else if(icons[i] == E_ICON_PIPC5){
			strncat(totalBuf, "5", 1);
			#ifdef PRINT_BRD_CALMULTI
			printf("icons[%d]:%d, totalBuf:%s, line:%d in board_calMulti()\n", i, icons[i], totalBuf, __LINE__);
			#endif
		}else if(icons[i] == E_ICON_PIPC10){
			strncat(totalBuf, "10", 2);
			#ifdef PRINT_BRD_CALMULTI
			printf("icons[%d]:%d, totalBuf:%s, line:%d in board_calMulti()\n", i, icons[i], totalBuf, __LINE__);
			#endif
		}else if(icons[i] == E_ICON_PIPC50){
			strncat(totalBuf, "50", 2);
			#ifdef PRINT_BRD_CALMULTI
			printf("icons[%d]:%d, totalBuf:%s, line:%d in board_calMulti()\n", i, icons[i], totalBuf, __LINE__);
			#endif
		}else if(icons[i] == E_ICON_PIPC100){
			strncat(totalBuf, "100", 3);
			#ifdef PRINT_BRD_CALMULTI
			printf("icons[%d]:%d, totalBuf:%s, line:%d in board_calMulti()\n", i, icons[i], totalBuf, __LINE__);
			#endif
		}else if(icons[i] == E_ICON_PIPC500){
			strncat(totalBuf, "500", 3);
			#ifdef PRINT_BRD_CALMULTI
			printf("icons[%d]:%d, totalBuf:%s, line:%d in board_calMulti()\n", i, icons[i], totalBuf, __LINE__);
			#endif
		}else{
			#ifdef PRINT_BRD_CALMULTI
			printf("icons[%d]:%d, totalBuf:%s, error case: no corresponded symbol, line:%d in board_calMulti()\n", i, icons[i], totalBuf, __LINE__);
			#endif
		}
	}
	
	//add 0 in string
	if((icons[1] == E_ICON_PIPCEMPTY) && (icons[4] == E_ICON_PIPCDOT) && (icons[7] != E_ICON_PIPCEMPTY)){//add 0 at first position
		char *noZeroBuf = (char *)malloc(sizeof(char)*4*BOARD_COLUMN);
		memset(noZeroBuf, 0, sizeof(noZeroBuf));
		strncpy(noZeroBuf, totalBuf, sizeof(totalBuf));
		memset(totalBuf, 0, sizeof(totalBuf));
		strncat(totalBuf, "0", 1);
		strncat(totalBuf, noZeroBuf, sizeof(noZeroBuf));
		#ifdef PRINT_BRD_CALMULTI
		printf("totalBuf:%s, noZeroBuf:%s, line:%d in board_calMulti()\n", totalBuf, noZeroBuf, __LINE__);
		#endif
	}else if((icons[1] != E_ICON_PIPCEMPTY) && (icons[4] == E_ICON_PIPCDOT) && (icons[7] == E_ICON_PIPCEMPTY)){//add 0 at last position
		strncat(totalBuf, "0", 1);
		#ifdef PRINT_BRD_CALMULTI
		printf("totalBuf:%s, line:%d in board_calMulti()\n", totalBuf, __LINE__);
		#endif
	}else if((icons[1] == E_ICON_PIPCEMPTY) && (icons[4] == E_ICON_PIPCDOT) && (icons[7] == E_ICON_PIPCEMPTY)){
		char *noZeroBuf = (char *)malloc(sizeof(char)*4*BOARD_COLUMN);
		memset(noZeroBuf, 0, sizeof(noZeroBuf));
		strncpy(noZeroBuf, totalBuf, sizeof(totalBuf));
		memset(totalBuf, 0, sizeof(totalBuf));
		strncat(totalBuf, "0", 1);
		strncat(totalBuf, noZeroBuf, sizeof(noZeroBuf));
		strncat(totalBuf, "0", 1);
		#ifdef PRINT_BRD_CALMULTI
		printf("totalBuf:%s, line:%d in board_calMulti()\n", totalBuf, __LINE__);
		#endif
	}
	#ifdef PRINT_BRD_CALMULTI
	printf("final totalBuf:%s, line:%d in board_calMulti()\n", totalBuf, __LINE__);
	#endif
	
	totalMultiTemp = atof(totalBuf);
	totalMulti = totalMultiTemp * times;//times used to prevent USD = 0.1
	#ifdef PRINT_BRD_CALMULTI
	printf("totalMultiTemp:%lf, totalMulti:%ld, line:%d in board_calMulti()\n", totalMultiTemp, totalMulti, __LINE__);
	#endif
	if(icons[10] == E_ICON_MUL2){
		totalMulti *= 2;
		#ifdef PRINT_BRD_CALMULTI
		printf("totalMulti:%ld, line:%d in board_calMulti()\n", totalMulti, __LINE__);
		#endif
	}else if(icons[10] == E_ICON_MUL5){
		totalMulti *= 5;
		#ifdef PRINT_BRD_CALMULTI
		printf("totalMulti:%ld, line:%d in board_calMulti()\n", totalMulti, __LINE__);
		#endif		
	}else if(icons[10] == E_ICON_MUL10){
		totalMulti *= 10;
		#ifdef PRINT_BRD_CALMULTI
		printf("totalMulti:%ld, line:%d in board_calMulti()\n", totalMulti, __LINE__);
		#endif		
	}
	#ifdef PRINT_BRD_CALMULTI
	printf("final totalMulti:%ld, line:%d in board_calMulti()\n", totalMulti, __LINE__);
	#endif

	return totalMulti;
}

void board_genPerform(const uint32_t (*performActWeight)[2], const uint32_t (*performPosWeight)[2], const int performLen, const uint8_t (*rollerIcon)[BOARD_COLUMN][ROLLER_ICON_LEN], const uint16_t (*rollerIconNum)[BOARD_COLUMN], int *iconsBefore, int *icons, int option, s64 normalMulti)
{
	int performAct = getRandomValue(performActWeight, performLen, PERFORM_WEIGHT_SUM);
	#ifdef PRINT_PERFORM
	printf("performAct:%d, line:%d in board_genPerform()\n", performAct, __LINE__);
	#endif	
	if(performAct != 0){
		int i, j, k, column;
		for(column = 0; column < BOARD_COLUMN; column++){
			i = column * BOARD_ROW;
			#ifdef PRINT_PERFORM
			printf("i:%d, icons[i + 1]:%d, line:%d in board_genPerform()\n", i, icons[i + 1], __LINE__);
			#endif
			if((icons[i + 1] != E_ICON_PIPCEMPTY) && (icons[i + 1] != E_ICON_PIPCDOT)){
				int randPos;
				randPos = getRandomValue(performPosWeight, performLen, PERFORM_WEIGHT_SUM);
				iconsBefore[i + 1] = E_ICON_PIPCEMPTY;
				j = (int)GetRand(rollerIconNum[option][column]);
				#ifdef PRINT_PERFORM
				printf("j:%d, randPos:%d, line:%d in board_genPerform()\n", j, randPos, __LINE__);
				#endif
				if(randPos != 0){//i+1 turn to i
					iconsBefore[i] = icons[i + 1];
					iconsBefore[i + 2] = rollerIcon[option][column][j];
				}else{//i+1 turn to i+2
					iconsBefore[i + 2] = icons[i + 1];
					iconsBefore[i] = rollerIcon[option][column][j];
				}
			}else{
				for(k = 0; k < BOARD_ROW; k++)
					iconsBefore[i + k] = icons[i + k];
			}
			#ifdef PRINT_PERFORM
			printf("iconsBefore[%d]:%d, iconsBefore[%d]:%d, iconsBefore[%d]:%d, line:%d in board_genPerform()\n", i, iconsBefore[i], i+1, iconsBefore[i+1], i+2, iconsBefore[i+2], __LINE__);
			#endif
		}
		if(normalMulti == 0){
			for(i = 0; i < BOARD_ICON_NUM - 3; i++)
				iconsBefore[i] = icons[i];
		}
	}else{
		memcpy(iconsBefore, icons, sizeof(int)*BOARD_ICON_NUM);
		#ifdef PRINT_PERFORM
		int i, column;
		for(column = 0; column < BOARD_COLUMN; column++){
			i = column * BOARD_ROW;
			printf("iconsBefore[%d]:%d, iconsBefore[%d]:%d, iconsBefore[%d]:%d, line:%d in board_genPerform()\n", i, iconsBefore[i], i+1, iconsBefore[i+1], i+2, iconsBefore[i+2], __LINE__);
		}
		#endif		
	}
}

int getRandomValue(const uint32_t (*weight)[2], int len, int weightValue)
{																	
    int i;
	if(weightValue == 1) 
		weightValue++;
	#ifdef PRINT_RAND
	printf("weightValue:%d\n", weightValue);
	#endif
	int ran = GetRand(weightValue) + 1;
    for(i = len-1; i >= 0; i--){
		#ifdef PRINT_RAND
		printf("ran:%d, weight[%d][0]:%d, weight[%d][1]:%d\n", ran, i, weight[i][0], i, weight[i][1]);
		#endif	
		if(ran <= weight[i][1])
			return weight[i][0];
        ran -= weight[i][1];
    }
    return 0;
}


void difficulty_control(int diff, int upMulti, int downMulti, const double (*targetRTP)[MAX_DIFF_NUM][2], int isSelect_A, s32 totalMulti, int *loopTime, int option)
{
	double ranPro = GetRandom();
	if(diff < NORMAL_DIFF)
	{
		if(totalMulti <= downMulti && ranPro < (double)targetRTP[option][diff][2-isSelect_A]/100)   //上限位的rtp
			(*loopTime)--;
		else
			(*loopTime) = 0;
	}
	else
	{
		if(totalMulti > upMulti && ranPro < (double)targetRTP[option][diff][2-isSelect_A]/100)   //下限位的rtp
			(*loopTime)--;
		else
			(*loopTime) = 0;
	}
}
