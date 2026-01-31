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
#include "utils.h"
#include "tools/uthash.h"

#ifdef __LINUX__
#include <sys/epoll.h>
#endif

#define RNG_UINT32_LEN sizeof(uint32_t)
#define RNG_HEAD_LEN (4 * RNG_UINT32_LEN)
#define RNG_HEART_BEAT_VALUE 0
#define RNG_INVALID_SOCKET -1
#define RNG_MAX_EVENTS 200
#define RNG_RANDOM_NUM 1000
#define RNG_KEEPALIVE 60

union RngVAL {
#ifdef __LINUX__
	int i_val;
	long l_val;
	struct linger linger_val;
	struct timeval timeval_val;
#endif
}rngVal;

static struct RngNetDb rngNet;
static char writeBuf[RNG_WRITE_LEN_MAX];

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

/////////////////////////////////【网络接口(仿制net.c)】//////////////////////////////
void RngCloseClient(int sock);
void RngSetTcpNoDelay(int sock);
void RngClearReadInfo(int sock);

struct RngConnectClient *RngClientInit(int sock)
{
	struct RngConnectClient *client = NULL, *tmp = NULL;
	char address[64] = {0};

	client = (struct RngConnectClient *)alg_calloc(1, sizeof(struct RngConnectClient));
	if(!client)
	{
		LOGE("RngClientInit Calloc memory failed.");
        return NULL; 
	}
    client->sock = sock;
	client->last = time(NULL);

	HASH_FIND(hh_rng_sock, rngNet.clients, &sock, sizeof(int), tmp);
	if (tmp != NULL)
	{
		LOGE("RngClientInit Device id duplicate");
		RngCloseClient(tmp->sock);
	}
	LOGE("RngClientInit Add [%d] to hash table, count: [%d]", sock, HASH_CNT(hh_rng_sock, rngNet.clients));
	HASH_ADD(hh_rng_sock, rngNet.clients, sock, sizeof(client->sock), client);

	RngClearReadInfo(sock);
    
	return client;
}

void RngCloseClient(int sock)
{
	struct RngConnectClient* client = NULL;

	LOGI("RngCloseClient start");

	pthread_mutex_lock(&rngNet.rmSocksMutex);
	HASH_FIND(hh_rng_sock, rngNet.clients, &sock, sizeof(int), client);
	if (client == NULL)
		LOGE("RngCloseClient error_1 hash_del: sock=%d", sock);
	else
		HASH_DELETE(hh_rng_sock, rngNet.clients, client);

	if (epoll_ctl(rngNet.epollfd, EPOLL_CTL_DEL, sock, NULL) == -1)
		LOGE("RngCloseClient error_2 epoll_del: %s sock=%d", strerror(errno), sock);

	if(client != NULL)
		LOGI("RngCloseClient Closing: [%d], count=[%d]", sock, HASH_CNT(hh_rng_sock, rngNet.clients));
	close(sock);
	alg_free(client);
	pthread_mutex_unlock(&rngNet.rmSocksMutex);

	LOGI("RngCloseClient end\n");
}

int RngRead(int sock, void *buf, size_t count)
{
	errno = 0;
	return read(sock, buf, count);
}

