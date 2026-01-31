#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

#include "net.h"
#include "server.h"
#include "utils.h"

#ifdef __LINUX__
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <net/if_arp.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <sys/epoll.h>
#endif

static int sg_client_cnt = 0;

union VAL {
#ifdef __LINUX__
	int i_val;
	long l_val;
	struct linger linger_val;
	struct timeval timeval_val;
#endif
}val;

PACK_INFO pack_info[MAX_SOCK_NUM];

char net_write_buf[STR_LEN_4096]; //要与PACK_INFO.buf[]大小一致

void SetTcpNoDelay(int sock);

int net_socket_get_address(int sock, char *buf, int len)
{
#ifdef __LINUX__
	struct sockaddr_storage addr;
	socklen_t addrlen;

	addrlen = sizeof(addr);
	if(!getpeername(sock, (struct sockaddr *)&addr, &addrlen)){
		if(addr.ss_family == AF_INET){
			if(inet_ntop(AF_INET, &((struct sockaddr_in *)&addr)->sin_addr.s_addr, buf, len)){
				return 0;
			}
		}else if(addr.ss_family == AF_INET6){
			if(inet_ntop(AF_INET6, &((struct sockaddr_in6 *)&addr)->sin6_addr.s6_addr, buf, len)){
				return 0;
			}
		}
	}
#endif
	return 1;
}

struct connect_client *client_init(struct alg_db *hs, int sock, int isFromGame)
{
	struct connect_client *client = NULL, *tmp = NULL;
	char address[64] = {0};

	client = (struct connect_client *)alg_calloc(1, sizeof(struct connect_client));
	if(!client)
	{
		LOGE("net_error Calloc memory failed.");
        return NULL; 
	}

    client->sock = sock;

	if (isFromGame)
	{
		if (net_socket_get_address(sock, address, sizeof(address)) == 0)
		{
			strncpy(client->addr, address, sizeof(client->addr) - 1);
		}
		else
		{
			LOGE("net_error Get peer address failed.\n");
			//return NULL;
		}
    }

	if (client->sock >= 0)
    {
    	HASH_FIND(hh_sock, hs->c_client, &sock, sizeof(int), tmp);
		if (tmp != NULL)
		{
			LOGE("net_error Device id duplicate.");
			//close(tmp->sock);
            HASH_DELETE(hh_sock, hs->c_client, tmp);
			alg_free(tmp);
		}
		sg_client_cnt++;
		LOGI("net Add [%s|%d] to hash table, count: [%d]", client->addr, sock, sg_client_cnt);
		client->last = client->login = time(NULL);
		client->isFromGame = isFromGame;
		HASH_ADD(hh_sock, hs->c_client, sock, sizeof(client->sock), client);
	}
    
	return client;
}

void close_client(int sock, struct connect_client* client)
{
	LOGI("net close_client() start");
	if (client == NULL)
	{
		HASH_FIND(hh_sock, alg.c_client, &sock, sizeof(int), client);
	}
	if (client == NULL || client->sock != sock)
	{
		LOGE("net_error close_client() sock=%d", sock);
		if (client != NULL)
			LOGE("net_error close_client() client_sock=%d", client->sock);
	}

	if (epoll_ctl(alg.epollfd, EPOLL_CTL_DEL, sock, NULL) == -1)
		LOGE("net_error close_client() epoll_del: %s sock=%d", strerror(errno), sock);
	close_pack_info(sock);
	close(sock);
	if (client != NULL)
		alg_client_delete(client);
	LOGI("net close_client() end\n");
}

int net_is_valid(int sock)
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

