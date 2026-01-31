#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>
#include <signal.h>
#include <pthread.h>

#include "server.h"
#include "algMammon.h"
#include "Alg2.h"
#include "net.h"
#include "Test/TestMain.h"

#ifdef __LINUX__
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h> 
#include <sys/epoll.h>
#endif

void handle_mammon_init(struct connect_client *client, cJSON *dataJson)
{
    STR_AlgInit val;
	STR_AlgInitResult ret;

	char errBuf[STR_LEN_128] = {0};
	char *json_out = NULL;
	cJSON *itemJson = NULL, *root = NULL, *opRet = NULL;

	memset(&val, 0, sizeof(val));
	itemJson = cJSON_GetObjectItem(dataJson, "tableId");
	if (itemJson != NULL)
		val.tableId = itemJson->valueint;
	else
	{
		strcpy(errBuf, "tableId is NULL");
		goto errorDone;
	}

	memset(&ret, 0, sizeof(ret));

	//////////////////////////////////////////////////////////////////////////
	
	alg_init(&val, &ret);

	//////////////////////////////////////////////////////////////////////////
	
	root = cJSON_CreateObject();
    if (root == NULL)
    {
        strcpy(errBuf, "Create JSON object error[1].");
        goto errorDone;
    }

	opRet = cJSON_CreateObject();
    if (opRet == NULL)
    {
        strcpy(errBuf, "Create JSON object error[2].");
        goto errorDone;
    }	

	cJSON_AddNumberToObject(root, "Result", 0);
	cJSON_AddNumberToObject(root, "operation", mammonInit);


	cJSON_AddNumberToObject(opRet, "algVersion", ret.algVerNum);
	cJSON_AddNumberToObject(opRet, "result", ret.result);

	cJSON_AddItemToObject(root, "opRet", opRet);

    json_out = cJSON_PrintUnformatted(root);
    if (json_out != NULL)
        net_write(client->sock, json_out, strlen((const char*)json_out));
    else
    {
        strcpy(errBuf, "Json string format error");
    }	

errorDone:
	if (root != NULL)
		cJSON_Delete(root);

	alg_free(json_out); 

	if (strlen(errBuf) > 0)
	{
		char buf[STR_LEN_256] = {0};
		LOGE("net_error handle_mammon_init(): [%s]", errBuf);
		snprintf(buf, sizeof(buf) - 1, "{\"Result\":-1,\"FailReason\":\"%s\",\"operation\":%d}", errBuf, mammonInit);

		net_write_error(client->sock, buf);
	}

	return;	
}

void handle_mammon_reset(struct connect_client *client, cJSON *dataJson)
{
    STR_AlgReset val;
	STR_AlgResetResult ret;
	STR_AlgSetting val_setting;
	char errBuf[STR_LEN_128] = {0};
	char *json_out = NULL;
	cJSON *itemJson = NULL, *root = NULL, *opRet = NULL;

	memset(&val, 0, sizeof(val));
	itemJson = cJSON_GetObjectItem(dataJson, "rate");
	if (itemJson != NULL)
		val.coin_rate = itemJson->valueint;
	else
	{
		strcpy(errBuf, "coinRate is NULL");
		goto errorDone;
	}    

	itemJson = cJSON_GetObjectItem(dataJson, "begin_table_id");
	if (itemJson != NULL)
		val.begin_table_id = itemJson->valueint;
	else
	{
		strcpy(errBuf, "begin_table_id id is NULL");
		goto errorDone;
	}
	
	itemJson = cJSON_GetObjectItem(dataJson, "end_table_id");
	if (itemJson != NULL)
		val.end_table_id = itemJson->valueint;
	else
	{
		strcpy(errBuf, "end_table_id id is NULL");
		goto errorDone;
	}

	memset(&ret, 0, sizeof(ret));

	//////////////////////////////////////////////////////////////////////////

	alg_reset(&val, &ret);

	//////////////////////////////////////////////////////////////////////////


	root = cJSON_CreateObject();
    if (root == NULL)
    {
        strcpy(errBuf, "Create JSON object error[1].");
        goto errorDone;
    }

	opRet = cJSON_CreateObject();
    if (opRet == NULL)
    {
        strcpy(errBuf, "Create JSON object error[2].");
        goto errorDone;
    }	

	cJSON_AddNumberToObject(root, "Result", 0);
	cJSON_AddNumberToObject(root, "operation", mammonReset);


	cJSON_AddNumberToObject(opRet, "result", ret.result);

	cJSON_AddItemToObject(root, "opRet", opRet);

    json_out = cJSON_PrintUnformatted(root);
    if (json_out != NULL)
        net_write(client->sock, json_out, strlen((const char*)json_out));
    else
    {
        strcpy(errBuf, "Json string format error");
    }	

errorDone:
	if (root != NULL)
		cJSON_Delete(root);

	alg_free(json_out); 

	if (strlen(errBuf) > 0)
	{
		char buf[STR_LEN_256] = {0};
		LOGE("net_error handle_mammon_reset(): [%s]", errBuf);
		snprintf(buf, sizeof(buf) - 1, "{\"Result\":-1,\"FailReason\":\"%s\",\"operation\":%d}", errBuf, mammonReset);

		net_write_error(client->sock, buf);
	}

	return;	    
}

