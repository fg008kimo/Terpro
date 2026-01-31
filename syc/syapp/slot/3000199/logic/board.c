#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "board.h"
#include "../server.h"
#include "../utils.h"


int board_isNormalIcon(int icon)
{
	return icon > E_ICON_NULL && icon < E_ICON_MUL_BASE;
}

int board_isMulIcon(int icon)
{
	return icon == E_ICON_MUL_BASE;
}

void board_fill(const uint8_t (*rollerIcon)[ROLLER_ICON_LEN], const uint16_t *rollerIconNum, int *icons)// board_fill(背景盤面矩陣,背景輪帶各icon個數矩陣,待生成之盤面)
{
	int column, i, j, k;

	for(column = 0; column < BOARD_COLUMN; column++)//BOARD_COLUMN = 6
	{
		i = column * BOARD_ROW;//BOARD_ROW = 5
		j = (int)GetRand(rollerIconNum[column]);//從背景輪帶上隨機選出一個數字,依行數下去選
		for(k = 0; k < BOARD_ROW; k++)
		{
			if(icons[i+k] != E_ICON_NULL)//若icons上的某個位置不是null icon就break => 這邊是要把null icon填滿,若已是非null,就跳過
				break;

			icons[i+k] = rollerIcon[column][j];//icons 先放完第一個column(i=0,k=0~4(盤面上第1行);i=5,k=0~4(盤面上第2行);i=10,k=0~4((盤面上第3行)))

			j++;//cascade=0,選到的數字依序往下多選四個,因為一個column有5個icon。cascade=1,只填滿有null icon的格子
			if(j >= rollerIconNum[column])//直到j >= 那行背景輪帶的icon數量上限就歸0
				j = 0;
			printf("icons[i+k]:%d\n",icons[i+k]);
		}
	}
}

void board_handle_normalIcon(int *icons, int *normalIconsNum, int *multi, int (*rewardInfo)[3], s16 *rewardIconType)//rewardInfo refererence: int normalRewardInfos[MAX_CASCADE_NUM][MAX_REWARD_ICON_TYPE][3];(所以(*rewardInfo)涵蓋normalRewardInfos[MAX_CASCADE_NUM][MAX_REWARD_ICON_TYPE])
{
	int i;

	memset(normalIconsNum, 0, sizeof(int) * ICON_NUM);//sizeof(int) * ICON_NUM = 4*11 = 44
	*multi = 0;
	*rewardIconType = 0;

	for(i = 0; i < BOARD_ICON_NUM; i++)//BOARD_ICON_NUM = 30 (這個for的i是盤面位置)
	{
		if(board_isNormalIcon(icons[i]))//如果是 icon = 1~9(心型糖果~香蕉)
			normalIconsNum[icons[i]]++;//normalIconsNum[icons[i]] = 統計每個水果的個數的矩陣
		printf("i:%d, icons[i]:%d, normalIconsNum[icons[i]]:%d\n", i, icons[i], normalIconsNum[icons[i]]);
	}//line54的normalIconsNum[icons[i]] 與 line65的normalIconsNum[i]都代表有得分的水果的個數,但只是因為兩個迴圈的i代表涵義不同

	for(i = E_ICON_PIPC1; i <= E_ICON_BANANA; i++)//(這個for的i是第幾個水果)
	{
		if(gIconMulti[i][normalIconsNum[i]] == 0)
			continue;

		*multi += gIconMulti[i][normalIconsNum[i]];//multi = 這局有得分的水果的分數相加總合, gIconMulti[i][normalIconsNum[i]] = 這局單一得分的水果的分數, normalIconsNum[i] = 得分的水果的個數
		rewardInfo[*rewardIconType][0] = i;//rewardInfo = normalRewardInfos[MAX_CASCADE_NUM],只是這邊用不到 CASCADE_NUM(第一維) 所以不用表現出來
		rewardInfo[*rewardIconType][1] = normalIconsNum[i];//因為超過8個才有分數,表示gIconMulti[i][normalIconsNum[i]] != 0, normalIconsNum[i]紀錄有分數的ICON的個數
		rewardInfo[*rewardIconType][2] = gIconMulti[i][normalIconsNum[i]];//gIconMulti = 图标中奖倍率矩陣,  gIconMulti[i][normalIconsNum[i]]存有分數的ICON的分數
		(*rewardIconType)++;// = 0 1 2 3(rewardIconType = 盤面上得分的是第幾種水果,因為盤面只有30格,一種得分水果佔8格,所以不會超過4種得分水果)
		printf("i:%d, normalIconsNum[i]:%d, gIconMulti[i][normalIconsNum[i]]:%d, *multi:%d, *rewardIconType:%d\n", i, normalIconsNum[icons[i]], gIconMulti[i][normalIconsNum[i]], *multi, *rewardIconType);
	}
}