int RngWrite(int sock, size_t totalBytes)
{
#ifdef __LINUX__
    char *tmp;
	struct RngConnectClient* client = NULL;
    int len = 0;
	errno = 0;

	HASH_FIND(hh_rng_sock, rngNet.clients, &sock, sizeof(int), client);
	if (client == NULL)
	{
		LOGE("RngWrite error hash_del: sock=%d", sock);
		return 0;
	}

	RngParseUInt2CharArr(htonl((3 * RNG_UINT32_LEN + totalBytes)), writeBuf);
	RngParseUInt2CharArr(htonl(client->rHeadId), &writeBuf[RNG_UINT32_LEN]);
	RngParseUInt2CharArr(htonl(client->rHeadMinValue), &writeBuf[2*RNG_UINT32_LEN]);
	RngParseUInt2CharArr(htonl(client->rHeadMaxValue), &writeBuf[3*RNG_UINT32_LEN]);

	totalBytes += 4 * RNG_UINT32_LEN;
	tmp = writeBuf;

    while (totalBytes != 0)
    {
        len = write(sock, tmp, totalBytes);
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

int RngSocketListen(int port)
{
#ifdef __LINUX__
	int sock = RNG_INVALID_SOCKET;
    int ss_opt = 1;
    struct sockaddr_in serveraddr;

	sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock == RNG_INVALID_SOCKET){
        LOGE("RngSocketListen Create socket failed, %s", strerror(errno));
        return RNG_INVALID_SOCKET;
    }

	ss_opt = 1;
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &ss_opt, sizeof(ss_opt));   

	if(RngSocketNonblock(sock)){
        LOGE("RngSocketListen Set nonblock mode failed, %s", strerror(errno));
		return RNG_INVALID_SOCKET;
	} 

    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET; 
    serveraddr.sin_port = htons(port); 
    serveraddr.sin_addr.s_addr = INADDR_ANY;
    if (bind(sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) == RNG_INVALID_SOCKET)
    {
        LOGE("RngSocketListen bind error, %s", strerror(errno));
        close(sock);
        return RNG_INVALID_SOCKET;
    }

	if (listen(sock, 100) == -1){
		LOGE("RngSocketListen listen error, %s", strerror(errno));
		close(sock);
		return RNG_INVALID_SOCKET;
	} 

    return sock;
#else
	return RNG_INVALID_SOCKET;
#endif
}