void handle_mammon_kill_fish(struct connect_client *client, cJSON *dataJson)
{
	STR_AlgKillFish val;
	STR_AlgKillFishResult ret;

	int i, j;
	char errBuf[STR_LEN_128] = {0};
	char *json_out = NULL;
	cJSON *root = NULL, *opRet = NULL, *itemJson = NULL, *arrJson = NULL, *arrJson2 = NULL;

	// {
	// 	json_out = cJSON_PrintUnformatted(dataJson);
	// 	LOGI("mykill_1:%s", json_out);
	// 	alg_free(json_out); 
	// }

	itemJson = cJSON_GetObjectItem(dataJson, "times");
	if (itemJson != NULL)
		val.times = itemJson->valueint;
	else
	{
		strcpy(errBuf, "times is NULL");
		goto errorDone;
	}	

	itemJson = cJSON_GetObjectItem(dataJson, "lineBet");
	if (itemJson != NULL)
		val.lineBet = (s64)itemJson->valuedouble;
	else
	{
		strcpy(errBuf, "lineBet is NULL");
		goto errorDone;
	}

	itemJson = cJSON_GetObjectItem(dataJson, "lineNum");
	if (itemJson != NULL)
		val.lineNum = (s16)itemJson->valueint;
	else
	{
		strcpy(errBuf, "lineNum is NULL");
		goto errorDone;
	}    

	itemJson = cJSON_GetObjectItem(dataJson, "tableId");
	if (itemJson != NULL)
		val.tableId = itemJson->valueint;
	else
	{
		strcpy(errBuf, "tableId is NULL");
		goto errorDone;
	}

	itemJson = cJSON_GetObjectItem(dataJson, "uid");
	if (itemJson != NULL)
	{
		char *uid = itemJson->valuestring;
		val.uid = uid[strlen(uid)-1] - '0';
	}
	else
	{
		strcpy(errBuf, "uid is NULL");
		goto errorDone;
	}

	itemJson = cJSON_GetObjectItem(dataJson, "blackDiff");
	if (itemJson != NULL)
		val.blackDiff = (s8)itemJson->valueint;
	else
	{
		val.blackDiff = -100;
	}

	itemJson = cJSON_GetObjectItem(dataJson, "playerDiff");
	if (itemJson != NULL)
		val.playerDiff = itemJson->valueint;
	else
	{
		val.playerDiff = -100;
	}

	itemJson = cJSON_GetObjectItem(dataJson, "blackDiffType");
	if (itemJson != NULL)
		val.blackDiffType = (s8)itemJson->valueint;
	else
	{
		val.blackDiffType = 5;
	}

	itemJson = cJSON_GetObjectItem(dataJson, "buyFreegameScore");
	if (itemJson != NULL)
		val.buyFreegameScore = (s64)itemJson->valuedouble;
	else
	{
		val.buyFreegameScore = 0;
	}

	//////////////////////////////////////////////////////////////////////////

	alg_kill_fish(&val, &ret);

	//////////////////////////////////////////////////////////////////////////

	root = cJSON_CreateObject();
	if (root == NULL)
	{
		strcpy(errBuf, "Create JSON object error[1].");
		goto errorDone;
	}

	opRet = cJSON_CreateObject();
	if (opRet == NULL)
	{
		strcpy(errBuf, "Create JSON object error[2].");
		goto errorDone;
	}	

	cJSON_AddNumberToObject(root, "Result", 0);
	cJSON_AddNumberToObject(root, "operation", mammonKillFish);

    cJSON_AddNumberToObject(opRet, "prizeType", ret.prizeType);
	cJSON_AddNumberToObject(opRet, "totalMulti", ret.totalMulti);

	cJSON_AddItemToObject(opRet, "normalIconsAgo", cJSON_CreateIntArray((const int*)ret.normalIconsAgo, BOARD_ICON_NUM));
	cJSON_AddItemToObject(opRet, "normalIcons", cJSON_CreateIntArray((const int*)ret.normalIcons, BOARD_ICON_NUM));
	cJSON_AddItemToObject(opRet, "normalWildMultis", cJSON_CreateIntArray((const int*)ret.normalWildMultis, BOARD_COLUMN));
	cJSON_AddNumberToObject(opRet, "normalMulti", ret.normalMulti);

	if(ret.normalLineNum > 0)
	{
		arrJson = cJSON_CreateArray();
		for(i = 0; i < ret.normalLineNum; i++)
			cJSON_AddItemToArray(arrJson, cJSON_CreateIntArray((const int*)(ret.normalLineInfos[i]), 3));
		cJSON_AddItemToObject(opRet, "normalLineInfos", arrJson);
	}
	if(ret.prizeType == E_PRIZETYPE_FREEGAME)
	{
		arrJson = cJSON_CreateArray();
		for(i = 0; i < ret.freegameNum; i++)
			cJSON_AddItemToArray(arrJson, cJSON_CreateIntArray((const int*)(ret.freegameIconsAgo[i]), BOARD_ICON_NUM));
		cJSON_AddItemToObject(opRet, "freegameIconsAgo", arrJson);
		arrJson = cJSON_CreateArray();
		for(i = 0; i < ret.freegameNum; i++)
			cJSON_AddItemToArray(arrJson, cJSON_CreateIntArray((const int*)(ret.freegameIcons[i]), BOARD_ICON_NUM));
		cJSON_AddItemToObject(opRet, "freegameIcons", arrJson);
		arrJson = cJSON_CreateArray();
		for(i = 0; i < ret.freegameNum; i++)
			cJSON_AddItemToArray(arrJson, cJSON_CreateIntArray((const int*)(ret.freeWildMultis[i]), BOARD_COLUMN));
		cJSON_AddItemToObject(opRet, "freeWildMultis", arrJson);
		arrJson = cJSON_CreateArray();
		cJSON_AddItemToObject(opRet, "freegameMultis", cJSON_CreateIntArray((const int*)ret.freegameMultis, ret.freegameNum));
		arrJson = cJSON_CreateArray();
		for(i = 0; i < ret.freegameNum; i++)
		{
			arrJson2 = cJSON_CreateArray();
			for(j = 0; j < ret.freegameLineNum[i]; j++)
				cJSON_AddItemToArray(arrJson2, cJSON_CreateIntArray((const int*)(ret.freegameLineInfos[i][j]), 3));
			cJSON_AddItemToArray(arrJson, arrJson2);
		}
		cJSON_AddItemToObject(opRet, "freegameLineInfos", arrJson);
	}

    cJSON_AddNumberToObject(opRet, "bet", val.lineBet);
    cJSON_AddNumberToObject(opRet, "betLineNum", val.lineNum);

	cJSON_AddItemToObject(root, "opRet", opRet);

	json_out = cJSON_PrintUnformatted(root);
	if (json_out != NULL)
		net_write(client->sock, json_out, strlen((const char*)json_out));
	else
	{
		strcpy(errBuf, "Json string format error");
	}	
	// LOGI("mykill_2:%s", json_out);

errorDone:
	if (root != NULL)
		cJSON_Delete(root);

	alg_free(json_out); 

	if (strlen(errBuf) > 0)
	{
		char buf[STR_LEN_256] = {0};
		LOGE("net_error handle_mammon_kill_fish(): [%s]", errBuf);
		snprintf(buf, sizeof(buf) - 1, "{\"Result\":-1,\"FailReason\":\"%s\",\"operation\":%d}", errBuf, mammonKillFish);

		net_write_error(client->sock, buf);
	}

	return; 	
}

void handle_mammon_revise(struct connect_client *client, cJSON *dataJson)
{
	STR_AlgInOutRevise val;
	STR_AlgInOutReviseResult ret;
	char errBuf[STR_LEN_128] = {0};
	char *json_out = NULL;
	cJSON *itemJson = NULL, *root = NULL, *opRet = NULL;

	memset(&val, 0, sizeof(val));
	itemJson = cJSON_GetObjectItem(dataJson, "inScore");
	if (itemJson != NULL)
		val.inScore = (s64)itemJson->valuedouble;
	else
	{
		strcpy(errBuf, "inScore is NULL");
		goto errorDone;
	}

	itemJson = cJSON_GetObjectItem(dataJson, "outScore");
	if (itemJson != NULL)
		val.outScore = (s64)itemJson->valuedouble;
	else
	{
		strcpy(errBuf, "outScore is NULL");
		goto errorDone;
	}

	itemJson = cJSON_GetObjectItem(dataJson, "times");
	if (itemJson != NULL)
		val.times = itemJson->valueint;
	else
	{
		strcpy(errBuf, "times is NULL");
		goto errorDone;
	}	

	itemJson = cJSON_GetObjectItem(dataJson, "tableId");
	if (itemJson != NULL)
		val.tableId = itemJson->valueint;
	else
	{
		strcpy(errBuf, "tableId is NULL");
		goto errorDone;
	}

	memset(&ret, 0, sizeof(ret));

	//////////////////////////////////////////////////////////////////////////

	alg_in_out_revise(&val, &ret);

	//////////////////////////////////////////////////////////////////////////

	root = cJSON_CreateObject();
	if (root == NULL)
	{
		strcpy(errBuf, "Create JSON object error[1].");
		goto errorDone;
	}

	opRet = cJSON_CreateObject();
	if (opRet == NULL)
	{
		strcpy(errBuf, "Create JSON object error[2].");
		goto errorDone;
	}	

	cJSON_AddNumberToObject(root, "Result", 0);
	cJSON_AddNumberToObject(root, "operation", mammonRevise);


	cJSON_AddNumberToObject(opRet, "result", ret.reviseResult);

	cJSON_AddItemToObject(root, "opRet", opRet);

	json_out = cJSON_PrintUnformatted(root);
	if (json_out != NULL)
		net_write(client->sock, json_out, strlen((const char*)json_out));
	else
	{
		strcpy(errBuf, "Json string format error");
	}	

errorDone:
	if (root != NULL)
		cJSON_Delete(root);

	alg_free(json_out); 

	if (strlen(errBuf) > 0)
	{
		char buf[STR_LEN_256] = {0};
		LOGE("net_error handle_mammon_revise(): [%s]", errBuf);
		snprintf(buf, sizeof(buf) - 1, "{\"Result\":-1,\"FailReason\":\"%s\",\"operation\":%d}", errBuf, mammonRevise);

		net_write_error(client->sock, buf);
	}

	return; 	
}

