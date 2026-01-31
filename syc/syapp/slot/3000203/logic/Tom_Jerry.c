#include "Tom_Jerry.h"

void gen_board(const uint8_t (*rollerIcon)[ROLLER_ICON_LEN], const uint16_t *rollerIconNum, const uint8_t (*activatedRollerIcon)[ROLLER_ICON_LEN], 
    const uint16_t *activatedRollerIconNum, int *icons, int *isActivated)
{
    int i, j;
    int index = 0;
    for (i = 0; i < BOARD_COLUMN; i++)
    {
        if(isActivated[i] == -1)
        {
            index = GetRand(rollerIconNum[i]);
            for (j = 0; j < BOARD_ROW; j++)
            {
                if(index+j >= rollerIconNum[i])
                    index -= rollerIconNum[i];
                icons[j+BOARD_ROW*i] = rollerIcon[i][index+j];
            }
        }
        else
        {
            index = GetRand(activatedRollerIconNum[i]);
            for (j = 0; j < BOARD_ROW; j++)
            {
                if(index+j >= activatedRollerIconNum[i])
                    index -= activatedRollerIconNum[i];
                icons[j+BOARD_ROW*i] = activatedRollerIcon[i][index+j];
            }
        }
    }
}

int calIconNum(int *icons, int icon)
{
    int i;
    int count = 0;
    for (i = 0; i < BOARD_ICON_NUM; i++)
    {
        if(icons[i] == icon)
            count++;
    }
    return count;
}

int isWildOrCatIcon(int icon)
{
    return icon == E_ICON_WILD || icon == E_ICON_CAT;
}

void changeWildMulti(int *icons, int *wildMultis)
{
    int i, j, k;

    for (i = 0; i < BOARD_COLUMN; i++)
    {
        wildMultis[i] = 1;
        for (j = 0; j < BOARD_ROW; j++)
        {
            if(icons[i*BOARD_ROW+j] == E_ICON_CAT)
            {
                for (k = j; k < BOARD_ROW; k++)
                {
                    if(icons[i*BOARD_ROW+k] == E_ICON_WILD)
                    {
                        wildMultis[i] = getRandomValue(gWildMultis, WILD_TYPE_NUM, WILD_WEIGHT);
                    }
                    icons[i*BOARD_ROW+k] = E_ICON_CAT;
                }
            }
        }
    }
}

int cal_boardMulti(s16 lineNum, int *icons, int (*lineInfos)[3], int *awardLineNum, int* wildColMulti)
{
    int i, j, k;
    int totalMulti = 0;

    if(awardLineNum)
		*awardLineNum = 0;
    
    changeWildMulti(icons, wildColMulti);
    for (k = 0; k < lineNum; k++)
    {
        int sameIconNum = 1;
        int targetIcon = 1;
        int wildMulti = 0;
        int lineMulti = 0;
        int index = 0;
        int singleMulti = 0;
        int lineIcons[BOARD_COLUMN] = {0};

        //计算cat和wild情况
        int lineWildCatIcons[BOARD_COLUMN] = {0};
        int sameWildCatIconNum = 0;
        int targetWildCatIcon = 1;

        for (i = 0; i < BOARD_COLUMN; i++)
        {
            lineIcons[i] = icons[i*BOARD_ROW+gLinePos[k][i]];
            lineWildCatIcons[i] = icons[i*BOARD_ROW+gLinePos[k][i]];
        }
       
        while(isWildOrCatIcon(lineIcons[index]))
        {
            index++;
            if(index == BOARD_COLUMN)
            {
                index = 0;
                break;
            }
        }

        //百搭图标的替换成其他类型的图标
        for (i = 0; i < BOARD_COLUMN; i++)
        {
            if(isWildOrCatIcon(lineIcons[i]))
                lineIcons[i] = lineIcons[index];
        }

        for (i = 1; i < BOARD_COLUMN; i++)
        {
            if(lineIcons[i-1] == lineIcons[i])
            {
                sameIconNum++;
                targetIcon = lineIcons[0];
            }
            else
                break;  
        }

        for (i = 0; i < BOARD_COLUMN; i++)
        {
            if(isWildOrCatIcon(lineWildCatIcons[i]))
            {
                sameWildCatIconNum++;
                targetWildCatIcon = lineWildCatIcons[0];
            }
            else
                break;
        }
        //比较wild图标倍率
        if(gIconMultis[targetWildCatIcon-1][sameWildCatIconNum] > gIconMultis[targetIcon-1][sameIconNum])
        {
            targetIcon = targetWildCatIcon;
            sameIconNum = sameWildCatIconNum;
        }

        if(sameIconNum >= 3 && targetIcon < E_ICON_SCAT)
        {
            for (i = 0; i < sameIconNum; i++)
            {
                if(isWildOrCatIcon(icons[i*BOARD_ROW+gLinePos[k][i]]))
                {
                    singleMulti = wildColMulti[i];
                    singleMulti = singleMulti == 1 ? 0 : singleMulti;
                    wildMulti += singleMulti;
                }
            }
            wildMulti = wildMulti == 0 ? 1 : wildMulti;
            lineMulti = gIconMultis[targetIcon-1][sameIconNum]*wildMulti;
            totalMulti += lineMulti;

            if(lineMulti > 0)
            {
                lineInfos[*awardLineNum][0] = k+1;
                lineInfos[*awardLineNum][1] = targetIcon;
                lineInfos[*awardLineNum][2] = lineMulti;
                (*awardLineNum)++;
            }
        }
    }
    return totalMulti;
}

