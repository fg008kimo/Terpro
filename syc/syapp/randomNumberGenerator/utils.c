#include <stdio.h>
#include <stdlib.h>

#include "utils.h"
#include "logic/rand.h"


#ifndef __LINUX__
int epoll_create (int __size) {return 0;}
int epoll_ctl (int __epfd, int __op, int __fd,struct epoll_event *__event){return 0;}
int epoll_wait (int __epfd, struct epoll_event *__events, int __maxevents, int __timeout){return 0;}
int dzlog_init(const char *confpath, const char *cname){return 0;}
#endif

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

void RandInit()
{
	RDInit();
}

double GetRandom()
{
	return RDGetRandom();
}

uint32_t GetRand(uint32_t minValue,uint32_t maxValue)
{
	return RDGetRand(minValue, maxValue);
}