int net_connect(int sock, int port, char* ip)
{
#ifdef __LINUX__
	struct sockaddr_in serveraddr;
	int flag;

	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(port);
	inet_pton(AF_INET, ip, &serveraddr.sin_addr);

	flag = connect(sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
	return (flag >= 0 || errno == EINPROGRESS) ? 1 : 0;
#else
	return 0;
#endif
}

int net_read(int sock, void *buf, size_t count)
{
	errno = 0;

	return read(sock, buf, count);
}

int net_write(int sock, void *buf, size_t count)
{
#ifdef __LINUX__
    char *tmp;
	char headBuf[PACK_HEAD_LEN_BYTES + PACK_HEAD_ID_BYTES] = {0};
    int len = 0;
	errno = 0;

    pthread_mutex_lock(&mutex);

	parse_int_2_charArr(htonl((PACK_HEAD_ID_BYTES + count)), headBuf);
	parse_int_2_charArr(htonl(get_pack_info_headId(sock)), &headBuf[PACK_HEAD_LEN_BYTES]);
	cpy_char_arr(net_write_buf, headBuf, (PACK_HEAD_LEN_BYTES + PACK_HEAD_ID_BYTES));

	cpy_char_arr(&net_write_buf[PACK_HEAD_LEN_BYTES + PACK_HEAD_ID_BYTES], buf, count);
	count += PACK_HEAD_LEN_BYTES + PACK_HEAD_ID_BYTES;
	tmp = net_write_buf;

    while (count != 0)
    {
        len = write(sock, tmp, count);
        if (len <= 0)
        {
            if (errno == EAGAIN)
            {
                usleep(100);
                LOGE("net_error write error [EAGAIN]\n");
                continue;
            }
            else
            {
				pthread_mutex_unlock(&mutex);
                return len;
            }
        }
        else if (len < count)
        {
            tmp += len;
            count -= len;
        }
        else
        {
            break;
        }
    }
    pthread_mutex_unlock(&mutex);

	return count;
#else
	return 0;
#endif
}

/*
 * Function Name: net_write_error
 *
 * Description  : send error message to client
 *
 * Parameter:     sock <IN>:  client connect socket
 *                buf <IN>:  error content
 *
 * Return:   None.
 *
 */
void net_write_error(int sock, char *buf)
{
    LOGI("net_error Send error message to client[%d]: %s", sock, buf);
    net_write(sock, buf, strlen(buf));
    usleep(10);
	close_client(sock, NULL);
}

int net_socket_nonblock(int sock)
{
#ifdef __LINUX__
	int opt = 0;
	/* Set non-blocking */
	opt = fcntl(sock, F_GETFL, 0);
	if(opt == -1){
		close_client(sock, NULL);
		return 1;
	}
	if(fcntl(sock, F_SETFL, opt | O_NONBLOCK) == -1){
		/* If either fcntl fails, don't want to allow this client to connect. */
		close_client(sock, NULL);
		return 1;
	} 

    return 0;
#else
	return 0;
#endif
}

int net_socket_listen(int port)
{
#ifdef __LINUX__
	int sock = INVALID_SOCKET;
    int ss_opt = 1;
    struct sockaddr_in serveraddr;

	sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock == INVALID_SOCKET){
        LOGE("net_error Create socket failed, %s", strerror(errno));
        return -1;
    }

	ss_opt = 1;
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &ss_opt, sizeof(ss_opt));   

	if(net_socket_nonblock(sock)){
        LOGE("net_error Set nonblock mode failed, %s", strerror(errno));
		return -1;
	} 

    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET; 
    serveraddr.sin_port = htons(port); 
    serveraddr.sin_addr.s_addr = INADDR_ANY;
    if (bind(sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) == INVALID_SOCKET)
    {
        LOGE("net_error bind error, %s", strerror(errno));
        close(sock);
        return -1;
    }

	if (listen(sock, 100) == -1){
		LOGE("net_error listen error, %s", strerror(errno));
		close(sock);
		return -1;
	} 

    return sock;
#else
	return 0;
#endif
}

int net_socket_accept(struct alg_db *hs)
{
#ifdef __LINUX__
	int new_sock = INVALID_SOCKET;
    struct connect_client *new_client;

	new_sock = accept(hs->listen_sock, NULL, 0);
	if(new_sock == INVALID_SOCKET) 
    {
        LOGE("net_error accept error, %d, %s", errno, strerror(errno));
        return -1;
	}

	if(net_socket_nonblock(new_sock)){
        LOGE("net_error set nonblock error, %s", strerror(errno));
		return INVALID_SOCKET;
	}

    new_client = client_init(hs, new_sock, 1);

	SetTcpNoDelay(new_sock);

    return new_sock;
#else
	return 0;
#endif
}