void handle_mammon_get_data(struct connect_client *client, cJSON *dataJson)
{
	STR_AlgTestData val;
	STR_AlgTestDataResult ret;
	char errBuf[STR_LEN_128] = {0};
	char *json_out = NULL;
	cJSON *itemJson = NULL, *root = NULL, *opRet = NULL;

	memset(&val, 0, sizeof(val));
	itemJson = cJSON_GetObjectItem(dataJson, "tableId");
	if (itemJson != NULL)
		val.tableId = itemJson->valueint;
	else
	{
		strcpy(errBuf, "table id is NULL");
		goto errorDone;
	}

	memset(&ret, 0, sizeof(ret));

	//////////////////////////////////////////////////////////////////////////

	alg_get_alg_data(&val, &ret);

	//////////////////////////////////////////////////////////////////////////

	root = cJSON_CreateObject();
	if (root == NULL)
	{
		strcpy(errBuf, "Create JSON object error[1].");
		goto errorDone;
	}

	opRet = cJSON_CreateObject();
	if (opRet == NULL)
	{
		strcpy(errBuf, "Create JSON object error[2].");
		goto errorDone;
	}	

	cJSON_AddNumberToObject(root, "Result", 0);
	cJSON_AddNumberToObject(root, "operation", mammonGetData);

    cJSON_AddNumberToObject(opRet, "coinRate", ret.coin_rate);
    cJSON_AddNumberToObject(opRet, "maxSend", ret.max_send);
    cJSON_AddNumberToObject(opRet, "real_diff", ret.real_diff);
	cJSON_AddNumberToObject(opRet, "random", ret.random);

    cJSON_AddNumberToObject(opRet, "rejustIn", ret.gRejustIn);
	cJSON_AddNumberToObject(opRet, "rejustOut", ret.gRejustOut);

	cJSON_AddItemToObject(root, "opRet", opRet);

	json_out = cJSON_PrintUnformatted(root);
	if (json_out != NULL)
	{
		net_write(client->sock, json_out, strlen((const char*)json_out));
		LOGI("get_alg_data [%d]=%s", val.tableId, json_out);
	}
	else
	{
		strcpy(errBuf, "Json string format error");
	}	


errorDone:
	if (root != NULL)
		cJSON_Delete(root);

	alg_free(json_out); 

	if (strlen(errBuf) > 0)
	{
		char buf[STR_LEN_256] = {0};
		LOGE("net_error handle_mammon_get_data(): [%s]", errBuf);
		snprintf(buf, sizeof(buf) - 1, "{\"Result\":-1,\"FailReason\":\"%s\",\"operation\":%d}", errBuf, mammonGetData);

		net_write_error(client->sock, buf);
	}

	return; 	
}


void handle_mammon_set_diff(struct connect_client *client, cJSON *dataJson)
{
	STR_AlgSetDiff val;
	STR_AlgSetDiffResult ret;
	STR_AlgSetting val_setting;
	char errBuf[STR_LEN_128] = {0};
	char *json_out = NULL;
	cJSON *itemJson = NULL, *root = NULL, *opRet = NULL;

	memset(&val, 0, sizeof(val));
	itemJson = cJSON_GetObjectItem(dataJson, "diff");
	if (itemJson != NULL)
		val.diff = itemJson->valueint;
	else
	{
		strcpy(errBuf, "diff is NULL");
		goto errorDone;
	}
	
	itemJson = cJSON_GetObjectItem(dataJson, "begin_table_id");
	if (itemJson != NULL)
		val.begin_table_id = itemJson->valueint;
	else
	{
		strcpy(errBuf, "begin_table_id id is NULL");
		goto errorDone;
	}
	
	itemJson = cJSON_GetObjectItem(dataJson, "end_table_id");
	if (itemJson != NULL)
		val.end_table_id = itemJson->valueint;
	else
	{
		strcpy(errBuf, "end_table_id id is NULL");
		goto errorDone;
	}

	memset(&ret, 0, sizeof(ret));

	//////////////////////////////////////////////////////////////////////////

	alg_set_diff(&val, &ret);

	//////////////////////////////////////////////////////////////////////////

	root = cJSON_CreateObject();
	if (root == NULL)
	{
		strcpy(errBuf, "Create JSON object error[1].");
		goto errorDone;
	}

	opRet = cJSON_CreateObject();
	if (opRet == NULL)
	{
		strcpy(errBuf, "Create JSON object error[2].");
		goto errorDone;
	}	

	cJSON_AddNumberToObject(root, "Result", 0);
	cJSON_AddNumberToObject(root, "operation", mammonSetDiff);

	cJSON_AddNumberToObject(opRet, "result", ret.result);

	cJSON_AddItemToObject(root, "opRet", opRet);

	json_out = cJSON_PrintUnformatted(root);
	if (json_out != NULL)
		net_write(client->sock, json_out, strlen((const char*)json_out));
	else
	{
		strcpy(errBuf, "Json string format error");
	}	

errorDone:
	if (root != NULL)
		cJSON_Delete(root);

	alg_free(json_out); 

	if (strlen(errBuf) > 0)
	{
		char buf[STR_LEN_256] = {0};
		LOGE("net_error handle_mammon_set_diff(): [%s]", errBuf);
		snprintf(buf, sizeof(buf) - 1, "{\"Result\":-1,\"FailReason\":\"%s\",\"operation\":%d}", errBuf, mammonSetDiff);

		net_write_error(client->sock, buf);
	}

	return; 	
}

