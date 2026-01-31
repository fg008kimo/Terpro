#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <net/if_arp.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>

#include "rng.h"
#include "../../utils.h"
#include "../uthash.h"

#ifdef __LINUX__
#include <sys/epoll.h>
#endif

#define RNG_UINT32_LEN sizeof(uint32_t)
#define RNG_HEAD_LEN (4 * RNG_UINT32_LEN)
#define RNG_HEART_BEAT_VALUE 0
#define RNG_INVALID_SOCKET -1
#define RNG_LOCAL_HOST "127.0.0.1"
#define RNG_MAX_EVENTS 1
#define RNG_WAIT_TIME 100

union RngVAL {
#ifdef __LINUX__
	int i_val;
	long l_val;
	struct linger linger_val;
	struct timeval timeval_val;
#endif
}rngVal;

static int rngPort;
static struct RngNetDb rngNet;
static struct RngDoubleBufData doubleBuf; //适用于[0,1)范围随机数
static struct RngBufData *bufs;
void RngRequest(uint32_t minValue, uint32_t maxValue);

void RngCpyCharArr(char* target_arr, char* source_arr, int count)
{
	int i;
	for (i = 0; i < count; i++)
		target_arr[i] = source_arr[i];
}

uint32_t RngParseCharArr2Uint(char* c_arr)
{
	uint32_t num;
	char* p = (char*)&num;

	p[0] = c_arr[0];
	p[1] = c_arr[1];
	p[2] = c_arr[2];
	p[3] = c_arr[3];

	return num;
}

void RngParseUInt2CharArr(uint32_t num, char* c_arr)
{
	char* p = (char*)&num;

	c_arr[0] = p[0];
	c_arr[1] = p[1];
	c_arr[2] = p[2];
	c_arr[3] = p[3];
}

double RngParseCharArr2Double(char* c_arr)
{
	double num;
	char* p = (char*)&num;
    int i;

    for(i = sizeof(double)-1; i >= 0; i--)
        p[i] = c_arr[i];

	return num;
}

void RngParseDouble2CharArr(double num, char* c_arr)
{
	char* p = (char*)&num;
    int i;

    for(i = sizeof(double)-1; i >= 0; i--)
        c_arr[i] = p[i];
}

uint32_t RngGetNextHeadId()
{
	static uint32_t headId = 0;
	if(headId > INT32_MAX)
		headId = 0;
	return ++headId;
}

// 本地临时随机数生成器，返回[0,1)随机浮点数（“不可预测性”优先）
double RngGetTempRandom()
{
#ifdef __LINUX__
	static int round = 10000;

	round++;
	if(round >= 10000)
	{
		round = 0;
		srand(time(NULL));
	}
#else
	static int isFirst = 1;
	if(isFirst)
	{
		isFirst = 0;
		srand(time(NULL));
	}
#endif

	return (double)(rand() % RAND_MAX) / RAND_MAX;
}

// 获取当前时间（微秒）
long getCurMicrosecond()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec * 1000000 + tv.tv_usec;
}

/////////////////////////////////【网络接口(仿制net.c)】//////////////////////////////
int RngSocketNonblock(int sock);
void RngSetTcpNoDelay(int sock);
void RngClearReadInfo();

int RngNetIsValid(int sock)
{
#ifdef __LINUX__
	struct tcp_info tcpInfo;
	int tcpInfoLen = sizeof(tcpInfo);
	getsockopt(sock, IPPROTO_TCP, TCP_INFO, &tcpInfo, (socklen_t*)&tcpInfoLen);
	return tcpInfo.tcpi_state == TCP_ESTABLISHED;
#else
	return 1;
#endif
}

int RngSocketNormal()
{
	int new_sock = RNG_INVALID_SOCKET;
	int ss_opt = 1;

	new_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (new_sock == RNG_INVALID_SOCKET) {
		LOGE("RngSocketNormal Create socket failed, %s", strerror(errno));
		return RNG_INVALID_SOCKET;
	}

	ss_opt = 1;
	setsockopt(new_sock, SOL_SOCKET, SO_REUSEADDR, &ss_opt, sizeof(ss_opt));

	if (RngSocketNonblock(new_sock)) {
		LOGE("RngSocketNormal Set nonblock mode failed, %s", strerror(errno));
		return RNG_INVALID_SOCKET;
	}

	RngSetTcpNoDelay(new_sock);

	return new_sock;
}