int net_socket_normal(struct alg_db* hs)
{
#ifdef __LINUX__
	int new_sock = INVALID_SOCKET;
	int ss_opt = 1;

	new_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (new_sock == INVALID_SOCKET) {
		LOGE("net_error net_socket_normal() Create socket failed, %s", strerror(errno));
		return INVALID_SOCKET;
	}

	ss_opt = 1;
	setsockopt(new_sock, SOL_SOCKET, SO_REUSEADDR, &ss_opt, sizeof(ss_opt));

	if (net_socket_nonblock(new_sock)) {
		LOGE("net_error net_socket_normal() Set nonblock mode failed, %s", strerror(errno));
		return INVALID_SOCKET;
	}

	SetTcpNoDelay(new_sock);

	return new_sock;
#else
	return 0;
#endif
}

void sock_str_flag(union VAL* ptr, int len)
{
#ifdef __LINUX__
	if (len != sizeof(int))
		LOGI("sizeof %d", len);
	else
		LOGI("%s", (ptr->i_val == 0) ? "off" : "on");
#endif
}

void SetTcpNoDelay(int sock)
{
#ifdef __LINUX__
	socklen_t len;
	int flag, result;

	LOGI("net SetTcpNodelay_1");

	len = sizeof(val);
	if (getsockopt(sock, IPPROTO_TCP, TCP_NODELAY, &val, &len) == -1)
		LOGE("net_error getsockopt error");
	else
		sock_str_flag(&val, len);

	flag = 1;
	result = setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(int));
	LOGI("net result=%d", result);

	len = sizeof(val);
	if (getsockopt(sock, IPPROTO_TCP, TCP_NODELAY, &val, &len) == -1)
		LOGE("net_error getsockopt error 2");
	else
		sock_str_flag(&val, len);

	LOGI("net SetTcpNodelay_2");
#endif
}

int get_pack_info_index(int sockfd)
{
	int i;
	for (i = 0; i < MAX_SOCK_NUM; i++)
		if (pack_info[i].sockfd == sockfd)
			return i;
	return PACK_SOCKFD_NULL;
}
PACK_INFO* get_pack_info(int sockfd)
{
	int index;
	index = get_pack_info_index(sockfd);
	if (index >= 0)
		return &pack_info[index];
	return NULL;
}
void clear_pack_info(int sockfd)
{
	int index;
	index = get_pack_info_index(sockfd);
	if (index >= 0)
	{
		pack_info[index].state = ePackStateHeadLenInComplete;
		pack_info[index].headLen = 0;
		pack_info[index].headId = 0;
		pack_info[index].bufUsefulInfoLen = 0;
	}
}
void init_pack_info(int sockfd)
{
	int i, index;
	index = get_pack_info_index(sockfd);
	if (index < 0)
	{
		for (i = 0; i < MAX_SOCK_NUM; i++)
			if (pack_info[i].sockfd < 0)
			{
				index = i;
				break;
			}
	}

	if (index < MAX_SOCK_NUM)
	{
		pack_info[index].sockfd = sockfd;
		clear_pack_info(sockfd);
	}
	else
	{
		LOGE("net_error net_pack_info[] is full\n");
	}
}
void close_pack_info(int sockfd)
{
	int index;
	index = get_pack_info_index(sockfd);
	if (index >= 0)
	{
		pack_info[index].sockfd = PACK_SOCKFD_NULL;
		clear_pack_info(sockfd);
	}
}
void close_all_pack_info()
{
	int i;
	for (i = 0; i < MAX_SOCK_NUM; i++)
	{
		pack_info[i].sockfd = PACK_SOCKFD_NULL;
		pack_info[i].state = ePackStateHeadLenInComplete;
		pack_info[i].headLen = 0;
		pack_info[i].headId = 0;
		pack_info[i].bufUsefulInfoLen = 0;
	}
}
int get_pack_info_targetLen(int sockfd)
{
	int index = get_pack_info_index(sockfd);
	if (pack_info[index].state == ePackStateHeadLenInComplete)
	{
		return PACK_HEAD_LEN_BYTES - pack_info[index].bufUsefulInfoLen;
	}
	else if (pack_info[index].state == ePackStateContentInComplete)
	{
		return pack_info[index].headLen - pack_info[index].bufUsefulInfoLen;
	}
	else
	{
		LOGE("net_error state error in get_pack_info_targetLen()");
		return -1;
	}
}
int get_pack_info_headId(int sockfd)
{
	int index;
	index = get_pack_info_index(sockfd);
	if (index >= 0)
		return pack_info[index].headId;
	else
		return -1;
}