void board_remove_reward_normalIcon(int *icons, const int *normalIconsNum)//這裡的*icons是icons[*cascadeNum+1](下一次的待換成icon_null的盤面)(icon reference: normalIcons[MAX_CASCADE_NUM][BOARD_ICON_NUM])
{
	int i, j;

	for(i = E_ICON_PIPC1; i <= E_ICON_BANANA; i++)
	{
		if(gIconMulti[i][normalIconsNum[i]] == 0)//如果沒有超過進入得分區的icon就continue
			continue;
		//printf("i:%d, gIconMulti[i][normalIconsNum[i]]:%d\n", i, gIconMulti[i][normalIconsNum[i]]);
		
		for(j = 0; j < BOARD_ICON_NUM; j++)
		{
			if(icons[j] == i)
				icons[j] = E_ICON_NULL;//將盤面上有得分(超過八個)的icon換成null icon
			printf("icons[j]:%d\n",icons[j]);
		}
	}
}

void board_cascade(int *icons)//消除得分的ICON後,若NULL ICON在NORMAL ICON中間,上方的NORMAL ICON需要往下掉,這邊用交換位置的方式
{
	int column, i, j, k, num;

	for(column = 0; column < BOARD_COLUMN; column++)
	{
		i = column * BOARD_ROW;
		for(j = BOARD_ROW - 1; j > 0; j--)//從column的最後一格往上找(往上找,array index是減少)
		{
			if(icons[i+j] == E_ICON_NULL)//若icon是null icon
			{
				for(k = j - 1; k >= 0; k--)//從已知是null icon的位置往上找
				{
					if(icons[i+k] != E_ICON_NULL)//往上找直到找到非null icon
					{
						swap(&icons[i+j], &icons[i+k]);
						printf("icons[i+j]:%d, icons[i+k]:%d\n", icons[i+j], icons[i+k]);
						break;
					}
				}
				if(k <= 0)//若k超出column矩陣上界線,就break
					break;
			}
		}
	}
}

void board_handle_scatterIcon(int *icons, int *multi, int (*rewardInfo)[3], s16 *rewardIconType)//處理盤面SC。 rewardInfo refererence: int normalRewardInfos[MAX_CASCADE_NUM][MAX_REWARD_ICON_TYPE][3];(所以(*rewardInfo)涵蓋normalRewardInfos[MAX_CASCADE_NUM][MAX_REWARD_ICON_TYPE])
{
	int num;

	num = board_calIconNum(icons, E_ICON_SCATTER);
	if(gIconMulti[E_ICON_SCATTER][num] > 0)//若SC數量>=4
	{
		*multi += gIconMulti[E_ICON_SCATTER][num];//multi = SC的分數相加
		rewardInfo[*rewardIconType][0] = E_ICON_SCATTER;//
		rewardInfo[*rewardIconType][1] = num;
		rewardInfo[*rewardIconType][2] = gIconMulti[E_ICON_SCATTER][num];
		(*rewardIconType)++;//rewardInfo 的資料結構其實 = normalRewardInfos[MAX_CASCADE_NUM],只是這邊用不到 CASCADE_NUM 所以不用表現出來,所以rewardInfo在這邊都還是存此盤面的得分資訊,沒有前一盤,後一盤
		printf("*rewardIconType:%d, E_ICON_SCATTER:%d, num:%d, gIconMulti[E_ICON_SCATTER][num]:%d, %d\n", *rewardIconType, E_ICON_SCATTER, num, gIconMulti[E_ICON_SCATTER][num], __LINE__);
	}
}

void board_handle_mulIcon(const uint32_t *mulIconProb, int *icons, int *mulIconMultis, s16 *mulIconNum)//處理盤面炸彈。mulIconProb 在base game = NULL, mulIconProb 在free game = freegameMulIconMultis
{
	int i;
	
	*mulIconNum = board_calIconNum(icons, E_ICON_MUL_BASE);
	printf("*mulIconNum:%d\n",*mulIconNum);
	for(i = 0; i < *mulIconNum; i++){
		mulIconMultis[i] = gMulIconMulti[RandSelect(mulIconProb, MUL_ICON_TYPE, PROB_SUM)];//gMulIconMulti[MUL_ICON_TYPE] = {2,3,5,8,10,12,15,18,20,25,30,35,50,100}
		printf("i:%d, mulIconMultis[i]:%d\n", i, mulIconMultis[i]);
	}
}

/////////////////////////////////【对外接口】////////////////////////////////////////