int RngSocketAccept()
{
#ifdef __LINUX__
	int new_sock = RNG_INVALID_SOCKET;
    struct RngConnectClient *new_client;

	new_sock = accept(rngNet.listenSock, NULL, 0);
	if(new_sock == RNG_INVALID_SOCKET) 
    {
        LOGE("RngSocketAccept accept error, %d, %s", errno, strerror(errno));
        return RNG_INVALID_SOCKET;
	}

	if(RngSocketNonblock(new_sock)){
        LOGE("RngSocketAccept set nonblock error, %s", strerror(errno));
		return RNG_INVALID_SOCKET;
	}

    new_client = RngClientInit(new_sock);

	RngSetTcpNoDelay(new_sock);

    return new_sock;
#else
	return RNG_INVALID_SOCKET;
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

void RngClearReadInfo(int sock)
{
	struct RngConnectClient* client = NULL;
	HASH_FIND(hh_rng_sock, rngNet.clients, &sock, sizeof(int), client);
	if (client != NULL)
	{
		client->rState = eRngPackStateHeadLenInComplete;
		client->rHeadLen = 0;
		client->rHeadId = 0;
		client->rBufUsefulInfoLen = 0;
	}
	else
	{
		LOGE("RngClearPackInfo error");
	}
}
int RngGetReadInfoTargetLen(int sock)
{
	struct RngConnectClient* client = NULL;
	HASH_FIND(hh_rng_sock, rngNet.clients, &sock, sizeof(int), client);
	if (client != NULL)
	{
		if (client->rState == eRngPackStateHeadLenInComplete)
		{
			return RNG_UINT32_LEN - client->rBufUsefulInfoLen;
		}
		else if (client->rState == eRngPackStateContentInComplete)
		{
			return client->rHeadLen - client->rBufUsefulInfoLen;
		}
		else
		{
			LOGE("RngGetReadInfoTargetLen error state=%d", client->rState);
			return -1;
		}
	}
	LOGE("RngGetReadInfoTargetLen error");
	return -1;
}

void *RngHbThread()
{
    struct RngConnectClient *client = NULL, *tmp = NULL;
    struct RngRemoveSock *rmSock = NULL, *tmp2 = NULL;
	time_t now = time(NULL);

    while(1)
    {
		now = time(NULL);

		pthread_mutex_lock(&rngNet.rmSocksMutex);
        HASH_ITER(hh_rng_sock, rngNet.clients, client, tmp)
        {
			if (now - client->last > RNG_KEEPALIVE)
			{
				HASH_FIND(hh_rng_remove_sock, rngNet.rmSocks, &client->sock, sizeof(int), tmp2);
				if(tmp2 == NULL)
				{
					LOGE("Client [%d] more than %ds doesn't receieve data, disconnect it.", client->sock, RNG_KEEPALIVE);
					rmSock = (struct RngRemoveSock *)alg_calloc(1, sizeof(struct RngRemoveSock));
					rmSock->sock = client->sock;
					HASH_ADD(hh_rng_remove_sock, rngNet.rmSocks, sock, sizeof(int), rmSock);
				}
			}
        }
		pthread_mutex_unlock(&rngNet.rmSocksMutex);

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

void RngHeartbeatMsg(struct RngConnectClient *client)
{
	client->last = time(NULL);
}

/////////////////////////////////【内部逻辑】////////////////////////////////////////
// 处理逻辑(仿制_mammon_packet())
void RngHandlePacket(struct RngConnectClient *client)
{
	int isDouble;
	int num, totalBytes = 0, itemBytes;
	
	client->last = time(NULL);
	isDouble = (client->rHeadMinValue == 0 && client->rHeadMaxValue == 1) ? 1 : 0;
	itemBytes = isDouble ? sizeof(double) : sizeof(uint32_t);
	for(num = 0; num < RNG_RANDOM_NUM; num++)
	{
		if(isDouble)
			RngParseDouble2CharArr(GetRandom(), &writeBuf[RNG_HEAD_LEN+totalBytes]);
		else
			RngParseUInt2CharArr(htonl(GetRand(client->rHeadMinValue, client->rHeadMaxValue)), &writeBuf[RNG_HEAD_LEN+totalBytes]);
		totalBytes += itemBytes;
	}

	RngWrite(client->sock, totalBytes);
}

// 处理网络请求(仿制_handle_mammon_msg())
int RngHandleMsg(int sock, uint32_t events)
{
#ifdef __LINUX__
    struct RngConnectClient *client = NULL;
	struct RngRemoveSock *tmp2;
    int recvLen, targetLen;

    if (events & EPOLLIN)
    {
		HASH_FIND(hh_rng_sock, rngNet.clients, &sock, sizeof(int), client);
		if (client == NULL)
		{
			LOGE("RngHandleMsg [%d] Client is null", sock);
			return -1;
		}

		while (1)
		{
			targetLen = RngGetReadInfoTargetLen(sock);;
			recvLen = RngRead(sock, &client->rBuf[client->rBufUsefulInfoLen], targetLen);
			if (recvLen < 0 || targetLen < 0 || recvLen > targetLen)
			{
				if (errno == EWOULDBLOCK)
					break;

				LOGE("RngHandleMsg recvLen error_1 [%d] [%s]", errno, strerror(errno));
				LOGE("RngHandleMsg recvLen error_2 [%d] [%d]", recvLen, targetLen);
				return -1;
			}

			client->rBufUsefulInfoLen += recvLen;
			client->rBuf[client->rBufUsefulInfoLen] = '\0';
			if (recvLen < targetLen)
            {
                if (recvLen == 0)
                {
                    return -1;
                }
				break;
            }

			client->rState++;
			if (client->rState == eRngPackStateContentInComplete)
			{
				client->rHeadLen = RngParseCharArr2Uint(client->rBuf);
				client->rHeadLen = ntohl(client->rHeadLen);
				if (client->rHeadLen < 3 * RNG_UINT32_LEN || client->rHeadLen >= RNG_READ_LEN_MAX)
				{
					LOGE("RngHandleMsg recv headLen is error [%u] [%u]", client->rHeadLen, RngParseCharArr2Uint(client->rBuf));
					return -1;
				}
				client->rBufUsefulInfoLen = 0;
			}
			else if(client->rState == eRngPackStateComplete)
			{
				client->rHeadId = RngParseCharArr2Uint(client->rBuf);
				client->rHeadId = ntohl(client->rHeadId);
				client->rHeadMinValue = RngParseCharArr2Uint(&client->rBuf[RNG_UINT32_LEN]);
				client->rHeadMinValue = ntohl(client->rHeadMinValue);
				client->rHeadMaxValue = RngParseCharArr2Uint(&client->rBuf[2*RNG_UINT32_LEN]);
				client->rHeadMaxValue = ntohl(client->rHeadMaxValue);
				if(client->rHeadMinValue == RNG_HEART_BEAT_VALUE && client->rHeadMaxValue == RNG_HEART_BEAT_VALUE) //心跳包
					RngHeartbeatMsg(client);
				else
					RngHandlePacket(client);
				RngClearReadInfo(client->sock);
				break;
			}
			else
			{
				LOGE("RngHandleMsg state overflow");
				return -1;
			}
		}
    }
#endif

    return 1;
}

/////////////////////////////////【对外接口】////////////////////////////////////////

void RngMainLoop(int port)
{
	int fdcount = 0;
	int i = 0, j = 0, ret = 0;
	struct epoll_event ev, events[RNG_MAX_EVENTS];    
    struct RngConnectClient *client = NULL, *tmp = NULL;
	struct RngRemoveSock *rmSock = NULL, *tmp2 = NULL;

	rngNet.listenSock = RngSocketListen(port);
	if(rngNet.listenSock == RNG_INVALID_SOCKET)
		LOGE("RngMainLoop Listening error");
	else
		LOGI("Listening on port %d ......\n", port);

	if ((rngNet.epollfd = epoll_create(RNG_MAX_EVENTS)) == -1) {
		LOGE("RngMainLoop Error in epoll creating: %s", strerror(errno));
		return;
	} 

	memset(&ev, 0, sizeof(struct epoll_event));
	memset(&events, 0, sizeof(struct epoll_event)*RNG_MAX_EVENTS);
	ev.data.fd = rngNet.listenSock;
	ev.events = EPOLLIN;
	if (epoll_ctl(rngNet.epollfd, EPOLL_CTL_ADD, rngNet.listenSock, &ev) == -1) {
		LOGE("RngMainLoop in epoll initial registering: %s", strerror(errno));
		(void)close(rngNet.epollfd);
		rngNet.epollfd = 0;
		return;
	}

	pthread_mutex_init(&rngNet.rmSocksMutex, NULL);
	ret = RngHbThreadCreate();
	if(ret == -1)
	{
		close(rngNet.listenSock);
		pthread_mutex_destroy(&rngNet.rmSocksMutex);
		return;
	}

    while (1)
    {
        HASH_ITER(hh_rng_remove_sock, rngNet.rmSocks, rmSock, tmp2)
        {
			LOGI("RngMainLoop Delete client [%d].", rmSock->sock);
			RngCloseClient(rmSock->sock);
			pthread_mutex_lock(&rngNet.rmSocksMutex);
			HASH_DELETE(hh_rng_remove_sock, rngNet.rmSocks, rmSock);
			alg_free(rmSock);
			pthread_mutex_unlock(&rngNet.rmSocksMutex);
        }
		
        fdcount = epoll_wait(rngNet.epollfd, events, RNG_MAX_EVENTS, -1);
		switch(fdcount){
		case -1:
			if(errno != EINTR){
				LOGE("RngMainLoop in epoll waiting: %s.", strerror(errno));
			}
			break;
		case 0:
			break;
		default:
			for(i=0; i < fdcount; i++){
				if (events[i].data.fd == rngNet.listenSock) {
					if (events[i].events & (EPOLLIN | EPOLLPRI)){
						if((ev.data.fd = RngSocketAccept()) != RNG_INVALID_SOCKET){
							ev.events = EPOLLIN;
							if (epoll_ctl(rngNet.epollfd, EPOLL_CTL_ADD, ev.data.fd, &ev) == -1)
								LOGE("RngMainLoop in epoll accepting: %s", strerror(errno));
						}                        
					}
				}
				else
				{
                    if (events[i].events & EPOLLIN)
    				{
                        if (RngHandleMsg(events[i].data.fd, events[i].events) == -1)
                        {
							RngCloseClient(events[i].data.fd);
                            events[i].data.fd = -1;
                        }
    				}
				}
			}
            break;
		}
    }
}