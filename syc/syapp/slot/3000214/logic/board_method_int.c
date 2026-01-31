#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#include "board.h"
#include "../server.h"
#include "../utils.h"
#include "../data.h"

//#define PRINT_BOARD    			1
//#define PRINT_BRD_CALMULTI    	1
//#define PRINT_RAND    			1

/////////////////////////////////【对外接口】////////////////////////////////////////

void board_gen(const int (*tableWeight)[TABLE_NUM][2], const int tableLen, const int (*symbolWeight)[TABLE_NUM][BOARD_COLUMN][ICON_NUM][2], const int (*symbolLen)[TABLE_NUM][BOARD_COLUMN], const int (*mulWeight)[MUL_NUM][2], int *icons, int respinIcon, int option)
{
	int i, j;
	int tablePos;
	if(respinIcon == 1)
		tablePos = 3;
	else
		tablePos = getRandomValue(tableWeight[option], tableLen, TABLE_WEIGHT_SUM);
	#ifdef PRINT_BOARD
	printf("tablePos:%d, line:%d in board_gen()\n", tablePos, __LINE__);
	//for(i = 0; i < BOARD_COLUMN; i++){
	//	printf("symbolLen[option][tablePos][i]:%d, line:%d in board_gen()\n", symbolLen[option][tablePos][i], __LINE__);
	//	for(j = 0; j < ICON_NUM; j++)
	//		printf("i:%d, j:%d, symbolWeight[option][tablePos][i][j][0]:%d, symbolWeight[option][tablePos][i][j][1]:%d, line:%d in board_gen()\n", i, j, symbolWeight[option][tablePos][i][j][0], symbolWeight[option][tablePos][i][j][1], __LINE__);
	//}
	#endif
	
	for(i = 0; i < BOARD_COLUMN; i++){
		icons[i] = getRandomValue(symbolWeight[option][tablePos][i], ICON_NUM, symbolLen[option][tablePos][i]);
		if(icons[3] == E_ICON_MUL){
			icons[3] = getRandomValue(mulWeight[option], MUL_NUM, MUL_WEIGHT_SUM);
			if(icons[3] == 2)
				icons[3] = E_ICON_MUL2;
			if(icons[3] == 5)
				icons[3] = E_ICON_MUL5;
			if(icons[3] == 10)
				icons[3] = E_ICON_MUL10;			
		}
		#ifdef PRINT_BOARD
		printf("icons[%d]:%d, line:%d in board_gen()\n", i, icons[i], __LINE__);
		#endif
	}
}