void handle_mammon_set_maxsend(struct connect_client *client, cJSON *dataJson)
{
	STR_AlgSetMaxSend val;
	STR_AlgSetMaxSendResult ret;
	STR_AlgSetting val_setting;
	char errBuf[STR_LEN_128] = {0};
	char *json_out = NULL;
	cJSON *itemJson = NULL, *root = NULL, *opRet = NULL;

	memset(&val, 0, sizeof(val));

	itemJson = cJSON_GetObjectItem(dataJson, "level");
	if (itemJson != NULL)
		val.level = itemJson->valueint;
	else
	{
		strcpy(errBuf, "level is NULL");
		goto errorDone;
	}    
	
	itemJson = cJSON_GetObjectItem(dataJson, "begin_table_id");
	if (itemJson != NULL)
		val.begin_table_id = itemJson->valueint;
	else
	{
		strcpy(errBuf, "begin_table_id id is NULL");
		goto errorDone;
	}
	
	itemJson = cJSON_GetObjectItem(dataJson, "end_table_id");
	if (itemJson != NULL)
		val.end_table_id = itemJson->valueint;
	else
	{
		strcpy(errBuf, "end_table_id id is NULL");
		goto errorDone;
	}

	memset(&ret, 0, sizeof(ret));

	//////////////////////////////////////////////////////////////////////////

	alg_set_max_send(&val, &ret);

	//////////////////////////////////////////////////////////////////////////

	root = cJSON_CreateObject();
	if (root == NULL)
	{
		strcpy(errBuf, "Create JSON object error[1].");
		goto errorDone;
	}

	opRet = cJSON_CreateObject();
	if (opRet == NULL)
	{
		strcpy(errBuf, "Create JSON object error[2].");
		goto errorDone;
	}	

	cJSON_AddNumberToObject(root, "Result", 0);
	cJSON_AddNumberToObject(root, "operation", mammonSetMaxSend);

	cJSON_AddNumberToObject(opRet, "result", ret.result);

	cJSON_AddItemToObject(root, "opRet", opRet);

	json_out = cJSON_PrintUnformatted(root);
	if (json_out != NULL)
		net_write(client->sock, json_out, strlen((const char*)json_out));
	else
	{
		strcpy(errBuf, "Json string format error");
	}	

errorDone:
	if (root != NULL)
		cJSON_Delete(root);

	alg_free(json_out); 

	if (strlen(errBuf) > 0)
	{
		char buf[STR_LEN_256] = {0};
		LOGE("net_error handle_mammon_set_maxsend(): [%s]", errBuf);
		snprintf(buf, sizeof(buf) - 1, "{\"Result\":-1,\"FailReason\":\"%s\",\"operation\":%d}", errBuf, mammonSetMaxSend);

		net_write_error(client->sock, buf);
	}

	return; 	
}


void handle_mammon_clear(struct connect_client *client, cJSON *dataJson)
{
	STR_ClearAllAlgData val;
	STR_ClearAllAlgDataResult ret;
	char errBuf[STR_LEN_128] = {0};
	char *json_out = NULL;
	cJSON *itemJson = NULL, *root = NULL, *opRet = NULL;   

	memset(&val, 0, sizeof(val));	
	itemJson = cJSON_GetObjectItem(dataJson, "begin_table_id");
	if (itemJson != NULL)
		val.begin_table_id = itemJson->valueint;
	else
	{
		strcpy(errBuf, "begin_table_id id is NULL");
		goto errorDone;
	}
	
	itemJson = cJSON_GetObjectItem(dataJson, "end_table_id");
	if (itemJson != NULL)
		val.end_table_id = itemJson->valueint;
	else
	{
		strcpy(errBuf, "end_table_id id is NULL");
		goto errorDone;
	}

	memset(&ret, 0, sizeof(ret));

	//////////////////////////////////////////////////////////////////////////

	alg_clear_data(&val, &ret);

	//////////////////////////////////////////////////////////////////////////

	root = cJSON_CreateObject();
	if (root == NULL)
	{
		strcpy(errBuf, "Create JSON object error[1].");
		goto errorDone;
	}

	opRet = cJSON_CreateObject();
	if (opRet == NULL)
	{
		strcpy(errBuf, "Create JSON object error[2].");
		goto errorDone;
	}	

	cJSON_AddNumberToObject(root, "Result", 0);
	cJSON_AddNumberToObject(root, "operation", mammonClear);


	cJSON_AddNumberToObject(opRet, "result", ret.result);

	cJSON_AddItemToObject(root, "opRet", opRet);

	json_out = cJSON_PrintUnformatted(root);
	if (json_out != NULL)
		net_write(client->sock, json_out, strlen((const char*)json_out));
	else
	{
		strcpy(errBuf, "Json string format error");
	}	

errorDone:
	if (root != NULL)
		cJSON_Delete(root);

	alg_free(json_out); 

	if (strlen(errBuf) > 0)
	{
		char buf[STR_LEN_256] = {0};
		LOGE("net_error handle_mammon_clear(): [%s]", errBuf);
		snprintf(buf, sizeof(buf) - 1, "{\"Result\":-1,\"FailReason\":\"%s\",\"operation\":%d}", errBuf, mammonClear);

		net_write_error(client->sock, buf);
	}

	return; 	
}


void handle_mammon_set_bibei_diff(struct connect_client *client, cJSON *dataJson)
{
	STR_AlgSetBiBeiDiff val;
	STR_AlgSetBiBeiDiffResult ret;
	char errBuf[STR_LEN_128] = {0};
	char *json_out = NULL;
	cJSON *itemJson = NULL, *root = NULL, *opRet = NULL;

	memset(&val, 0, sizeof(val));
	itemJson = cJSON_GetObjectItem(dataJson, "diff");
	if (itemJson != NULL)
		val.diff = itemJson->valueint;
	else
	{
		strcpy(errBuf, "diff is NULL");
		goto errorDone;
	}
	
	itemJson = cJSON_GetObjectItem(dataJson, "tableId");
	if (itemJson != NULL)
		val.tableId = itemJson->valueint;
	else
	{
		strcpy(errBuf, "table id is NULL");
		goto errorDone;
	}

	memset(&ret, 0, sizeof(ret));

	root = cJSON_CreateObject();
	if (root == NULL)
	{
		strcpy(errBuf, "Create JSON object error[1].");
		goto errorDone;
	}

	opRet = cJSON_CreateObject();
	if (opRet == NULL)
	{
		strcpy(errBuf, "Create JSON object error[2].");
		goto errorDone;
	}	

	cJSON_AddNumberToObject(root, "Result", 0);
	cJSON_AddNumberToObject(root, "operation", mammonSetBiBeiDiff);

	cJSON_AddNumberToObject(opRet, "result", ret.result);

	cJSON_AddItemToObject(root, "opRet", opRet);

	json_out = cJSON_PrintUnformatted(root);
	if (json_out != NULL)
		net_write(client->sock, json_out, strlen((const char*)json_out));
	else
	{
		strcpy(errBuf, "Json string format error");
	}	

errorDone:
	if (root != NULL)
		cJSON_Delete(root);

	alg_free(json_out); 

	if (strlen(errBuf) > 0)
	{
		char buf[STR_LEN_256] = {0};
		LOGE("net_error handle_mammon_set_bibei_diff(): [%s]", errBuf);
		snprintf(buf, sizeof(buf) - 1, "{\"Result\":-1,\"FailReason\":\"%s\",\"operation\":%d}", errBuf, mammonSetBiBeiDiff);

		net_write_error(client->sock, buf);
	}

	return; 	
}

