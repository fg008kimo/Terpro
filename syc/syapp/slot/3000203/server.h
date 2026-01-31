#ifndef ALG_SRV_H
#define ALG_SRV_H

#ifdef __cplusplus
extern "C" {
#endif

#define __LINUX__  //关掉__LINUX__：可跨平台编译、可跑主逻辑，但无法运行若干依赖linux模块例如网络&zlog&platform
// #define __DEBUG__
#ifndef __DEBUG__
    #define __ZLOG__
#endif 

#include <time.h>
#include <pthread.h>
#include "tools/uthash.h"
#include "tools/cJSON.h"

#ifdef __LINUX__
#include "zlog.h"
#endif

#define ALG_GAME_FISH        "fish"
#define ALG_GAME_CHARGE_POINT     "chargePoint"
#define ALG_GAME_MAMMON      "mammon"
#define ALG_GAME_HEAVEN      "heaven"
#define LOG_PATH           "/home/syc/log/"

#define MAX_EVENTS          200
#define ALG_KEEPALIVE         60
#define RANDOM_AUTH_STR_LEN   20

#define STR_LEN_8       8
#define STR_LEN_16      16
#define STR_LEN_20      20
#define STR_LEN_24      24
#define STR_LEN_32      32
#define STR_LEN_48      48
#define STR_LEN_64      64
#define STR_LEN_80      80
#define STR_LEN_128     128
#define STR_LEN_136     136     
#define STR_LEN_256     256
#define STR_LEN_512     512
#define STR_LEN_1024    1024
#define STR_LEN_2048    2048
#define STR_LEN_3072    3072
#define STR_LEN_3584    3584
#define STR_LEN_3840    3840
#define STR_LEN_4096    4096

enum game_type {
    fish = 0,
    chargePoint,
};

struct alg_db {
    int port;
	int listen_sock;
	int total_clients;
    int daemon;
    unsigned char password[STR_LEN_20];
	unsigned int keepalive;

	struct alg_client *db_client;     //all clients
    struct connect_client *c_client;     //conecting or connected clients	

	int epollfd;
};

struct alg_client {
    int index;
    unsigned int id;
	int connectStatus;
    char devName[STR_LEN_32];

	UT_hash_handle hh_id;    
};

struct connect_client {
    int sock;
    unsigned int id;
    char devName[STR_LEN_32];
	int auth;
	int remove;
	time_t last;
	time_t login;

    int isFromGame;

    char addr[STR_LEN_64];
	char incomplete_data[STR_LEN_3072];

    char random_num2[RANDOM_AUTH_STR_LEN + 4];

	UT_hash_handle hh_sock;   
};

struct alg_db alg;

#if (defined(__LINUX__) && defined(__ZLOG__)) 
    #define LOGD dzlog_debug
    #define LOGI dzlog_info
    #define LOGW dzlog_warn
    #define LOGE dzlog_error
#else
    #define __STR(R)    #R
    #define STR(R)  __STR(R)
    #define LOGD(...)   do{printf("\n");printf(__VA_ARGS__);}while(0)
    #define LOGI(...)   do{printf("\n");printf(__VA_ARGS__);}while(0)
    #define LOGW(...)   do{printf("\n");printf(__VA_ARGS__);}while(0)
    #define LOGE(...)   do{printf("\n");printf(__VA_ARGS__);}while(0)
#endif

//跨平台编译但不支持实际运行
#ifndef __LINUX__
#ifndef SIGPIPE
#define SIGPIPE 13
#endif
/* epoll */
#define EPOLL_CTL_ADD 1
#define EPOLL_CTL_DEL 2
#define EPOLL_CTL_MOD 3
#define EPOLLIN 1
#define EPOLLPRI 1
#define EPOLLOUT 1
typedef union epoll_data
{
    void *ptr;
    int fd;
    uint32_t u32;
    uint64_t u64;
} epoll_data_t;
struct epoll_event {
    uint32_t events;	/* Epoll events */
    epoll_data_t data;	/* User data variable */
};
int epoll_create (int __size);
int epoll_ctl (int __epfd, int __op, int __fd,struct epoll_event *__event);
int epoll_wait (int __epfd, struct epoll_event *__events, int __maxevents, int __timeout);
/* zlog */
int dzlog_init(const char *confpath, const char *cname);
#endif

pthread_mutex_t mutex; 

void alg_client_delete(struct connect_client *client);

void alg_sigpipe(int signal);
void alg_daemonise(void);

void alg_config_init(struct alg_db *db, int port, char *password);
void handle_login_msg(struct connect_client *client, cJSON *jMsg, char *version);

void handle_start_setting(int algPort);

int config_parse_args(struct alg_db *db, int argc, char *argv[]);

int alg_hb_thread_create(pthread_t *id);

void alg_heartbeat_msg(struct connect_client *client);

#ifdef __cplusplus
}
#endif

#endif