int Rngconnect(int sock, int port, char* ip)
{
	struct sockaddr_in serveraddr;
	int flag;

	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(port);
	inet_pton(AF_INET, ip, &serveraddr.sin_addr);

	flag = connect(sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
	return (flag >= 0 || errno == EINPROGRESS) ? 1 : 0;
}

void RngClientInit(int sock)
{
    rngNet.client.sock = sock;
	RngClearReadInfo();
}

void RngCloseClient()
{
	LOGI("RngCloseClient start");

	if (epoll_ctl(rngNet.epollfd, EPOLL_CTL_DEL, rngNet.client.sock, NULL) == -1)
		LOGE("RngCloseClient error epoll_del: %s sock=%d", strerror(errno), rngNet.client.sock);

	LOGI("RngCloseClient Closing: [%d]", rngNet.client.sock);
	close(rngNet.client.sock);
	rngNet.client.sock = RNG_INVALID_SOCKET;

	LOGI("RngCloseClient end\n");
}

int RngRead(int sock, void *buf, size_t count)
{
	errno = 0;
	return read(sock, buf, count);
}

int RngWrite(uint32_t headId, uint32_t headMinValue, uint32_t headMaxValue)
{
#ifdef __LINUX__
	char writeBuf[RNG_WRITE_LEN_MAX], *tmp;
	size_t totalBytes;
    int len = 0;
	errno = 0;

	RngParseUInt2CharArr(htonl(3 * RNG_UINT32_LEN), writeBuf);
	RngParseUInt2CharArr(htonl(headId), &writeBuf[RNG_UINT32_LEN]);
	RngParseUInt2CharArr(htonl(headMinValue), &writeBuf[2*RNG_UINT32_LEN]);
	RngParseUInt2CharArr(htonl(headMaxValue), &writeBuf[3*RNG_UINT32_LEN]);

	totalBytes = RNG_HEAD_LEN;
	tmp = writeBuf;

    while (totalBytes != 0)
    {
        len = write(rngNet.client.sock, tmp, totalBytes);
        if (len <= 0)
        {
            if (errno == EAGAIN)
            {
                usleep(100);
                LOGE("RngWrite error [EAGAIN]\n");
                continue;
            }
            else
            {
                return len;
            }
        }
        else if (len < totalBytes)
        {
            tmp += len;
            totalBytes -= len;
        }
        else
        {
            break;
        }
    }

	return totalBytes;
#else
	return 0;
#endif
}

int RngSocketNonblock(int sock)
{
#ifdef __LINUX__
	int opt = 0;
	/* Set non-blocking */
	opt = fcntl(sock, F_GETFL, 0);
	if(opt == -1){
		RngCloseClient(sock);
		return 1;
	}
	if(fcntl(sock, F_SETFL, opt | O_NONBLOCK) == -1){
		/* If either fcntl fails, don't want to allow this client to connect. */
		RngCloseClient(sock);
		return 1;
	} 

    return 0;
#else
	return 0;
#endif
}

void RngSockStrFlag(union RngVAL* ptr, int len)
{
#ifdef __LINUX__
	if (len != sizeof(int))
		LOGI("sizeof %d", len);
	else
		LOGI("%s", (ptr->i_val == 0) ? "off" : "on");
#endif
}

void RngSetTcpNoDelay(int sock)
{
#ifdef __LINUX__
	socklen_t len;
	int flag, result;

	LOGI("RngSetTcpNoDelay_1");

	len = sizeof(rngVal);
	if (getsockopt(sock, IPPROTO_TCP, TCP_NODELAY, &rngVal, &len) == -1)
		LOGE("net_error getsockopt error");
	else
		RngSockStrFlag(&rngVal, len);

	flag = 1;
	result = setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(int));
	LOGI("RngSetTcpNoDelay result=%d", result);

	len = sizeof(rngVal);
	if (getsockopt(sock, IPPROTO_TCP, TCP_NODELAY, &rngVal, &len) == -1)
		LOGE("net_error getsockopt error 2");
	else
		RngSockStrFlag(&rngVal, len);

	LOGI("RngSetTcpNoDelay_2");
#endif
}