void handle_mammon_get_bibei(struct connect_client *client, cJSON *dataJson)
{
	STR_AlgBiBei val;
	STR_AlgBiBeiResult ret;
	char errBuf[STR_LEN_128] = {0};
	char *json_out = NULL;
	cJSON *itemJson = NULL, *root = NULL, *opRet = NULL;

	memset(&val, 0, sizeof(val));
	itemJson = cJSON_GetObjectItem(dataJson, "bet");
	if (itemJson != NULL)
		val.bet = (s64)itemJson->valuedouble;
	else
	{
		strcpy(errBuf, "bet is NULL");
		goto errorDone;
	}

	itemJson = cJSON_GetObjectItem(dataJson, "times");
	if (itemJson != NULL)
		val.times = itemJson->valueint;
	else
	{
		strcpy(errBuf, "times is NULL");
		goto errorDone;
	}	

	itemJson = cJSON_GetObjectItem(dataJson, "multi");
	if (itemJson != NULL)
		val.multi = itemJson->valueint;
	else
	{
		strcpy(errBuf, "multi is NULL");
		goto errorDone;
	}
    
	itemJson = cJSON_GetObjectItem(dataJson, "tableId");
	if (itemJson != NULL)
		val.tableId = itemJson->valueint;
	else
	{
		strcpy(errBuf, "table id is NULL");
		goto errorDone;
	}

	memset(&ret, 0, sizeof(ret));

	//////////////////////////////////////////////////////////////////////////

	alg_bi_bei(&val, &ret);

	//////////////////////////////////////////////////////////////////////////

	root = cJSON_CreateObject();
	if (root == NULL)
	{
		strcpy(errBuf, "Create JSON object error[1].");
		goto errorDone;
	}

	opRet = cJSON_CreateObject();
	if (opRet == NULL)
	{
		strcpy(errBuf, "Create JSON object error[2].");
		goto errorDone;
	}	

	cJSON_AddNumberToObject(root, "Result", 0);
	cJSON_AddNumberToObject(root, "operation", mammonBiBei);

	cJSON_AddNumberToObject(opRet, "result", ret.result);

	cJSON_AddItemToObject(root, "opRet", opRet);

	json_out = cJSON_PrintUnformatted(root);
	if (json_out != NULL)
		net_write(client->sock, json_out, strlen((const char*)json_out));
	else
	{
		strcpy(errBuf, "Json string format error");
	}	

errorDone:
	if (root != NULL)
		cJSON_Delete(root);

	alg_free(json_out); 

	if (strlen(errBuf) > 0)
	{
		char buf[STR_LEN_256] = {0};
		LOGE("net_error handle_mammon_get_bibei(): [%s]", errBuf);
		snprintf(buf, sizeof(buf) - 1, "{\"Result\":-1,\"FailReason\":\"%s\",\"operation\":%d}", errBuf, mammonBiBei);

		net_write_error(client->sock, buf);
	}

	return; 	
}

void handle_mammon_set_jackpot_rate(struct connect_client *client, cJSON *dataJson)
{
	STR_AlgSetJackpotRate val;
	STR_AlgSetJackpotRateResult ret;
	char errBuf[STR_LEN_128] = {0};
	char *json_out = NULL;
	cJSON *itemJson = NULL, *root = NULL, *opRet = NULL;

	memset(&val, 0, sizeof(val));
	itemJson = cJSON_GetObjectItem(dataJson, "jackpot_rate");
	if (itemJson != NULL)
		val.jackpot_rate = itemJson->valueint;
	else
	{
		strcpy(errBuf, "jackpot_rate is NULL");
		goto errorDone;
	}

	itemJson = cJSON_GetObjectItem(dataJson, "idx");
	if (itemJson != NULL)
		val.idx = itemJson->valueint;
	else
	{
		strcpy(errBuf, "idx is NULL");
		goto errorDone;
	}

	memset(&ret, 0, sizeof(ret));

	//////////////////////////////////////////////////////////////////////////

	alg_set_jackpot_rate(&val, &ret);

	//////////////////////////////////////////////////////////////////////////

	root = cJSON_CreateObject();
	if (root == NULL)
	{
		strcpy(errBuf, "Create JSON object error[1].");
		goto errorDone;
	}

	opRet = cJSON_CreateObject();
	if (opRet == NULL)
	{
		strcpy(errBuf, "Create JSON object error[2].");
		goto errorDone;
	}	

	cJSON_AddNumberToObject(root, "Result", 0);
	cJSON_AddNumberToObject(root, "operation", mammonSetJackpotRate);

	cJSON_AddNumberToObject(opRet, "result", ret.result);

	cJSON_AddItemToObject(root, "opRet", opRet);

	json_out = cJSON_PrintUnformatted(root);
	if (json_out != NULL)
		net_write(client->sock, json_out, strlen((const char*)json_out));
	else
	{
		strcpy(errBuf, "Json string format error");
	}	

errorDone:
	if (root != NULL)
		cJSON_Delete(root);

	alg_free(json_out); 

	if (strlen(errBuf) > 0)
	{
		char buf[STR_LEN_256] = {0};
		LOGE("net_error handle_mammon_set_jackpot_rate(): [%s]", errBuf);
		snprintf(buf, sizeof(buf) - 1, "{\"Result\":-1,\"FailReason\":\"%s\",\"operation\":%d}", errBuf, mammonSetJackpotRate);

		net_write_error(client->sock, buf);
	}

	return;	
}

void handle_mammon_alg_setting(struct connect_client *client, cJSON *dataJson)
{
	STR_AlgSetting val;
	STR_AlgSettingResult ret;
	char errBuf[STR_LEN_128] = {0};
	char *json_out = NULL;
	cJSON *itemJson = NULL, *root = NULL, *opRet = NULL;

	memset(&val, 0, sizeof(val));
	itemJson = cJSON_GetObjectItem(dataJson, "diff");
	if (itemJson != NULL)
		val.diff = itemJson->valueint;
	else
	{
		strcpy(errBuf, "diff is NULL");
		goto errorDone;
	}

	itemJson = cJSON_GetObjectItem(dataJson, "max_send");
	if (itemJson != NULL)
		val.max_send = itemJson->valueint;
	else
	{
		strcpy(errBuf, "max_send is NULL");
		goto errorDone;
	}

	itemJson = cJSON_GetObjectItem(dataJson, "coin_rate");
	if (itemJson != NULL)
		val.coin_rate = itemJson->valueint;
	else
	{
		strcpy(errBuf, "coin_rate is NULL");
		goto errorDone;
	}	

	itemJson = cJSON_GetObjectItem(dataJson, "begin_table_id");
	if (itemJson != NULL)
		val.begin_table_id = itemJson->valueint;
	else
	{
		strcpy(errBuf, "begin_table_id is NULL");
		goto errorDone;
	}
	
	itemJson = cJSON_GetObjectItem(dataJson, "end_table_id");
	if (itemJson != NULL)
		val.end_table_id = itemJson->valueint;
	else
	{
		strcpy(errBuf, "end_table_id is NULL");
		goto errorDone;
	}

	memset(&ret, 0, sizeof(ret));

	//////////////////////////////////////////////////////////////////////////

	alg_setting(&val, &ret);

	//////////////////////////////////////////////////////////////////////////

	root = cJSON_CreateObject();
	if (root == NULL)
	{
		strcpy(errBuf, "Create JSON object error[1].");
		goto errorDone;
	}

	opRet = cJSON_CreateObject();
	if (opRet == NULL)
	{
		strcpy(errBuf, "Create JSON object error[2].");
		goto errorDone;
	}	

	cJSON_AddNumberToObject(root, "Result", 0);
	cJSON_AddNumberToObject(root, "operation", mammonAlgSetting);

	cJSON_AddNumberToObject(opRet, "result", ret.result);

	cJSON_AddItemToObject(root, "opRet", opRet);

	json_out = cJSON_PrintUnformatted(root);
	if (json_out != NULL)
		net_write(client->sock, json_out, strlen((const char*)json_out));
	else
	{
		strcpy(errBuf, "Json string format error");
	}	

errorDone:
	if (root != NULL)
		cJSON_Delete(root);

	alg_free(json_out); 

	if (strlen(errBuf) > 0)
	{
		char buf[STR_LEN_256] = {0};
		LOGE("net_error handle_mammon_alg_setting(): [%s]", errBuf);
		snprintf(buf, sizeof(buf) - 1, "{\"Result\":-1,\"FailReason\":\"%s\",\"operation\":%d}", errBuf, mammonAlgSetting);

		net_write_error(client->sock, buf);
	}

	return;	
}

