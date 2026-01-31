#ifndef NET_H
#define NET_H

#ifdef __cplusplus
extern "C" {
#endif

#include "server.h"

#define INVALID_SOCKET      -1
#define LOCAL_HOST "127.0.0.1"

int net_socket_get_address(int sock, char *buf, int len);

struct connect_client* client_init(struct alg_db* hs, int sock, int isFromGame);

void close_client(int sock, struct connect_client* client);

int net_is_valid(int sock);
int net_connect(int sock, int port, char* ip);
int net_read(int sock, void *buf, size_t count);
int net_write(int sock, void *buf, size_t count);
void net_write_error(int sock, char *buf);

int net_socket_nonblock(int sock);

int net_socket_listen(int port);

int net_socket_accept(struct alg_db *hs);

int net_socket_normal(struct alg_db* hs);

#define MAX_SOCK_NUM  100
#define PACK_HEAD_LEN_BYTES 4
#define PACK_HEAD_ID_BYTES 4
#define PACK_SOCKFD_NULL -1
enum pack_state {
	ePackStateHeadLenInComplete = 0, // 包头的长度字段不完整
	ePackStateContentInComplete,     // 包内容不完整
	ePackStateComplete,              // 完整的包
};
typedef struct {
	int sockfd;
	int state;
	int headLen;
	int headId;
	char buf[STR_LEN_4096]; //要与net_write_buf[]大小一致
	int bufUsefulInfoLen;
}PACK_INFO;
PACK_INFO* get_pack_info(int sockfd);
void clear_pack_info(int sockfd);
void init_pack_info(int sockfd);
void close_pack_info(int sockfd);
void close_all_pack_info();
int get_pack_info_targetLen(int sockfd);
int get_pack_info_headId(int sockfd);

#ifdef __cplusplus
}
#endif

#endif