void RngClearReadInfo()
{
	rngNet.client.rState = eRngPackStateHeadLenInComplete;
	rngNet.client.rHeadLen = 0;
	rngNet.client.rHeadId = 0;
	rngNet.client.rBufUsefulInfoLen = 0;
}
int RngGetReadInfoTargetLen()
{
	if (rngNet.client.rState == eRngPackStateHeadLenInComplete)
	{
		return RNG_UINT32_LEN - rngNet.client.rBufUsefulInfoLen;
	}
	else if (rngNet.client.rState == eRngPackStateContentInComplete)
	{
		return rngNet.client.rHeadLen - rngNet.client.rBufUsefulInfoLen;
	}
	else
	{
		LOGE("RngGetReadInfoTargetLen error state=%d", rngNet.client.rState);
		return -1;
	}
	LOGE("RngGetReadInfoTargetLen error");
	return -1;
}

int RngGenSocket(int port)
{
    int new_sock;
    int isOk;

    new_sock = RngSocketNormal();
    isOk = 0;
    if (new_sock >= 0)
    {
        LOGI("RngGenSocket_success: port=%d sock=%d", port, new_sock);
        isOk = 1;
        if (!Rngconnect(new_sock, port, RNG_LOCAL_HOST))
        {
            LOGE("RngGenSocket connect error : port=%d sock=%d", port, new_sock);
            isOk = 0;
        }
    }

    if (isOk)
    {
        if (!RngNetIsValid(new_sock))
        {
            LOGE("RngGenSocket net_is_invalid : port=%d sock=%d", port, new_sock);
            isOk = 0;
        }
    }

    if (isOk)
    {
		RngClientInit(new_sock);
    }

    if (isOk)
    {
        struct epoll_event ev;
        ev.data.fd = new_sock;
        ev.events = EPOLLIN;
        if (epoll_ctl(rngNet.epollfd, EPOLL_CTL_ADD, new_sock, &ev) == -1) {
            LOGE("RngGenSocket epoll add error: %s port=%d sock=%d", strerror(errno), port, new_sock);
            isOk = 0;
        }
    }

    if (!isOk)
    {
        RngCloseClient();
        LOGE("RngGenSocket fail: port=%d ###############", port);
    }

    return isOk ? new_sock : RNG_INVALID_SOCKET;
}

int RngIsDouble(uint32_t minValue, uint32_t maxValue)
{
	return minValue == 0 && maxValue == 1;
}

int RngIsHb(uint32_t minValue, uint32_t maxValue)
{
	return minValue == RNG_HEART_BEAT_VALUE && maxValue == RNG_HEART_BEAT_VALUE;
}

void *RngHbThread()
{
    while(1)
    {
#ifdef __LINUX__
		RngRequest(RNG_HEART_BEAT_VALUE, RNG_HEART_BEAT_VALUE);
#endif
		sleep(30);
    }
}

int RngHbThreadCreate()
{
	pthread_t id;
    int ret = 0;

    ret = pthread_create(&id, NULL, (void *)RngHbThread, NULL);
    if (ret != 0) {
        LOGE("RngHbThreadCreate create pthread error!\n");
        return -1;
    }

    return 0;
}

/////////////////////////////////【内部逻辑】////////////////////////////////////////
void checkTimeout(long startTime)
{
	long needTime = getCurMicrosecond() - startTime;
	if(needTime >= 10000) //10毫秒
		LOGE("checkTimeout error: %ld", needTime);
}

long RngGetKey(uint32_t minValue, uint32_t maxValue)
{
	return (long)minValue + maxValue * 1000;
}

struct RngBufData *RngGetBufData(uint32_t minValue, uint32_t maxValue)
{
	if(RngIsDouble(minValue, maxValue))
		return NULL;