void handle_mammon_set_config(struct connect_client *client, cJSON *dataJson)
{
	LOGI("mammonSetConfig_1");
	STR_AlgSetConfig val;
	STR_AlgSetConfigResult ret;
	char errBuf[STR_LEN_128] = {0};
	char *json_out = NULL;
	cJSON *itemJson = NULL, *root = NULL, *opRet = NULL;

	memset(&val, 0, sizeof(val));
	itemJson = cJSON_GetObjectItem(dataJson, "content");
	if (itemJson != NULL)
	{
		val.content = itemJson->valuestring;
		LOGI("mammonSetConfig_2  content = %s", val.content);
	}
	else
	{
		strcpy(errBuf, "content is NULL");
		goto errorDone;
	}

	memset(&ret, 0, sizeof(ret));
	ret.config = NULL;

	//////////////////////////////////////////////////////////////////////////

	alg_set_config(&val, &ret);
	LOGI("mammonSetConfig_4");

	//////////////////////////////////////////////////////////////////////////

	if(ret.config == NULL)
	{
		strcpy(errBuf, "ret.config == NULL");
		goto errorDone;
	}

	root = cJSON_CreateObject();
	if (root == NULL)
	{
		strcpy(errBuf, "Create JSON object error[1].");
		goto errorDone;
	}

	opRet = cJSON_CreateObject();
	if (opRet == NULL)
	{
		strcpy(errBuf, "Create JSON object error[2].");
		goto errorDone;
	}	

	cJSON_AddNumberToObject(root, "Result", 0);
	cJSON_AddNumberToObject(root, "operation", mammonSetConfig);


	cJSON_AddNumberToObject(opRet, "result", ret.result);
	if(ret.config)
		cJSON_AddStringToObject(opRet, "config", ret.config);

	cJSON_AddItemToObject(root, "opRet", opRet);

	json_out = cJSON_PrintUnformatted(root);
	if (json_out != NULL)
	{
		net_write(client->sock, json_out, strlen((const char*)json_out));
        LOGI("mammonSetConfig_5 json_out = %s", json_out);
	}
	else
	{
		strcpy(errBuf, "Json string format error");
	}	

errorDone:
	if (root != NULL)
		cJSON_Delete(root);

	alg_free(ret.config);
	alg_free(json_out); 

	if (strlen(errBuf) > 0)
	{
		char buf[STR_LEN_256] = {0};
		LOGE("net_error handle_mammon_set_config(): [%s]", errBuf);
		snprintf(buf, sizeof(buf) - 1, "{\"Result\":-1,\"FailReason\":\"%s\",\"operation\":%d}", errBuf, mammonSetConfig);

		net_write_error(client->sock, buf);
	}

	return;	
}

void process_incomplete_data(struct connect_client *client)
{
    cJSON *jMsg = NULL, *gameJson = NULL, *opJson = NULL, *dataJson = NULL, *rpcJson = NULL, *msgJson;
    char *end = NULL;
    char *buf = client->incomplete_data;
    char data[STR_LEN_3072] = {0};	
    char game[STR_LEN_16] = {0};
	int opType = 0;
    
    if (IS_EMPTY(buf)) 
    {
        return;
    }

    LOGI("net func: %s, incomplete data: %s", __FUNCTION__, client->incomplete_data);
    do {
        jMsg = cJSON_ParseWithOpts(buf, (const char **)&end, 0);
    	if (jMsg != NULL)
    	{
            gameJson = cJSON_GetObjectItem(jMsg, "game");
            if (gameJson != NULL)
            {
                strncpy(game, gameJson->valuestring, sizeof(game) - 1);
                if (strcmp(game, ALG_GAME_MAMMON) == 0)
                {
                    opJson = cJSON_GetObjectItem(jMsg, "operation");
                    dataJson = cJSON_GetObjectItem(jMsg, "data"); 
                    if (((opJson != NULL) && (opJson->type == cJSON_Number)) && ((dataJson != NULL) && (dataJson->type == cJSON_Object)))
                    {
                        opType = opJson->valueint;

                        switch (opType)
                        {
                            case mammonKillFish:

                                handle_mammon_kill_fish(client, dataJson);

                                break;
                            case mammonInit:

                                handle_mammon_init(client, dataJson);

                                break; 
                            case mammonGetData:

                                handle_mammon_get_data(client, dataJson);

                                break;
                           case mammonReset:

                                handle_mammon_reset(client, dataJson);

                                break;
                           case mammonRevise:

                                handle_mammon_revise(client, dataJson);

                                break;
                            case mammonSetMaxSend:

                                handle_mammon_set_maxsend(client, dataJson);

                                break;
                            case mammonClear:

                                handle_mammon_clear(client, dataJson);

                                break;
                           case mammonSetDiff:

                                handle_mammon_set_diff(client, dataJson);

                                break;
                           case mammonSetBiBeiDiff:

                                handle_mammon_set_bibei_diff(client, dataJson);

                                break;
                            case mammonBiBei:

                                handle_mammon_get_bibei(client, dataJson);

                                break;   
                            case mammonAlgSetting:

                                handle_mammon_alg_setting(client, dataJson);

                                break;  

							case mammonSetJackpotRate:
								handle_mammon_set_jackpot_rate(client, dataJson);
								break;	
							case mammonSetConfig:
								handle_mammon_set_config(client, dataJson);
								break;	
                            default:
                                LOGE("net_error Mammon operation[%d] doesn't exist.", opType);

                                break;
                        }
                    }
                }
            }

            rpcJson = cJSON_GetObjectItem(jMsg, "RPCMethod");
            if (rpcJson != NULL)
            {
                switch (rpcJson->valueint)
                {
                    case rpc_login:
                        handle_login_msg(client, jMsg, ALG_MAMMON_VERSION);
                        break;
                    case rpc_hb:
                        //LOGE("net Receive hearbeat from [%s|%d]", client->addr, client->sock);
                        alg_heartbeat_msg(client);
                        client->last = time(NULL);
                        break;                    
                    default:
                        LOGE("net_error RPCMethod[%d] error", rpcJson->valueint);
                        break;
                }
            }

			//处理由算法自身发出协议请求的回应
			if (gameJson == NULL)
			{
				msgJson = cJSON_GetObjectItem(jMsg, "operation");
				if (msgJson != NULL)
				{
					client->last = time(NULL);
					dataJson = cJSON_GetObjectItem(jMsg, "opRet");
					switch (msgJson->valueint)
					{
					default:
						LOGE("net_error notice   2 error");
						break;
					}
				}

				msgJson = cJSON_GetObjectItem(jMsg, "MsgType");
				if (msgJson != NULL)
				{
					switch (msgJson->valueint)
					{
					case srv_rm_hbpong:
						client->last = time(NULL);
						/*if (client->sock == server_infos[0].sock)
							LOGE("net notice=%d   2 : %s", eHeartbeat, buf);*/
						break;
					default:
						LOGE("net_error notice   2 error : %s", buf);
						break;
					}
				}
			}

            cJSON_Delete(jMsg);
            buf = end;
            if (strlen(end) == 0)
                break;
    	}
    	else
    	{
    		LOGE("net_error Incomplete data [%s] is not JSON format", buf);
    		char *p = NULL;
    		p = strchr(buf, '}');
    		if (p != NULL)
    		{
            	strncpy(data, p + 1, sizeof(data) - 1);
            }
            else
            	strncpy(data, buf, sizeof(data) - 1);
		  
            memset(client->incomplete_data, 0, sizeof(client->incomplete_data));
            strncpy(client->incomplete_data, data, sizeof(client->incomplete_data) - 1);
            break;
    	} 
    }while (end != NULL);

    return;    
}


