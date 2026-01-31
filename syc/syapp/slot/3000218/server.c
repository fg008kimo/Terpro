#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>

#include "server.h"
#include "tools/cJSON.h"
#include "net.h"
#include "Alg2.h"

// Function Name: _client_delete
// Description  : Remove client info from hash table
// Parameter    : client <IN>:  
// Return       : None.
void alg_client_delete(struct connect_client *client)
{
    struct alg_client *tmp_client = NULL;
    struct connect_client *client1 = NULL, *tmp = NULL;
   //update client status
    
    if (client->id >= MIN_CLIENT_ID)
    {
        HASH_FIND(hh_id, alg.db_client, &(client->id), sizeof(int), tmp_client);
        if (tmp_client != NULL)
        {
            LOGE("Set client connect status as 0.");
            tmp_client->connectStatus = 0;
        }
    }

    pthread_mutex_lock(&mutex);
    LOGI("Delete client [%d|%s|%u|%s]", client->sock, client->addr, client->id, client->devName);
    HASH_DELETE(hh_sock, alg.c_client, client);
    alg_free(client);
    pthread_mutex_unlock(&mutex);   
}


void alg_sigpipe(int signal)
{
    LOGI("#######receive sigpipe signal, ignore###########");
}

void alg_config_init(struct alg_db *db, int port, char *password)
{
    memset(db, 0, sizeof(struct alg_db));
    db->port = port;
	db->keepalive = ALG_KEEPALIVE;
    strcpy((char *)(db->password), password);
}

void alg_daemonise(void)
{
#ifdef __LINUX__
    char err[STR_LEN_256] = {0};
    pid_t pid;

    pid = fork();
    if(pid < 0)
    {
        strerror_r(errno, err, 256);
        LOGE("Error in fork: %s", err);
        exit(1);
    }
    else if(pid > 0)
    {
        exit(0);
    }

    if(setsid() < 0)
    {
        strerror_r(errno, err, 256);
        LOGE("Error in setsid: %s", err);
        exit(1);
    }
#endif
}

void handle_login_msg(struct connect_client *client, cJSON *jMsg, char *version)
{
    cJSON *root = NULL, *item = NULL;
    char *json_out = NULL;
    char errorBuf[STR_LEN_128] = {0}, random_str[STR_LEN_24] = {0};

    item = cJSON_GetObjectItem(jMsg, "random_num1");
    if ((item == NULL) || IS_EMPTY(item->valuestring) || (strlen(item->valuestring) != RANDOM_AUTH_STR_LEN))
    {
        strcpy(errorBuf, "{\"Result\":-2,\"FailReason\":\"random num1 error.\"}");
        goto error_done;     
    }
    else
    {
        root = cJSON_CreateObject();
        if (root == NULL)
        {
            strcpy(errorBuf, "{\"Result\":-1,\"FailReason\":\"Create JSON object error.\"}");
            goto error_done;                
        }
        
        cJSON_AddNumberToObject(root, "Result", 0);
        cJSON_AddStringToObject(root, "random_num1", item->valuestring);
        generate_random_str(random_str, RANDOM_AUTH_STR_LEN);
        strncpy(client->random_num2, random_str, RANDOM_AUTH_STR_LEN);
        memset(random_str, 0, sizeof(random_str));
        generate_random_str(random_str, RANDOM_ENC_KEY_LEN);
        cJSON_AddStringToObject(root, "random_num2", client->random_num2);
        cJSON_AddStringToObject(root, "version", version);
		cJSON_AddNumberToObject(root, "keepalive", alg.keepalive);
        json_out = cJSON_PrintUnformatted(root);
        if (json_out != NULL)
        {
            net_write(client->sock, json_out, strlen((const char*)json_out));
        }
        else
        {
            strcpy(errorBuf, "{\"Result\":-1,\"FailReason\":\"JSON string format or encrypt error.\"}");
            goto error_done;                
        }
    }

error_done:
    if (root != NULL)
        cJSON_Delete(root);
    alg_free(json_out); 

    if (strlen(errorBuf) > 0)
    {
        LOGE("Error: [%s]", errorBuf);
        net_write_error(client->sock, errorBuf);
        close_client(client->sock, client);
	}
    
    return;	   
}