	struct RngBufData *pBuf = NULL;
	long key = RngGetKey(minValue, maxValue);
	HASH_FIND(hh_rng_buf, bufs, &key, sizeof(long), pBuf);
	if(pBuf == NULL)
	{
		pBuf = (struct RngBufData *)alg_calloc(1, sizeof(struct RngBufData));
		if(pBuf == NULL)
			LOGE("RngGetBufData Calloc memory failed.");
		pBuf->key = key;
		pBuf->index = RNG_RANDOM_NUM;
		HASH_ADD(hh_rng_buf, bufs, key, sizeof(long), pBuf);
	}
	return pBuf;
}

// 处理逻辑(仿制_mammon_packet())
void RngHandlePacket()
{
	struct RngConnectClient *client = &rngNet.client;
	struct RngBufData *pBuf = RngGetBufData(client->rHeadMinValue, client->rHeadMaxValue);
	int isDouble, totalBytes, itemBytes, num = 0;
	
	isDouble = RngIsDouble(client->rHeadMinValue, client->rHeadMaxValue);
	totalBytes = RNG_HEAD_LEN - RNG_UINT32_LEN; //不包括rHeadLen
	itemBytes = isDouble ? sizeof(double) : sizeof(uint32_t);
	while(num < RNG_RANDOM_NUM && totalBytes < client->rBufUsefulInfoLen)
	{
		if(isDouble)
		{
			if(num == 0)
				doubleBuf.index = 0;
			doubleBuf.data[num] = RngParseCharArr2Double(&client->rBuf[totalBytes]);
		}
		else
		{
			if(num == 0)
				pBuf->index = 0;
			pBuf->data[num] = ntohl(RngParseCharArr2Uint(&client->rBuf[totalBytes]));
		}
		num++;
		totalBytes += itemBytes;
	}
}

int RngShouldRequest(uint32_t minValue, uint32_t maxValue)
{
	if(RngIsHb(minValue, maxValue)) //心跳包
	{
		return 1;
	}
	else if(RngIsDouble(minValue, maxValue))
	{
		return doubleBuf.index >= RNG_RANDOM_NUM;
	}
	else
	{
		struct RngBufData *buf;
		buf = RngGetBufData(minValue, maxValue);
		return buf->index >= RNG_RANDOM_NUM;
	}
}