void handldMaxxMode(int *icons, int *isActivated)
{
    int i, j;

    for (i = 0; i < BOARD_COLUMN; i++)
    {
        if(isActivated[i] > -1)
        {
            int ran = GetRand(gScatInFreeRell[2]);
            if(ran < gScatInFreeRell[0])
            {
                int ranRow = getRandomValue(gScatAppearInRow, BOARD_ROW, 4);
                int ranCat = getDynamicWeight(ranRow+1);
                icons[i*BOARD_ROW+ranRow] = E_ICON_SCAT;
                icons[i*BOARD_ROW+ranCat] = E_ICON_CAT;
            }
            else
            {
                int ranCat = getDynamicWeight(0);
                icons[i*BOARD_ROW+ranCat] = E_ICON_CAT;
            }
        }
    }

    for (i = 0; i < BOARD_COLUMN; i++)
    {
        for (j = 0; j < BOARD_ROW; j++)
        {
            if(icons[i*BOARD_ROW+j] == E_ICON_CAT)
            {
                isActivated[i] = i;
                break;
            }
        }       
    }
}

int getPrizeType(int *icons)
{
    if(calIconNum(icons, E_ICON_SCAT) >= gFreeGameInfo[0][0])
        return E_PRIZETYPE_FREEGAME;
    else
        return E_PRIZETYPE_NORMAL;
}

int getDynamicWeight(int scatPos)
{
    int i, j;
    int ran = GetRand(REEL_WEIGHT);
    int scatNotInFreeRell[BOARD_ROW][2] = {{0,140},{1,160},{2,180},{3,260},{4,260}};
    for (i = 0; i < scatPos; i++)
    {
        scatNotInFreeRell[scatPos][1] += scatNotInFreeRell[i][1];
        scatNotInFreeRell[i][1] = 0;
    }

    for(j = BOARD_ROW-1; j >= 0; j--)
    {
        if(ran < scatNotInFreeRell[j][1])
            return j;
        ran -= scatNotInFreeRell[j][1];
    }
    return 0;
}

int getRandomValue(const int (*weight)[2], int len, int max_weight)
{
    int i;
    int ran = GetRand(max_weight);
    for(i = len-1; i >= 0; i--)
    {
        if(ran < weight[i][1])
            return weight[i][0];
        ran -= weight[i][1];
    }
    return 0;
}

int selectWeight(s8 uid)
{
	int selectVersion = uid % 2 != 0 ? 7 : 7;   //uid奇数对应v0版本，偶数对应v3版本 
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