static void _mammon_packet(struct connect_client *client, char *buf)
{
    cJSON *jMsg = NULL, *gameJson = NULL, *opJson = NULL, *dataJson = NULL, *rpcJson = NULL, *msgJson = NULL;
    char *end = NULL;
    char game[STR_LEN_16] = {0};
	int opType = 0, flg = 0;
    
    if (IS_EMPTY(buf)) 
    {
        return;
    }

    do {
        jMsg = cJSON_ParseWithOpts(buf, (const char **)&end, 0);
    	if (jMsg != NULL)
    	{
            gameJson = cJSON_GetObjectItem(jMsg, "game");
            if (gameJson != NULL)
            {
                strncpy(game, gameJson->valuestring, sizeof(game) - 1);
                if (strcmp(game, ALG_GAME_MAMMON) == 0)

                {
                	client->last = time(NULL);
                    opJson = cJSON_GetObjectItem(jMsg, "operation");
                    dataJson = cJSON_GetObjectItem(jMsg, "data"); 
                    if (((opJson != NULL) && (opJson->type == cJSON_Number)) && ((dataJson != NULL) && (dataJson->type == cJSON_Object)))
                    {
                        opType = opJson->valueint;

                        switch (opType)
                        {
                            case mammonKillFish:

                                handle_mammon_kill_fish(client, dataJson);

                                break;
                            case mammonInit:

                                handle_mammon_init(client, dataJson);

                                break; 
                            case mammonGetData:

                                handle_mammon_get_data(client, dataJson);

                                break;
                           case mammonReset:

                                handle_mammon_reset(client, dataJson);

                                break;
                           case mammonRevise:

                                handle_mammon_revise(client, dataJson);

                                break;
                            case mammonSetMaxSend:

                                handle_mammon_set_maxsend(client, dataJson);

                                break;
                            case mammonClear:

                                handle_mammon_clear(client, dataJson);

                                break;
                           case mammonSetDiff:

                                handle_mammon_set_diff(client, dataJson);

                                break;
                           case mammonSetBiBeiDiff:

                                handle_mammon_set_bibei_diff(client, dataJson);

                                break;
                            case mammonBiBei:

                                handle_mammon_get_bibei(client, dataJson);

                                break;  
                            case mammonAlgSetting:

                                handle_mammon_alg_setting(client, dataJson);

                                break;  
							case mammonSetJackpotRate:
								handle_mammon_set_jackpot_rate(client, dataJson);
								break;
							case mammonSetConfig:
								handle_mammon_set_config(client, dataJson);
								break;
                            default:
                                LOGE("Mammon operation[%d] doesn't exist.", opType);

                                break;
                        }
                    }
                }
            }

            rpcJson = cJSON_GetObjectItem(jMsg, "RPCMethod");
            if (rpcJson != NULL)
            {
                switch (rpcJson->valueint)
                {
                    case rpc_login:
                        handle_login_msg(client, jMsg, ALG_MAMMON_VERSION);
                        break;
                    case rpc_hb:
                        //LOGE("net Receive hearbeat from [%s|%d]", client->addr, client->sock);
                        alg_heartbeat_msg(client);
                        client->last = time(NULL);
                        break;
                    default:
                        LOGE("net_error RPCMethod[%d] error", rpcJson->valueint);
                        break;
                }
            }

			//处理由算法自身发出协议请求的回应
			if (gameJson == NULL)
			{
				msgJson = cJSON_GetObjectItem(jMsg, "operation");
				if (msgJson != NULL)
				{
					client->last = time(NULL);
					dataJson = cJSON_GetObjectItem(jMsg, "opRet");
					switch (msgJson->valueint)
					{
					default:
						LOGE("net_error notice   2 error");
						break;
					}
				}

				msgJson = cJSON_GetObjectItem(jMsg, "MsgType");
				if (msgJson != NULL)
				{
					switch (msgJson->valueint)
					{
					case srv_rm_hbpong:
						client->last = time(NULL);
						//LOGI("net notice_2 heartbeat");
						break;
					default:
						LOGE("net_error notice   2 error : %s", buf);
						break;
					}
				}
			}

            cJSON_Delete(jMsg);
            buf = end;
            if (strlen(end) == 0)
                break;
    	}
    	else
    	{
    		LOGE("net_error Recieve message [%s] is not JSON format", buf);

            if (strlen(client->incomplete_data) == 0)
                flg = 1;

			if ((strlen(client->incomplete_data) + strlen(buf)) >= sizeof(client->incomplete_data))
			{
				memset(client->incomplete_data, 0, sizeof(client->incomplete_data));
				strncpy(client->incomplete_data, buf, sizeof(client->incomplete_data) - 1);
			}
			else
            	strcat(client->incomplete_data, buf);

            if (flg != 1)
                process_incomplete_data(client);			
            break;
    	} 
    }while (end != NULL);

    return;
}


static int _handle_mammon_msg(struct alg_db *hs, int sock, uint32_t events)

