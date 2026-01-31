#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>

#include "utils.h"
#include "rng.h"
#include "test/testMain.h"

struct alg_db {
    int port;
};
struct alg_db alg;

void alg_sigpipe(int signal)
{
    LOGI("#######receive sigpipe signal, ignore###########");
}

void alg_daemonise(void)
{
#ifdef __LINUX__
    char err[256] = {0};
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

int config_parse_args(struct alg_db *db, int argc, char *argv[])
{
    int i = 0;
    int int_tmp = 0;

    for (i = 1; i < argc; i++)
    {
        if (!strcmp(argv[i], "-p") || !strcmp(argv[i], "--port"))
        {
			if (i < (argc - 1))
            {
				int_tmp = atoi(argv[i + 1]);
				if ((int_tmp < 5000) || (int_tmp > 65535))
                {
					printf("Error: Invalid port specified (%d), must between 5000 ~ 65535", int_tmp);
					return 1;
				}
                else
                {
					db->port = int_tmp;
				}
			}
            else
            {
				printf("Error: -p|--port argument given, but no port specified.");
				return 1;
			}
			i++;
        }   
		else if(!strcmp(argv[i], "-r") || !strcmp(argv[i], "--reboot"))
		{
			printf("Error: reboot+++++++++++++++");
		}
        else
        {
			printf("Error: Unknown option '%s'.\n", argv[i]);
			return 1;
		}
	}

    return 0;
}

int main(int argc, char *argv[])
{
	signal(SIGPIPE, alg_sigpipe);

	memset(&alg, 0, sizeof(struct alg_db));
	if (config_parse_args(&alg, argc, argv) != 0)
    {
        return 1;
    }  

	char log_dir[64] = {0};
	snprintf(log_dir, sizeof(log_dir) - 1, "log%d.conf", alg.port);
	FILE *fp = fopen(log_dir, "w+");
	fprintf(fp, "[rules]\n");
	fprintf(fp, "*.*	\"/home/syc/log/rng_%d.log\",    20MB * 1", alg.port);
	fclose(fp);

	if (dzlog_init(log_dir, "my_cat")) {
		printf("zlog init failed\n");
		return -1;
	}

	LOGI("##########################################################");
    LOGI("#\n#\n#      Current alg platform server version is %d\n#\n#", ALG_VERSION);
    LOGI("##########################################################\n");

#ifndef __DEBUG__
    alg_daemonise();
#endif

	RandInit();

#ifdef __DEBUG__
		TestMain();
#endif

    RngMainLoop(alg.port);

    LOGE("###########EXIT##############\n");

    return 0;
}