// 网络请求[阻塞等待](仿制platform_notice()和_mammon_main_loop())
void RngRequest(uint32_t minValue, uint32_t maxValue)
{
	if(!RngShouldRequest(minValue, maxValue))
		return;

	pthread_mutex_lock(&rngNet.requestMutex);
	int i;
	for(i = 0; i < 2; i++) //若通信失败则尝试一次断线重连
	{
		struct RngConnectClient *client = &rngNet.client;

		if(rngNet.client.sock == RNG_INVALID_SOCKET)
		{
			LOGI("RngRequest: port=%d ---------------", rngPort);
			if(RngGenSocket(rngPort) < 0)
				continue;
		}
		if(!RngNetIsValid(rngNet.client.sock))
		{
			LOGE("RngRequest net_is_invalid: port=%d sock=%d", rngPort, rngNet.client.sock);
			RngCloseClient();
			continue;
		}

		client->rHeadId = RngGetNextHeadId();
		client->rHeadMinValue = minValue;
		client->rHeadMaxValue = maxValue;
		RngWrite(client->rHeadId, client->rHeadMinValue, client->rHeadMaxValue);
		if(RngIsHb(minValue, maxValue)) //心跳包无需返回值
		{
			pthread_mutex_unlock(&rngNet.requestMutex);
			return;
		}

		long startTime = getCurMicrosecond();
		int fdcount = 0;
		struct epoll_event events[RNG_MAX_EVENTS]; 
		fdcount = epoll_wait(rngNet.epollfd, events, RNG_MAX_EVENTS, RNG_WAIT_TIME);
		if(fdcount != RNG_MAX_EVENTS || !(events[i].events & EPOLLIN))
		{
			LOGE("RngRequest error: fdcount=%d events=%u", fdcount, events[i].events);
			RngCloseClient();
			continue;
		}
		//checkTimeout(startTime); //SY modified at 0306

		while (1)
		{
			int targetLen = RngGetReadInfoTargetLen();;
			int recvLen = RngRead(client->sock, &client->rBuf[rngNet.client.rBufUsefulInfoLen], targetLen);
			if (recvLen < 0 || targetLen < 0 || recvLen > targetLen)
			{
				if (errno == EWOULDBLOCK)
					break;

				LOGE("RngRequest recvLen error_1 [%d] [%s]", errno, strerror(errno));
				LOGE("RngRequest recvLen error_2 [%d] [%d]", recvLen, targetLen);
				RngCloseClient();
				break;
			}

			client->rBufUsefulInfoLen += recvLen;
			client->rBuf[client->rBufUsefulInfoLen] = '\0';
			if (recvLen < targetLen)
            {
                if (recvLen == 0)
                {
					LOGE("RngRequest recvLen==0");
                    RngCloseClient();
					break;
                }
				continue;
            }

			client->rState++;
			if (client->rState == eRngPackStateContentInComplete)
			{
				client->rHeadLen = RngParseCharArr2Uint(client->rBuf);
				client->rHeadLen = ntohl(client->rHeadLen);
				if (client->rHeadLen < 3 * RNG_UINT32_LEN || client->rHeadLen >= RNG_READ_LEN_MAX)
				{
					LOGE("RngRequest recv headLen is error [%u] [%u]", client->rHeadLen, RngParseCharArr2Uint(client->rBuf));
					RngCloseClient();
					break;
				}
				client->rBufUsefulInfoLen = 0;
			}
			else if(client->rState == eRngPackStateComplete)
			{
				uint32_t headId = RngParseCharArr2Uint(client->rBuf);
				headId = ntohl(headId);
				if(headId != client->rHeadId)
				{
					LOGE("RngRequest headId error [%u] [%u]", client->rHeadId, headId);
					RngCloseClient();
					break;
				}
				RngHandlePacket();
				RngClearReadInfo();
				break;
			}
			else
			{
				LOGE("RngRequest state overflow");
				RngCloseClient();
				break;
			}
		}

		if(rngNet.client.sock != RNG_INVALID_SOCKET)
			break;
	}
	pthread_mutex_unlock(&rngNet.requestMutex);
}

/////////////////////////////////【对外接口】////////////////////////////////////////

void RngInit(int algPort)
{
	rngPort = algPort / 10000 * 10000 + 1;
	rngNet.client.sock = RNG_INVALID_SOCKET;
	pthread_mutex_init(&rngNet.requestMutex, NULL);
	if ((rngNet.epollfd = epoll_create(RNG_MAX_EVENTS)) == -1) {
		LOGE("RngInit Error in epoll creating: %s", strerror(errno));
		return;
	} 
	doubleBuf.index = RNG_RANDOM_NUM;
	RngHbThreadCreate();
}

double RngGetRandom()
{
#ifdef __LINUX__
	RngRequest(0, 1);
	if(RngShouldRequest(0, 1))
	{
		//todo telegram报警
		LOGE("RngGetRandom error: no data");
		usleep(1000);
		return RngGetRandom(); //递归请求，直到有数据为止
	}
	else
	{
		return doubleBuf.data[doubleBuf.index++];
	}
#else
	return RngGetTempRandom();
#endif
}

uint32_t RngGetRand(uint32_t minValue,uint32_t maxValue)
{
#ifdef __LINUX__
	RngRequest(minValue, maxValue);
	if(RngShouldRequest(minValue, maxValue))
	{
		//todo telegram报警
		LOGE("RngGetRand error: no data [%u,%u)", minValue, maxValue);
		usleep(1000);
		return RngGetRand(minValue, maxValue); //递归请求，直到有数据为止
	}
	else
	{
		struct RngBufData *pBuf;
		pBuf = RngGetBufData(minValue, maxValue);
		return pBuf->data[pBuf->index++];
	}
#else
	return (uint32_t)(RngGetTempRandom() * (maxValue - minValue)) + minValue;
#endif
}