double board_calMulti(int *icons)
{
	int i;
	double totalMulti = 0.0;
	
	if((icons[0] == E_ICON_PIPCEMPTY) || (icons[0] == E_ICON_PIPC000) || (icons[0] == E_ICON_PIPC00) || (icons[0] == E_ICON_PIPC0) || (icons[0] == E_ICON_PIPCDOT)){//從盤面最左邊算(最大bit開始數)
		totalMulti = 0;
		#ifdef PRINT_BRD_CALMULTI
		printf("totalMulti:%lf, line:%d in board_calMulti()\n", totalMulti, __LINE__);
		#endif
	}else if(icons[0] == E_ICON_PIPC1){
		totalMulti = 1;
		#ifdef PRINT_BRD_CALMULTI
		printf("totalMulti:%lf, line:%d in board_calMulti()\n", totalMulti, __LINE__);
		#endif
	}else if(icons[0] == E_ICON_PIPC5){
		totalMulti = 5;
		#ifdef PRINT_BRD_CALMULTI
		printf("totalMulti:%lf, line:%d in board_calMulti()\n", totalMulti, __LINE__);
		#endif
	}else if(icons[0] == E_ICON_PIPC10){
		totalMulti = 10;
		#ifdef PRINT_BRD_CALMULTI
		printf("totalMulti:%lf, line:%d in board_calMulti()\n", totalMulti, __LINE__);
		#endif
	}else if(icons[0] == E_ICON_PIPC50){
		totalMulti = 50;
		#ifdef PRINT_BRD_CALMULTI
		printf("totalMulti:%lf, line:%d in board_calMulti()\n", totalMulti, __LINE__);
		#endif
	}else if(icons[0] == E_ICON_PIPC100){
		totalMulti = 100;
		#ifdef PRINT_BRD_CALMULTI
		printf("totalMulti:%lf, line:%d in board_calMulti()\n", totalMulti, __LINE__);
		#endif
	}else if(icons[0] == E_ICON_PIPC500){
		totalMulti = 500;
		#ifdef PRINT_BRD_CALMULTI
		printf("totalMulti:%lf, line:%d in board_calMulti()\n", totalMulti, __LINE__);
		#endif
	}
		
	for(i = 1; i < BOARD_COLUMN - 1; i++){
		if((icons[i] == E_ICON_PIPCEMPTY)){
			#ifdef PRINT_BRD_CALMULTI
			printf("icons[i] is NULL, cotinue, line:%d in board_calMulti()\n", totalMulti, __LINE__);
			#endif
			continue;
		}else if(icons[i] == E_ICON_PIPCDOT){
			if((icons[i + 1] == E_ICON_PIPCEMPTY) || (icons[i + 1] == E_ICON_PIPC000) || (icons[i + 1] == E_ICON_PIPC00) || (icons[i + 1] == E_ICON_PIPC0) || (icons[i + 1] == E_ICON_PIPCDOT)){
				#ifdef PRINT_BRD_CALMULTI
				printf("totalMulti:%lf, line:%d in board_calMulti()\n", totalMulti, __LINE__);
				#endif
				break;
			}else if((icons[i + 1] == E_ICON_PIPC1) || (icons[i + 1] == E_ICON_PIPC10) || (icons[i + 1] == E_ICON_PIPC100)){				
				totalMulti += 0.1;
				#ifdef PRINT_BRD_CALMULTI
				printf("totalMulti:%lf, line:%d in board_calMulti()\n", totalMulti, __LINE__);
				#endif				
				break;
			}else if((icons[i + 1] == E_ICON_PIPC5) || (icons[i + 1] == E_ICON_PIPC50) || (icons[i + 1] == E_ICON_PIPC500)){			
				totalMulti += 0.5;
				#ifdef PRINT_BRD_CALMULTI
				printf("totalMulti:%lf, line:%d in board_calMulti()\n", totalMulti, __LINE__);
				#endif					
				break;
			}
		}else{
			totalMulti *= 10;
			#ifdef PRINT_BRD_CALMULTI
			printf("totalMulti:%lf, line:%d in board_calMulti()\n", totalMulti, __LINE__);
			#endif
			if(icons[i] == E_ICON_PIPC000){
				totalMulti *= 100;
				#ifdef PRINT_BRD_CALMULTI
				printf("totalMulti:%lf, line:%d in board_calMulti()\n", totalMulti, __LINE__);
				#endif
			}else if(icons[i] == E_ICON_PIPC00){
				totalMulti *= 10;
				#ifdef PRINT_BRD_CALMULTI
				printf("totalMulti:%lf, line:%d in board_calMulti()\n", totalMulti, __LINE__);
				#endif
			}else if(icons[i] == E_ICON_PIPC1){
				totalMulti += 1;
				#ifdef PRINT_BRD_CALMULTI
				printf("totalMulti:%lf, line:%d in board_calMulti()\n", totalMulti, __LINE__);
				#endif
			}else if(icons[i] == E_ICON_PIPC5){
				totalMulti += 5;
				#ifdef PRINT_BRD_CALMULTI
				printf("totalMulti:%lf, line:%d in board_calMulti()\n", totalMulti, __LINE__);
				#endif
			}else if(icons[i] == E_ICON_PIPC10){
				totalMulti *= 10;
				totalMulti += 10;
				#ifdef PRINT_BRD_CALMULTI
				printf("totalMulti:%lf, line:%d in board_calMulti()\n", totalMulti, __LINE__);
				#endif
			}else if(icons[i] == E_ICON_PIPC50){
				totalMulti *= 10;
				totalMulti += 50;
				#ifdef PRINT_BRD_CALMULTI
				printf("totalMulti:%lf, line:%d in board_calMulti()\n", totalMulti, __LINE__);
				#endif
			}else if(icons[i] == E_ICON_PIPC100){
				totalMulti *= 100;
				totalMulti += 100;
				#ifdef PRINT_BRD_CALMULTI
				printf("totalMulti:%lf, line:%d in board_calMulti()\n", totalMulti, __LINE__);
				#endif
			}else if(icons[i] == E_ICON_PIPC500){
				totalMulti *= 100;
				totalMulti += 500;
				#ifdef PRINT_BRD_CALMULTI
				printf("totalMulti:%lf, line:%d in board_calMulti()\n", totalMulti, __LINE__);
				#endif
			}
			//if icons[i] = E_ICON_PIPC0, no need to do anything
		}
	}
	
	if(icons[3] == E_ICON_MUL2){
		totalMulti *= 2;
		#ifdef PRINT_BRD_CALMULTI
		printf("totalMulti:%lf, line:%d in board_calMulti()\n", totalMulti, __LINE__);
		#endif
	}else if(icons[3] == E_ICON_MUL5){
		totalMulti *= 5;
		#ifdef PRINT_BRD_CALMULTI
		printf("totalMulti:%lf, line:%d in board_calMulti()\n", totalMulti, __LINE__);
		#endif		
	}else if(icons[3] == E_ICON_MUL10){
		totalMulti *= 10;
		#ifdef PRINT_BRD_CALMULTI
		printf("totalMulti:%lf, line:%d in board_calMulti()\n", totalMulti, __LINE__);
		#endif		
	}
	#ifdef PRINT_BRD_CALMULTI
	printf("final totalMulti:%lf, line:%d in board_calMulti()\n", totalMulti, __LINE__);
	#endif	

	return totalMulti;
}

int getRandomValue(const int (*weight)[2], int len, int weightValue)
{																	
    int i;
	if(weightValue == 1) 
		weightValue++;
	#ifdef PRINT_RAND
	printf("weightValue:%d\n", weightValue);
	#endif
	if(weightValue == 1) 
		weightValue++;
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
