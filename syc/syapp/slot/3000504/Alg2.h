#ifndef _IOALG_H2_
#define _IOALG_H2_

#include "algMammon.h"


#define TOTAL_JACKPOT_NUM 10

typedef struct{
	s32 diff; 
	s32 max_send;
	s32 coin_rate;
}SettingData;

int get_alg_ver(void);
void set_port(int new_port);

void alg_kill_fish(STR_AlgKillFish* algDataInput, STR_AlgKillFishResult* outAlgData);
void alg_init(STR_AlgInit* val, STR_AlgInitResult* ret);
void alg_get_alg_data(STR_AlgTestData* val, STR_AlgTestDataResult* ret);
void alg_reset(STR_AlgReset* val, STR_AlgResetResult* ret);
void alg_in_out_revise(STR_AlgInOutRevise* val, STR_AlgInOutReviseResult* ret);
void alg_set_max_send(STR_AlgSetMaxSend* val, STR_AlgSetMaxSendResult* ret);
void alg_clear_data(STR_ClearAllAlgData* val, STR_ClearAllAlgDataResult* ret);
void alg_set_diff(STR_AlgSetDiff* val, STR_AlgSetDiffResult* ret);
void alg_bi_bei(STR_AlgBiBei* val, STR_AlgBiBeiResult* ret);
void alg_set_jackpot_rate(STR_AlgSetJackpotRate* val, STR_AlgSetJackpotRateResult* ret);
void alg_setting(STR_AlgSetting* val, STR_AlgSettingResult* ret);
void alg_set_config(STR_AlgSetConfig* val, STR_AlgSetConfigResult* ret);

#endif