{
#ifdef __LINUX__
    struct connect_client *client = NULL;
	PACK_INFO* p_pack_info;
    int recvLen, targetLen;

    if (events & EPOLLIN)
    {
        HASH_FIND(hh_sock, hs->c_client, &sock, sizeof(int), client);
		if ((client != NULL) && (client->remove == 1))
		{
			LOGE("net  Client [%d|%s] error, may be close or exit, close.", sock, client->addr);
			return -1;
		}
		else if (client == NULL)
		{
			LOGE("net_error Client is null");
			return -1;
		}

		p_pack_info = get_pack_info(sock);
		while (1)
		{
			targetLen = get_pack_info_targetLen(sock);
			recvLen = net_read(sock, &p_pack_info->buf[p_pack_info->bufUsefulInfoLen], targetLen);
			if (recvLen < 0 || targetLen < 0 || recvLen > targetLen)
			{
				if (errno == EWOULDBLOCK)
					break;

				LOGE("net_error recvLen error_1 [%d] [%s]", errno, strerror(errno));
				LOGE("net_error recvLen error_2 [%d] [%d]", recvLen, targetLen);
				return -1;
			}

			p_pack_info->bufUsefulInfoLen += recvLen;
			p_pack_info->buf[p_pack_info->bufUsefulInfoLen] = '\0';
			if (recvLen < targetLen)
            {
                if (recvLen == 0)
                {
                    return -1;
                }
				break;
            }

			p_pack_info->state++;
			if (p_pack_info->state == ePackStateContentInComplete)
			{
				p_pack_info->headLen = parse_charArr_2_int(p_pack_info->buf);
				p_pack_info->headLen = ntohl(p_pack_info->headLen);
				if (p_pack_info->headLen < PACK_HEAD_ID_BYTES || p_pack_info->headLen >= STR_LEN_4096)
				{
					LOGE("net_error recv headLen is error [%d] [%d]", p_pack_info->headLen, parse_charArr_2_int(p_pack_info->buf));
					return -1;
				}
				p_pack_info->bufUsefulInfoLen = 0;
			}
			else if(p_pack_info->state == ePackStateComplete)
			{
				p_pack_info->headId = parse_charArr_2_int(p_pack_info->buf);
				p_pack_info->headId = ntohl(p_pack_info->headId);
				_mammon_packet(client, &p_pack_info->buf[PACK_HEAD_ID_BYTES]);
				clear_pack_info(sock);
			}
			else
			{
				LOGE("net_error state overflow");
				return -1;
			}
		}
    }
#endif

    return 1;
}


static void _mammon_main_loop()

{
	int fdcount = 0;
	int i = 0, j = 0;
	struct epoll_event ev, events[MAX_EVENTS];    
    struct connect_client *client = NULL, *tmp = NULL;

	if ((alg.epollfd = epoll_create(MAX_EVENTS)) == -1) {
		LOGE("net_error Error in epoll creating: %s", strerror(errno));
		return;
	} 

	memset(&ev, 0, sizeof(struct epoll_event));
	memset(&events, 0, sizeof(struct epoll_event)*MAX_EVENTS);
	ev.data.fd = alg.listen_sock;
	ev.events = EPOLLIN;
	if (epoll_ctl(alg.epollfd, EPOLL_CTL_ADD, alg.listen_sock, &ev) == -1) {
		LOGE("net_error in epoll initial registering: %s", strerror(errno));
		(void)close(alg.epollfd);
		alg.epollfd = 0;
		return;
	}

    while (1)
    {
        HASH_ITER(hh_sock, alg.c_client, client, tmp)
        {
            if (client->remove == 1)
            {
                LOGI("net Delete client [%s|%d].", client->addr, client->sock);
				close_client(client->sock, client);
            }
        }
        fdcount = epoll_wait(alg.epollfd, events, MAX_EVENTS, -1);
		switch(fdcount){
		case -1:
			if(errno != EINTR){
				LOGE("net_error in epoll waiting: %s.", strerror(errno));
			}
			break;
		case 0:
			break;
		default:
			for(i=0; i < fdcount; i++){
				if (events[i].data.fd == alg.listen_sock) {
					if (events[i].events & (EPOLLIN | EPOLLPRI)){
						if((ev.data.fd = net_socket_accept(&alg)) != -1){
							ev.events = EPOLLIN;
							if (epoll_ctl(alg.epollfd, EPOLL_CTL_ADD, ev.data.fd, &ev) == -1) {
								LOGE("net_error in epoll accepting: %s", strerror(errno));
							}
							init_pack_info(ev.data.fd);
						}                        
					}
				}
				else
				{
                    if (events[i].events & EPOLLIN)
    				{
                        if (_handle_mammon_msg(&alg, events[i].data.fd, events[i].events) == -1)

                        {
							close_client(events[i].data.fd, client);
                            events[i].data.fd = -1;
                        }
                        else
                        {
                            events[i].events = EPOLLOUT;
                            epoll_ctl(alg.epollfd, EPOLL_CTL_MOD, events[i].data.fd, &events[i]); 
                        }
    				}
                    else if (events[i].events & EPOLLOUT)
                    {
                        events[i].events = EPOLLIN;
                        epoll_ctl(alg.epollfd, EPOLL_CTL_MOD, events[i].data.fd, &events[i]);                     
                    }
				}
			}
            break;
		}
    }
}

//打印平台号
void print_platform()
{
	//todo
}

int main(int argc, char *argv[])
{
    int ret = 0;
	int sock = 0;
	int i = 0;
	pthread_t id = 0;
    char buf[256] = {0}, log_dir[64] = {0};
	STR_AlgInit val;
	STR_AlgInitResult result;

	signal(SIGPIPE, alg_sigpipe);

    //init
    alg_config_init(&alg, ALG_MAMMON_PORT, ALG_MAMMON_PASSWORD);    

    if (config_parse_args(&alg, argc, argv) != 0)
    {
        return 1;
    }    

	snprintf(log_dir, sizeof(log_dir) - 1, "log%d.conf", alg.port);
	FILE *fp = fopen(log_dir, "w+");
	fprintf(fp, "[rules]\n");
	fprintf(fp, "*.*	\"/home/syc/log/algMammon_%d.log\",    2MB * 1", alg.port);
	fclose(fp);

	if (dzlog_init(log_dir, "my_cat")) {
		printf("zlog init failed\n");
		return -1;
	}

    LOGI("##########################################################");
    
    LOGI("#\n#\n#      Current alg mammon server version is %u\n#\n#", get_alg_ver());


    LOGI("##########################################################");

#ifndef __DEBUG__
    alg_daemonise();
#endif

    LOGI("Initial password is [%s]", alg.password);

	print_platform();

	sock = net_socket_listen(alg.port);
    if (sock == -1)
    {
        return 1;
    }
    else
    {
		alg.listen_sock = sock;

        if (alg.port == ALG_MAMMON_PORT)
        {
            LOGI("Listening on defalut port %d ......\n", alg.port);
        }
        else
        {
            LOGI("Listening on port %d ......\n", alg.port);
        }

		close_all_pack_info();

		memset(&val, 0, sizeof(val));
		for (i = 0; i < 10000; i++)
		{
			val.tableId = i;
			alg_init(&val, &result);
		}

		handle_start_setting(alg.port);

        pthread_mutex_init(&mutex, NULL);    

        ret = alg_hb_thread_create(&id);
        if (ret == -1)
        {
            close(alg.listen_sock);
            pthread_mutex_destroy(&mutex);
            return 1;
        }
#ifdef __DEBUG__
		test_main();
#endif
		_mammon_main_loop();

        close(alg.listen_sock);

        pthread_mutex_destroy(&mutex);  
    }

    return 0;
}