void board_gen(const uint8_t (*mainRollerIcon)[ROLLER_ICON_LEN], const uint16_t *mainRollerIconNum, //有const代表此參數不能被修改(mainRollerIcon和secRollerIcon都是背景輪帶,不能改返回值;icons是盤面陣列,可以修改返回值)
    const uint8_t (*secRollerIcon)[ROLLER_ICON_LEN], const uint16_t *secRollerIconNum, const uint32_t *mulIconProb, 
    int (*icons)[BOARD_ICON_NUM], int *multis, int (*rewardInfos)[MAX_REWARD_ICON_TYPE][3], int *mulIconMultis, 
    s16 *cascadeNum, s16 *rewardIconTypes, s16 *mulIconNum)
{
	const uint8_t (*rollerIcon)[ROLLER_ICON_LEN];
	const uint16_t *rollerIconNum;
	int normalIconsNum[ICON_NUM];//normalIconsNum用來記錄盤面上普通icon數量, ICON_NUM = 图标种类 = 11

	*cascadeNum = 0;//用來決定main or second的開關(main 是初始盤面,second是得分的水果,消除後的盤面)
	printf("enter board_gen *cascadeNum = %d\n", *cascadeNum);
	while(1)
	{
		rollerIcon = (*cascadeNum == 0 ? mainRollerIcon : secRollerIcon);//rollerIcon載入背景盤面
		rollerIconNum = (*cascadeNum == 0 ? mainRollerIconNum : secRollerIconNum);//rollerIconNum載入背景盤面六個輪帶的各個icon個數
		if(*cascadeNum == 0)
		{
			memset(icons[*cascadeNum], E_ICON_NULL, sizeof(int)*BOARD_ICON_NUM);//BOARD_ICON_NUM = 30, 若cascadeNum=0就將盤面都塞滿null icon, sizeof(int) = 4
		}

		// 填充所有空格
		board_fill(rollerIcon, rollerIconNum, icons[*cascadeNum]);
		// 统计获奖的普通图
		board_handle_normalIcon(icons[*cascadeNum], normalIconsNum, &multis[*cascadeNum], rewardInfos[*cascadeNum], &rewardIconTypes[*cascadeNum]);//rewardIconTypes用來放得分的水果有幾種,multis是此局遊戲分數,rewardIconTypes=三维数组[[[图标id,个数,倍率]]]
		//(multis 和 rewardIconTypes 在function內部是int要加&,rewardInfos是array就不用)

		printf("end of board_handle_normalIcon() *cascadeNum:%d, multis[*cascadeNum]:%d\n",*cascadeNum, multis[*cascadeNum]);
		// 初始化下个盘面
		if(multis[*cascadeNum] > 0)// > 0代表此次盤面有消除
		{
			// 直接拷贝
			cpy_int_arr(icons[*cascadeNum+1], icons[*cascadeNum], BOARD_ICON_NUM);//icons reference: normalIcons[MAX_CASCADE_NUM][BOARD_ICON_NUM], 所以 *cascadeNum+1 是第下一次的消除數
			// 消除已中奖的普通图标
			board_remove_reward_normalIcon(icons[*cascadeNum+1], normalIconsNum);
			// 上方图标滑落填充空格
			board_cascade(icons[*cascadeNum+1]);//cascade:傾瀉
		}

		if((multis[*cascadeNum] == 0) || (*cascadeNum + 1 == MAX_CASCADE_NUM)) //结束 (MAX_CASCADE_NUM=100,至多刷盤面100次)
		{
			// 统计获奖的分散图（加到最后一个盘面中）
			board_handle_scatterIcon(icons[*cascadeNum], &multis[*cascadeNum], rewardInfos[*cascadeNum], &rewardIconTypes[*cascadeNum]);//是int的參數都要加&,rewardInfos是array不用加
			// 统计获奖的倍率符 (炸彈)
			board_handle_mulIcon(mulIconProb, icons[*cascadeNum], mulIconMultis, mulIconNum);

			(*cascadeNum)++;
			if(*cascadeNum == MAX_CASCADE_NUM)
				LOGE("board_gen error: max cascade illegal");
			break;//只要這盤沒有一個水果超過8個(沒有可以銷的),都會break
		}
		else
		{
			(*cascadeNum)++;
		}
		printf("end of while loop *cascadeNum:%d\n", *cascadeNum);
	}
}

int board_calPrizeType(int *icons)
{
	if(board_calIconNum(icons, E_ICON_SCATTER) >= gFreeGameInfo[0][0])//gFreeGameInfo[0][0] = 4, 4個棒糖進入free game
		return E_PRIZETYPE_FREEGAME;
	else
		return E_PRIZETYPE_NORMAL;
}

int board_calIconNum(int *icons, int targetIcon)
{
	int i, num = 0;
	for(i = 0; i < BOARD_ICON_NUM; i++)
		if(icons[i] == targetIcon)
			num++;
	return num;
}
