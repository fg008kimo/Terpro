#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

#include "utils.h"
#include "tools/random/rng.h"


#ifdef __LINUX__
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "zlog.h"
#endif

//#define PRINT_RAND    	1

char *get_current_time(char *time_now)
{
#ifdef __LINUX__
    struct tm nowtime;
    struct timeval tv;
    gettimeofday(&tv, NULL);
    localtime_r(&tv.tv_sec,&nowtime);

    sprintf(time_now,"%d-%02d-%02d %02d:%02d:%02d.%03d",
        nowtime.tm_year+1900,
        nowtime.tm_mon+1,
        nowtime.tm_mday,
        nowtime.tm_hour,
        nowtime.tm_min,
        nowtime.tm_sec,
        (int)(tv.tv_usec/1000)
    );
    return time_now;
#else
	return NULL;
#endif
}

void *alg_malloc(size_t size)
{
	void *mem = malloc(size);
	return mem;
}

void *alg_calloc(size_t nmemb, size_t size)
{
	void *mem = calloc(nmemb, size);
	return mem;
}

void alg_free(void *mem)
{
	if (mem != NULL)
		free(mem);
}

void InitRandom(int algPort)
{
	RngInit(algPort);
}

double GetRandom()
{
	return RngGetRandom();
}

uint32_t GetRand(uint32_t maxValue)
{
	return  GetRand_2(0, maxValue);
}

uint32_t GetRand_2(uint32_t minValue,uint32_t maxValue)
{
	return RngGetRand(minValue, maxValue);
}

int RandSelect(const uint32_t* probArray, uint16_t probNum, uint32_t probSum)
{
	int i;
	uint32_t randNum = GetRand(probSum) + 1;//+1把[0,probSum)转为[1,probSum]  

	for (i = 0; i < probNum; i++)
    {
        #ifdef PRINT_RAND
		printf("randNum:%d, i:%d, probArray[i]:%d, %s\n", randNum, i, probArray[i], __FUNCTION__);
		#endif
		if (randNum <= probArray[i])
        {
            #ifdef PRINT_RAND
			printf("in if randNum:%d, i:%d, %s\n", randNum, i, __FUNCTION__);
			#endif
			return i;
        }
        randNum -= probArray[i];
    }

	LOGI("RandSelect error: probNum=%hd probSum=%d", probNum, probSum);
    return 0;
}

void generate_random_str(char *str, int length)
{
    int i = 0, flag = 0;
     
	srand((unsigned)time(NULL));
    for (i = 0; i < length; i ++)
    {
		flag = rand()%3;
		switch (flag)
		{
    		case 0:
    			str[i] = rand()%26 + 'a'; 
    			break;
    		case 1:
    			str[i] = rand()%26 + 'A'; 
    			break;
    		case 2:
    			str[i] = rand()%10 + '0'; 
    			break;
		}
    }
    printf("Random string: %s\n", str);
     
    return;
}

void print_cjson(cJSON* dataJson)
{
	char* str;

	if (dataJson == NULL)
		return;

	str = cJSON_PrintUnformatted(dataJson);
	LOGI("%s", str);
	alg_free(str);
}

void parse_str_2_intArr(char* str, int* intArr, int* count)
{
	char tempStr[20];
	int i, j, len;

	if (str == NULL || intArr == NULL || count == NULL)
	{
		return;
	}

	*count = 0;
	j = 0;
	len = strlen(str);
	for (i = 0; i < len; i++)
	{
		if (str[i] == '[')
			continue;

		if (str[i] == ',' || str[i] == ']')
		{
			tempStr[j] = '\0';
			if (strlen(tempStr) == 0)
				continue;

			intArr[*count] = atoi(tempStr);
			(*count)++;
			j = 0;
		}
		else
		{
			tempStr[j] = str[i];
			j++;
		}
	}
}

void cpy_char_arr(char* target_arr, char* source_arr, int count)
{
	int i;
	for (i = 0; i < count; i++)
		target_arr[i] = source_arr[i];
}

void cpy_int_arr(int* target_arr, int* source_arr, int count)
{
	int i;
	for (i = 0; i < count; i++)
		target_arr[i] = source_arr[i];
}

int parse_charArr_2_int(char* c_arr)
{
	int num;
	char* p = (char*)&num;

	p[0] = c_arr[0];
	p[1] = c_arr[1];
	p[2] = c_arr[2];
	p[3] = c_arr[3];

	return num;
}

void parse_int_2_charArr(int num, char* c_arr)
{
	char* p = (char*)&num;

	c_arr[0] = p[0];
	c_arr[1] = p[1];
	c_arr[2] = p[2];
	c_arr[3] = p[3];
}

u64 convert_double_2_ulong(double fValue)
{
	u64 iValue = (u64)fValue;
	if(iValue + 1 - fValue < 0.01)
		iValue++;
	return iValue;
}

void swap(int* pNum1, int* pNum2)
{
	int temp;
	temp = *pNum1;
	*pNum1 = *pNum2;
	*pNum2 = temp;
}

void shuffle(int tableId, int* arr, int len)
{
	int i, k;

	for (i = len - 1; i >= 0; i--)
	{
		k = GetRand(10000) % (i + 1);
		swap(&arr[i], &arr[k]);
	}
}