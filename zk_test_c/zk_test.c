#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>
#include <zookeeper/zookeeper.h>

static zhandle_t *zh;

void log_api(char* tag, char *file, int line, const char *function, char *msg, ...) 
{
    char buf[1024]; 
    int n = snprintf(buf, sizeof(buf), "[%s][%s:%d][%s]", tag, file, line, function);
    va_list argp;    
    va_start(argp, msg); 
    int ret = vsnprintf(buf + n, sizeof(buf) - n, msg, argp);  
    va_end(argp);           

    printf("%s\n", buf);
}
    
#define log_err(...) log_api("error", __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)
#define log_info(...) log_api("info", __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)
#define log_debug(...) log_api("debug", __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)

char *foo_get_cert_once(char* id) { return 0; }

void watcher(zhandle_t *zzh, int type, int state, const char *path, void *watcherCtx) 
{
	log_info("state: %d, path: %s, type: %d", state, path, type);
}

int main(int argc, char *argv[])
{
	char buffer[512];
	char p[2048];
	char *cert = 0;
	char appId[64];

	strcpy(appId, "example.foo_test");
	cert = foo_get_cert_once(appId);
	if (cert != 0) {
		fprintf(stderr, 
				"Certficate for appid [%s] is [%s]\n", appId, cert);
		strncpy(p, cert, sizeof(p) - 1);
		free(cert);
	} else {
		fprintf(stderr, 
				"Certficate for appid [%s] not found! Use default one\n", appId);
		strcpy(p, "dummy");
	}

	zoo_set_debug_level(ZOO_LOG_LEVEL_DEBUG);

	zh = zookeeper_init("192.168.0.40:2181", watcher, 1000, 0, 0, 0);
    //zh = zookeeper_init("192.168.0.40:2181,localhost:2182,localhost:2183/xyz", watcher, 1000, 0, 0, 0);
    if (!zh) {
        return errno;
    }

    //	if (zoo_add_auth(zh, "foo", p, strlen(p), 0, 0) != ZOK) {
    //		return 2;
    //	}

    //	struct ACL CREATE_ONLY_ACL[] = {{ZOO_PERM_CREATE, ZOO_AUTH_IDS}};
    struct ACL CREATE_ONLY_ACL[] = {{ZOO_PERM_CREATE, ZOO_ANYONE_ID_UNSAFE}};
    struct ACL_vector CREATE_ONLY = {1, CREATE_ONLY_ACL};
    //int rc = zoo_create(zh, "/xyz", "value", 5, &ZOO_OPEN_ACL_UNSAFE, ZOO_SEQUENCE,
    int rc = zoo_create(zh, "/xyz", "value", 5, &CREATE_ONLY, ZOO_EPHEMERAL,
            buffer, sizeof(buffer) - 1);
    if (rc) {
        log_err("/xyz create failed: %d", rc);
        goto err;
    }
    int buflen = sizeof(buffer);
    struct Stat stat;
    rc = zoo_get(zh, "/xyz", 0, buffer, &buflen, &stat);
    if (rc) {
        log_err("Get /xyz failed: %d", rc);
        goto err;
    }

err:
    zookeeper_close(zh);

	return 0;
}
