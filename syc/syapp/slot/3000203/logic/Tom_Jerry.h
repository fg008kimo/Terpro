#ifndef FISHERMAN_H_
#define FISHERMAN_H_

#include "../data.h"
#include "../utils.h"

//生成初始盘面
void gen_board(const uint8_t (*rollerIcon)[ROLLER_ICON_LEN], const uint16_t *rollerIconNum, const uint8_t (*activatedRollerIcon)[ROLLER_ICON_LEN], 
    const uint16_t *activatedRollerIconNum, int *icons, int *isActivated);

//计算某个图标的个数
int calIconNum(int *icons, int icon);

//计算盘面的得分，并记录中奖线的信息
int cal_boardMulti(s16 lineNum, int *icons, int (*lineInfos)[3], int *awardLineNum, int* wildColMulti);

//处理MAXX免费游戏玩法，将列转为激活状态
void handldMaxxMode(int *icons, int *isActivated);

//获取游戏类型
int getPrizeType(int *icons);

//选择AB权重的版本号
int selectWeight(s8 uid);

//难度调控
void difficulty_control(s16 lineNum, int diff, int upMulti, int downMulti, const double (*targetRTP)[3], int isSelect_A, s32 totalMulti, int *loopTime);

#endif