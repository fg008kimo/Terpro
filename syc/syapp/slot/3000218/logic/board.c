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

void board_fill(const uint8_t (*rollerIcon)[ROLLER_ICON_LEN], const uint16_t *rollerIconNum, int *icons)
{
	int column, i, j, k;

	for(column = 0; column < BOARD_COLUMN; column++)
	{
		i = column * BOARD_ROW;
		j = (int)GetRand(rollerIconNum[column]);
		for(k = 0; k < BOARD_ROW; k++)
		{
			if(icons[i+k] != E_ICON_NULL)
				break;

			icons[i+k] = rollerIcon[column][j];

			j++;
			if(j >= rollerIconNum[column])
				j = 0;
		}
	}
}

void board_handle_normalIcon(int *icons, int *normalIconsNum, int *multi, int (*rewardInfo)[3], s16 *rewardIconType)
{
	int i;

	memset(normalIconsNum, 0, sizeof(int) * ICON_NUM);
	*multi = 0;
	*rewardIconType = 0;

	for(i = 0; i < BOARD_ICON_NUM; i++)
	{
		if(board_isNormalIcon(icons[i]))
			normalIconsNum[icons[i]]++;
	}

	for(i = E_ICON_PIPC1; i <= E_ICON_BANANA; i++)
	{
		if(gIconMulti[i][normalIconsNum[i]] == 0)
			continue;

		*multi += gIconMulti[i][normalIconsNum[i]];
		rewardInfo[*rewardIconType][0] = i;
		rewardInfo[*rewardIconType][1] = normalIconsNum[i];
		rewardInfo[*rewardIconType][2] = gIconMulti[i][normalIconsNum[i]];
		(*rewardIconType)++;
	}
}

void board_remove_reward_normalIcon(int *icons, const int *normalIconsNum)
{
	int i, j;

	for(i = E_ICON_PIPC1; i <= E_ICON_BANANA; i++)
	{
		if(gIconMulti[i][normalIconsNum[i]] == 0)
			continue;
		
		for(j = 0; j < BOARD_ICON_NUM; j++)
		{
			if(icons[j] == i)
				icons[j] = E_ICON_NULL;
		}
	}
}

void board_cascade(int *icons)
{
	int column, i, j, k, num;

	for(column = 0; column < BOARD_COLUMN; column++)
	{
		i = column * BOARD_ROW;
		for(j = BOARD_ROW - 1; j > 0; j--)
		{
			if(icons[i+j] == E_ICON_NULL)
			{
				for(k = j - 1; k >= 0; k--)
				{
					if(icons[i+k] != E_ICON_NULL)
					{
						swap(&icons[i+j], &icons[i+k]);
						break;
					}
				}
				if(k <= 0)
					break;
			}
		}
	}
}

void board_handle_scatterIcon(int *icons, int *multi, int (*rewardInfo)[3], s16 *rewardIconType)
{
	int num;

	num = board_calIconNum(icons, E_ICON_SCATTER);
	if(gIconMulti[E_ICON_SCATTER][num] > 0)
	{
		*multi += gIconMulti[E_ICON_SCATTER][num];
		rewardInfo[*rewardIconType][0] = E_ICON_SCATTER;
		rewardInfo[*rewardIconType][1] = num;
		rewardInfo[*rewardIconType][2] = gIconMulti[E_ICON_SCATTER][num];
		(*rewardIconType)++;
	}
}

void board_handle_mulIcon(const uint32_t *mulIconProb, int *icons, int *mulIconMultis, s16 *mulIconNum)
{
	int i;
	
	*mulIconNum = board_calIconNum(icons, E_ICON_MUL_BASE);
	for(i = 0; i < *mulIconNum; i++)
		mulIconMultis[i] = gMulIconMulti[RandSelect(mulIconProb, MUL_ICON_TYPE, PROB_SUM)];
}

/////////////////////////////////【对外接口】////////////////////////////////////////

void board_gen(const uint8_t (*mainRollerIcon)[ROLLER_ICON_LEN], const uint16_t *mainRollerIconNum, 
    const uint8_t (*secRollerIcon)[ROLLER_ICON_LEN], const uint16_t *secRollerIconNum, const uint32_t *mulIconProb, 
    int (*icons)[BOARD_ICON_NUM], int *multis, int (*rewardInfos)[MAX_REWARD_ICON_TYPE][3], int *mulIconMultis, 
    s16 *cascadeNum, s16 *rewardIconTypes, s16 *mulIconNum)
{
	const uint8_t (*rollerIcon)[ROLLER_ICON_LEN];
	const uint16_t *rollerIconNum;
	int normalIconsNum[ICON_NUM];

	*cascadeNum = 0;
	while(1)
	{
		rollerIcon = (*cascadeNum == 0 ? mainRollerIcon : secRollerIcon);
		rollerIconNum = (*cascadeNum == 0 ? mainRollerIconNum : secRollerIconNum);
		if(*cascadeNum == 0)
		{
			memset(icons[*cascadeNum], E_ICON_NULL, sizeof(int)*BOARD_ICON_NUM);
		}

		// 填充所有空格
		board_fill(rollerIcon, rollerIconNum, icons[*cascadeNum]);
		// 统计获奖的普通图
		board_handle_normalIcon(icons[*cascadeNum], normalIconsNum, &multis[*cascadeNum], rewardInfos[*cascadeNum], &rewardIconTypes[*cascadeNum]);

		// 初始化下个盘面
		if(multis[*cascadeNum] > 0)
		{
			// 直接拷贝
			cpy_int_arr(icons[*cascadeNum+1], icons[*cascadeNum], BOARD_ICON_NUM);
			// 消除已中奖的普通图标
			board_remove_reward_normalIcon(icons[*cascadeNum+1], normalIconsNum);
			// 上方图标滑落填充空格
			board_cascade(icons[*cascadeNum+1]);
		}

		if((multis[*cascadeNum] == 0) || (*cascadeNum + 1 == MAX_CASCADE_NUM)) //结束
		{
			// 统计获奖的分散图（加到最后一个盘面中）
			board_handle_scatterIcon(icons[*cascadeNum], &multis[*cascadeNum], rewardInfos[*cascadeNum], &rewardIconTypes[*cascadeNum]);
			// 统计获奖的倍率符
			board_handle_mulIcon(mulIconProb, icons[*cascadeNum], mulIconMultis, mulIconNum);

			(*cascadeNum)++;
			if(*cascadeNum == MAX_CASCADE_NUM)
				LOGE("board_gen error: max cascade illegal");
			break;
		}
		else
		{
			(*cascadeNum)++;
		}
	}
}

int board_calPrizeType(int *icons)
{
	if(board_calIconNum(icons, E_ICON_SCATTER) >= gFreeGameInfo[0][0])
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
