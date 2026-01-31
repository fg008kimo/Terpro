#include "buff.h"
//printf("%s,%d\n",__FUNCTION__,__LINE__);     PRINT TEMPLATE

int gen_board(const uint8_t (*rollerIcon)[BOARD_COLUMN][ROLLER_ICON_LEN], const uint16_t (*rollerIconNum)[BOARD_COLUMN], const int (*reelWeight)[2], int len, int* icons)//reelWeight = gBaseReelWeight, len = BASE_REEL_NUM+4 = 12, icons = [BOARD_ICON_NUM]
{
    int i, j;
    int index = 0;
    int pos = 0; //母輪帶變數(底下還有R1~R5)

    int reelPos = getRandomValue(reelWeight, len, REEL_WEIGHT);//reelWeight = gBaseReelWeight[BASE_REEL_NUM+4][2] = {{0,8995},{1,264},{2,234},{3,172},{4,80},{5,80},{6,80},{7,130},{8,20},{9,20},{10,20},{11,5}}, len = 12, REEL_WEIGHT = 10000
    //printf("reelPos:%d\n", reelPos);//大機率reelPos都是0
	if(len == BASE_REEL_NUM)   // 免费游戏 BASE_REEL_NUM(reel數量) = 8
        pos = reelPos; 
    else
        pos = reelPos < E_ICON_PIC5 ? 0 : reelPos-4;//if reelPos < 5 choose 0 if not choose reelPos-4 (9825/10000(0+..+5的權重)會是 pos = 0)
	printf("pos:%d, reelPos:%d\n", pos, reelPos);
    for (i = 0; i < BOARD_COLUMN; i++)//BOARD_COLUMN = 5, i是R1~R5
    {
        index = GetRand(rollerIconNum[pos][i]);//pos是第幾條母REEL,i是R1~R5(其中一個子輪帶))
		printf("i:%d, index:%d, rollerIconNum[pos][%d]:%d, func:%s\n", i, index, i, rollerIconNum[pos][i], __FUNCTION__);
        for (j = 0; j < BOARD_ROW; j++)//BOARD_ROW = 4
        {
            if(index+j >= rollerIconNum[pos][i]){//若 index超出輪帶上限,回到輪帶上的第0個位置
                index -= rollerIconNum[pos][i];//若超過上限,進來此if,則index = index - 子輪帶長度(變成負數,然後在line 27有index+j就會=0)
				printf("in if index:%d, rollerIconNum[pos][i]\n", index, rollerIconNum[pos][i]);
			}
            icons[j+BOARD_ROW*i] = rollerIcon[pos][i][index+j];//將盤面存到icons一維矩陣  rollerIcon[REEL1~8][R1~R5][子輪帶的位置]
            
            //将免费游戏的PIC1图标根据相对应的权重替换成金牛符
            if(len == BASE_REEL_NUM)
            {
                if(icons[j+BOARD_ROW*i] == E_ICON_PIC1)//若icons上的元素已經是普通牛了
                {
                    int ran = GetRand(gPic1ToGoldWeight[2])+1;//gPic1ToGoldWeight為將PIC1轉為金牛用的矩陣, const int gPic1ToGoldWeight[3] = {30,70,100};(30 = Gold, 70 = Normal),所以ran 是機率 ,有30%能讓一般牛變金牛
                    if(ran <= gPic1ToGoldWeight[0]){
                        icons[j+BOARD_ROW*i] = E_ICON_BUFF;//E_ICON_BUFF=17 金牛
						printf("ran:%d, icons[j+BOARD_ROW*i]:%d\n", ran, icons[j+BOARD_ROW*i]);
					}
                }
            }
			printf("j:%d, icons[j+BOARD_ROW*i]:%d, func:%s\n", j, icons[j+BOARD_ROW*i], __FUNCTION__);
        }
    }
    return reelPos;
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

int findScatPos(int* icons, int icon)
{
    int i;
    for (i = 0; i < BOARD_ICON_NUM; i++)//BOARD_ICON_NUM = 20
    {
        if(icons[i] == icon){
			printf("i:%d, icons[i] == icon, %s\n", i, __FUNCTION__);
			break;
		}
    }
    int row = i/BOARD_ROW;//BOARD_ROW = 4, i = 0~3
    if(row == BOARD_COLUMN){//BOARD_COLUMN = 5 => i =20
        printf("row == BOARD_COLUMN, %s\n",__FUNCTION__);
		return 0;
	}
	printf("row:%d, %s\n", row, __FUNCTION__);
    return row;
}

void calArrangeIconNum(int prizeType, int* icons, int icon, int *gameMulti, int wildMulti)// 這裡 icons = iconsTemp, icon = icons[m] = 第一列icon, gameMulti是倍率矩陣, wildMulti是紀錄wild倍數variable
{
    int i, j;
    int iconNum = 1;
    int targetIcon = 0;
    int sameCount = 0;

    int index = findScatPos(icons, icon);
	printf("index:%d\n, %d", index, __LINE__);//index 大部分是0
    for (i = index; i < BOARD_COLUMN; i++)
    {
        int num = 0;
        for (j = 0; j < BOARD_ROW; j++)
        {
            if(icons[i*BOARD_ROW+j] == icon)
            {
                targetIcon = icon;
                sameCount++;
                num++;
				printf("targetIcon:%d, sameCount:%d, num:%d, %d\n", targetIcon, sameCount, num, __LINE__);
                break;
            }
        }
        if(num == 0){
			printf("num == 0, %d\n", __LINE__);
			break;
		}
    }

    //计算每一列中元素的个数，累乘
    for (i = index; i < BOARD_COLUMN; i++)
    {
        int colIconCount = 0;
        for (j = 0; j < BOARD_ROW; j++)
        {
            if(icons[i*BOARD_ROW+j] == icon){
				colIconCount++;
				printf("if icons[%d*%d+%d] == icon: colIconCount:%d, %d\n", i, BOARD_ROW, j, colIconCount, __LINE__);
			}
        }
        if(colIconCount == 0){
			printf("colIconCount == 0, %d\n", __LINE__);
			break;
		}

        iconNum *= colIconCount;
		printf("iconNum:%d, %d\n", iconNum, __LINE__);
    }

    if(sameCount > 1 && targetIcon < E_ICON_WILD)
    {
        int multi = gIconMulti[targetIcon-1][sameCount] * iconNum * wildMulti;//gIconMulti = 各symbol倍率表, targetIcon = 1~17, targetIcon-1=矩陣index, sameCount=有幾個相同icon, 所以gIconMulti[targetIcon-1][sameCount] = 某個symbol有幾個的倍率
        //multi = 倍率的數字* icon數量* 幾倍
		*gameMulti += multi;
        gPayIconScore[prizeType][targetIcon-1][sameCount] += multi;
		printf("*gameMulti:%d, gPayIconScore[prizeType][targetIcon-1][sameCount]:%d, %d\n", *gameMulti, gPayIconScore[prizeType][targetIcon-1][sameCount], __LINE__);
    }
}

int isWildIcon(int icon)
{
	return icon == gWildMulti[0][0] || icon == gWildMulti[1][0] || icon == gWildMulti[2][0];//gWildMulti[3][2] = {{12,1}, {13,2},{14,3}}; so gWildMulti[0~2][0] = 12 13 14
}

void changeIcon(int prizeType, int* icons, int icon, int *gameMulti)//icon = icons[m] = 第一列的icon
{
    int i;
    int wildMulti = 1;
    int iconsTemp[BOARD_ICON_NUM] = {0};
    
    memcpy(iconsTemp, icons, sizeof(iconsTemp));
    for (i = 0; i < BOARD_ICON_NUM; i++)//BOARD_ICON_NUM = 盘面图标数量 = 20
    {
        if(isWildIcon(icons[i])){//當icons[i] = wild(12) or wildx2(13) or wildx3(14)
            wildMulti *= gWildMulti[icons[i]-E_ICON_WILD][1];//E_ICON_WILD = 12, icons[i]-E_ICON_WILD = 0 1 2, gWildMulti[0~2][1] = 1 2 3, wildMulti=紀錄wild倍數variable
			printf("i:%d, icons[i]:%d, wildMulti:%d, gWildMulti[icons[i]-E_ICON_WILD][1]:%d, %d\n", i, icons[i], wildMulti, gWildMulti[icons[i]-E_ICON_WILD][1], __LINE__);
		}
        if(isWildIcon(iconsTemp[i])){
            iconsTemp[i] = icon;//icon = icons[m] = 第一列icon, 把它放到iconsTemp[i]
			printf("icon:%d, iconsTemp[i]:%d, %d\n", icon, iconsTemp[i], __LINE__);
		}
    }
    calArrangeIconNum(prizeType, iconsTemp, icon, gameMulti, wildMulti);
}

void normalIconsToBuff(int reelPos, int *icons)
{
    int i;
    if(reelPos >= E_ICON_PIC1 && reelPos < E_ICON_PIC5)//reelPos = 1~4 (reelPos 是 < E_ICON_PIC5 不是 <= )
    {
        for (i = 0; i < BOARD_ICON_NUM; i++)
        {
            if(icons[i] == reelPos+1){//reelPos = 2~5 剛好是其他四種動物
                icons[i] = E_ICON_PIC1;
				printf("icons[i]:%d, reelPos:%d\n", icons[i], reelPos);
			}
        }
    }
}

void freeIconsToBuff(int *icons, int buffCounter)
{
    int i, j;
    for (i = 0; i < BOARD_ICON_NUM; i++)//BOARD_ICON_NUM = 20
    {
        if(icons[i] == E_ICON_BUFF){
            icons[i] = E_ICON_PIC1;//盤面上所有金牛被換成一般牛
			printf("icons[i]:%d\n", icons[i]);
		}
    }

    for (i = 0; i < 4; i++)
    {
        if(buffCounter >= gBuffChangeIcons[i][1])//gBuffChangeIcons[4][2] = {{2,4}, {3,7}, {4,13}, {5,15}};<-金牛4個,ID=2是老鷹,金牛7個,ID=3是老虎以此類推
        {
            for (j = 0; j < BOARD_ICON_NUM; j++)//BOARD_ICON_NUM = 20
            {
                if(icons[j] == gBuffChangeIcons[i][0]){//當盤面上第j個icon為鷹虎狼鹿的其中一隻時
                    icons[j] = E_ICON_PIC1;//其他動物變一般牛
					printf("icons[i]:%d\n", icons[i]);
				}
            }
        }
    }
}

void cal_boardMulti(int prizeType, int *icons, int *gameMulti, int scatType)
{
    int i, m;
    int reIcon[BOARD_ICON_NUM] = {0};//BOARD_ICON_NUM = 20
    for (i = 0; i < BOARD_ICON_NUM; i++)//盤面上的icon全部跑一遍
    {
        if(icons[i] == E_ICON_WILD)
        {
            if(prizeType == E_PRIZETYPE_NORMAL)
                icons[i] = getRandomValue(gNormalWildWeight, 3, WILD_WEIGHT);//gNormalWildWeight[3][2] = {{12,60},{13,38},{14,2}}, WILD_WEIGHT = 100 (這邊是讓已經是WILD的icon有機率變成x1(就是不變)(60%),變成x2(38%)或x3(2%))
            else
                icons[i] = getRandomValue(gFreeWildWeight, 3, WILD_WEIGHT);//gFreeWildWeight[3][2] = {{12,0},{13,80},{14,20}};
			printf("icons[i]\n", icons[i]);
        }
    }

    for (m = 0; m < BOARD_ROW; m++)//BOARD_ROW=4  ,此for loop將wild icon換成第一列有的icon 
    {
		if(!reIcon[icons[m]-1])//m=0~3, icons[0~3] = 第一個row上的icon(1~17), 如果第一個row有重複的icon,就不會進changeIcon()
            changeIcon(prizeType, icons, icons[m], gameMulti);//將wild換成第一列的icon(因為中獎icon肯定是第一列存在的icon), prizeType = E_PRIZETYPE_NORMAL, icons[m]是第一列的icon, gameMulti是倍率矩陣
        reIcon[icons[m]-1] = 1;//第一列有可能有重複的icon,此array用來防止再計算一次,若已經計算過則reIcon[icons[m]-1] = 1(只要第一列有相同的第二個數字reIcon就變成1)(-1是因為array index和icon數字差了1(icon是1~17))
		#ifdef PRINT_BUFF
		printf("m:%d, icons[m]:%d ,reIcon[icons[m]-1]:d, %d\n", m, icons[m], reIcon[icons[m]-1], __LINE__);
		#endif
    }
    
    int scatNum = calIconNum(icons, scatType);//計算盤面上單一icon數量,第一次call的時候計算base game scatter
    if(scatNum >= 3)
    {
        *gameMulti += gIconMulti[scatType-1][scatNum];//gIconMulti是中獎倍率矩陣, scatType = E_ICON_NORMAL_SCAT = 15, scatType-1=14={15,0,0,75,375,750}這列
        gPayIconScore[prizeType][scatType-1][scatNum] += gIconMulti[scatType-1][scatNum];///////gPayIconScore這參數沒看到定義  有點奇怪
        #ifdef PRINT_BUFF
        printf("*gameMulti:%d, %d\n", *gameMulti, __LINE__);
        #endif		
    }
}

int getPrizeType(int *icons)
{
    if(calIconNum(icons, E_ICON_NORMAL_SCAT) >= gFreeGameInfo[1][0])//E_ICON_NORMAL_SCAT = base game scatter, gFreeGameInfo[FREEGAME_TYPE][2] = {{2,5},{3,8},{4,15},{5,20}},so gFreeGameInfo[1][0] = 3 (盤面上3個SC就觸發free game)
        return E_PRIZETYPE_FREEGAME;
    else
        return E_PRIZETYPE_NORMAL;
}
																	//				 gFreeReelWeight[BASE_REEL_NUM][2] = {{0,9000},{1,150},{2,250},{3,250},{4,100},{5,100},{6,100},{7,50}}			    
int getRandomValue(const int (*weight)[2], int len, int weightValue)//(*weight)[2] = gBaseReelWeight[BASE_REEL_NUM+4][2] = {{0,8995},{1,264},{2,234},{3,172},{4,80},{5,80},{6,80},{7,130},{8,20},{9,20},{10,20},{11,5}}
{																	//len=12, weightValue = REEL_WEIGHT = 10000; free game 的weightValue = WILD_WEIGHT = 100
    int i;
    int ran = GetRand(weightValue)+1;//ran是從 10000裡面取隨機值當作這回合權重, GetRand最後跑去RngGetTempRandom()取隨機值    GetRand() = RngGetTempRandom() in rng.c
    for(i = len-1; i >= 0; i--)
    {
        printf("ran:%d, weight[%d][0]:%d, weight[%d][1]:%d\n", ran, i, weight[i][0], i, weight[i][1]);
		if(ran <= weight[i][1]){//base game:因為ran是在0~10000取隨機值,所以要<264不太容易    ||free game:ran在0~100取值
			printf("in if ran:%d, weight[%d][0]:%d\n", ran, i, weight[i][0]);
			return weight[i][0];//base game:這邊是return0~11								||free game:
		}	
        ran -= weight[i][1];
    }
    return 0;
}

/*
print getRandomValue in base game
ran:1859, weight[11][0]:11, weight[11][1]:5
ran:1839, weight[10][0]:10, weight[10][1]:20
ran:1819, weight[9][0]:9, weight[9][1]:20
ran:1799, weight[8][0]:8, weight[8][1]:20
ran:1669, weight[7][0]:7, weight[7][1]:130
ran:1589, weight[6][0]:6, weight[6][1]:80
ran:1509, weight[5][0]:5, weight[5][1]:80
ran:1429, weight[4][0]:4, weight[4][1]:80
ran:1257, weight[3][0]:3, weight[3][1]:172
ran:1023, weight[2][0]:2, weight[2][1]:234
ran:759, weight[1][0]:1, weight[1][1]:264

print getRandomValue in free game
ran:79, weight[11][0]:12, weight[11][1]:10
ran:69, weight[10][0]:10, weight[10][1]:10
ran:59, weight[9][0]:8, weight[9][1]:10
ran:49, weight[8][0]:5, weight[8][1]:10
ran:39, weight[7][0]:2, weight[7][1]:10
ran:34, weight[6][0]:9, weight[6][1]:5
ran:29, weight[5][0]:7, weight[5][1]:5
ran:24, weight[4][0]:4, weight[4][1]:5
ran:19, weight[3][0]:1, weight[3][1]:5
ran:9, weight[2][0]:11, weight[2][1]:10
hitTarget = 5, so return weight[1][0] = 6(因為它是找到weight[2][0] = 11的下一個,但不會print出來)
*/