void handle_start_setting(int algPort)
{
    InitRandom(algPort);
}

int config_parse_args(struct alg_db *db, int argc, char *argv[])
{
    int i = 0;
    int int_tmp = 0;

    for (i = 1; i < argc; i++)
    {
        if(!strcmp(argv[i], "-d") || !strcmp(argv[i], "--daemon"))
        {
			db->daemon = 1;
		}        
        else if(!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help"))
        {
			//print_usage();
			return 1;
		}
        else if(!strcmp(argv[i], "-v") || !strcmp(argv[i], "--version"))
        {
			printf("\nversion: %u\n", get_alg_ver());
			return 1;
		}
        else if(!strcmp(argv[i], "-k") || !strcmp(argv[i], "--keepalive"))
        {
			if (i < (argc - 1))
            {
				int_tmp = atoi(argv[i + 1]);
				if ((int_tmp < 60) || (int_tmp > 65535))
                {
					LOGE("Error: Invalid keepalive specified (%d), must between 60 ~ 65535", int_tmp);
					return 1;
				}
                else
                {
					db->keepalive = int_tmp;
				}
			}
            else
            {
				LOGE("Error: -k|--keepalive argument given, but no value specified.");
				return 1;
			}
			i++;
		}
        else if (!strcmp(argv[i], "-p") || !strcmp(argv[i], "--port"))
        {
			if (i < (argc - 1))
            {
				int_tmp = atoi(argv[i + 1]);
				if ((int_tmp < 5000) || (int_tmp > 65535))
                {
					LOGE("Error: Invalid port specified (%d), must between 5000 ~ 65535", int_tmp);
					return 1;
				}
                else
                {
					db->port = int_tmp;
					set_port(int_tmp);
				}
			}
            else
            {
				LOGE("Error: -p|--port argument given, but no port specified.");
				return 1;
			}
			i++;
        }   
        else if (!strcmp(argv[i], "-dj") || !strcmp(argv[i+1], "--#$sfyw576ffch2"))
        {
			i+= 2;
        }   
		else if(!strcmp(argv[i], "-r") || !strcmp(argv[i], "--reboot"))
		{
			LOGE("Error: reboot+++++++++++++++");
		}
        else
        {
			LOGE("Error: Unknown option '%s'.\n", argv[i]);
			//print_usage();
			return 1;
		}
	}

    return 0;
}

static void *alg_hb_thread()
{
    struct connect_client *client = NULL, *tmp = NULL;
	time_t now = time(NULL);

    while(1)
    {
		now = time(NULL);

        HASH_ITER(hh_sock, alg.c_client, client, tmp)
        {
			if ((client->remove == 0) && (now > client->last) && ((now - client->last) > ALG_KEEPALIVE * 3 / 2))  //3min
			{
				LOGE("Client [%s] more than %ds doesn't receieve data, disconnect it.", client->addr, ALG_KEEPALIVE);
                //close_client(client->sock, client);
                client->remove = 1;
			}
        }

		sleep(2);
    }
}


int alg_hb_thread_create(pthread_t *id)
{
    int ret = 0;

    ret = pthread_create(id, NULL, (void *)alg_hb_thread, NULL);
    if (ret != 0) {
        LOGE("Create jackpot sync pthread error!\n");
        return -1;
    }

    return 0;
}

void alg_heartbeat_msg(struct connect_client *client)
{
	char data[STR_LEN_128] = {0}, time_now[STR_LEN_24] = {0};
	
	snprintf(data, sizeof(data) - 1, "{\"MsgType\":%d,\"Result\":0,\"time\":\"%s\"}", srv_rm_hbpong, get_current_time(time_now)); 
	client->last = time(NULL);
	
	net_write(client->sock, data, strlen(data));

	return;
}

#ifndef __LINUX__
int epoll_create (int __size) {return 0;}
int epoll_ctl (int __epfd, int __op, int __fd,struct epoll_event *__event){return 0;}
int epoll_wait (int __epfd, struct epoll_event *__events, int __maxevents, int __timeout){return 0;}
int dzlog_init(const char *confpath, const char *cname){return 0;}
#endif
