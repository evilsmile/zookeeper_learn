#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <stdarg.h>
#include <zookeeper/zookeeper.h>

#define BUF_SIZ 512

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

static char *username = NULL;
static char *passwd = NULL;

void watcher(zhandle_t *zzh, int type, int state, const char *path, void *watcherCtx) 
{
	log_info("state: %d, path: %s, type: %d", state, path, type);
}

int init_zk()
{
    zoo_set_debug_level(ZOO_LOG_LEVEL_ERROR);

    zh = zookeeper_init("192.168.0.40:2181", watcher, 100000, 0, 0, 0);
    if (!zh) {
        log_err("zk init failed.");
        return -1;
    }

    return 0;
}

void test_unsafe_create_and_get()
{
	char buffer[BUF_SIZ];
    int buflen = BUF_SIZ;

    struct Stat stat;
    struct String_vector children;

    int i = 0;

    struct ACL CREATE_ONLY_ACL_UNSAFE[] = {{ZOO_PERM_CREATE, ZOO_ANYONE_ID_UNSAFE}};
    struct ACL_vector CREATE_ONLY = {1, CREATE_ONLY_ACL_UNSAFE};

    // Create root node
    char *test_root_node = "/test";
    char *test_node_value = "test_value";

    memset(buffer, 0, sizeof(buffer));
    int error = zoo_create(zh, 
                           test_root_node, 
                           test_node_value, strlen(test_node_value), 
                           &ZOO_OPEN_ACL_UNSAFE, 
                           0, 
                           buffer, sizeof(buffer) - 1
                           );
    if (error) {
        log_err("'%s' create failed: [%d]: %s", test_root_node, error, zerror(error));
    } else {
        log_info("create node [%s] succ. Ret path: %s", test_root_node, buffer);
    }

    memset(buffer, 0, sizeof(buffer));

    // Get root node value
    error = zoo_get(zh, test_root_node, 0, buffer, &buflen, &stat);
    if (error) {
        log_err("Get node '%s' failed: [%d]:%s", test_root_node, error, zerror(error));
        goto clean;
    } 

    log_info("Get node '%s' succ. ctime: %s. Value: [%s]", test_root_node, localtime((time_t*)&(stat.ctime)), buffer);

    // test Ephemeral node
    char *test_eph_node_path = "/test/ephemeral";
    test_node_value = "test_ephemeral_value";

    error = zoo_create(zh, 
                       test_eph_node_path, 
                       test_node_value, strlen(test_node_value), 
                       &ZOO_OPEN_ACL_UNSAFE, 
                       ZOO_EPHEMERAL, 
                       buffer, sizeof(buffer) - 1
                       );
    if (error) {
        log_err("ephemeral node '%s' create failed: [%d]: %s", test_eph_node_path, error, zerror(error));
        goto clean;
    }

    log_info("ephemeral node create node [%s] succ", test_eph_node_path);

    memset(buffer, 0, sizeof(buffer));

    error = zoo_get(zh, test_eph_node_path, 0, buffer, &buflen, &stat);
    if (error) {
        log_err("Get node '%s' failed: [%d]:%s", test_eph_node_path, error, zerror(error));
        goto clean;
    } 

    // test Sequence node
    char *test_seq_node_path = "/test/seq_";
    test_node_value = "test_seq_value";

    error = zoo_create(zh, 
                      test_seq_node_path, 
                      test_node_value, strlen(test_node_value), 
                      &ZOO_OPEN_ACL_UNSAFE, 
                      ZOO_SEQUENCE, 
                      buffer, sizeof(buffer) - 1
                      );
    if (error) {
        log_err("sequence node '%s' create failed: [%d]: %s", test_seq_node_path, error, zerror(error));
        goto clean;
    }

    log_info("sequence node create node [%s] succ", test_seq_node_path);

clean:
    // clean all children
    error = zoo_get_children(zh, test_root_node, 0, &children);
    if (error) {
        log_err("get root node children of '%s' failed: [%d]: %s", test_eph_node_path, error, zerror(error));
        return;
    }

    int test_root_node_len = strlen(test_root_node);
    char wholepath[1024] = {0};
    for (i = 0; i < children.count; ++i) {
        memset(wholepath, 0, sizeof(wholepath));
        memcpy(wholepath, test_root_node, test_root_node_len);
        memcpy(wholepath + test_root_node_len, "/", 1);
        memcpy(wholepath + test_root_node_len + 1, children.data[i], strlen(children.data[i]));

        log_info("Delete [%d]: %s", i, wholepath);
        error = zoo_delete(zh, wholepath, -1);
        if (error) {
            log_err("delete children '%s' failed: [%d]: %s", children.data[i], error, zerror(error));
            continue;
        }
    }

    // clean root node
    error = zoo_delete(zh, test_root_node, -1);
    if (error) {
        log_err("delete node '%s' create failed: [%d]: %s", test_root_node, error, zerror(error));
        return;
    }

}

void test_auth_create_and_get()
{
    if (passwd != NULL) {
        if (zoo_add_auth(zh, username, passwd, strlen(passwd), 0, 0) != ZOK) {
            return;
        }
    }
	char buffer[BUF_SIZ];
    int buflen = BUF_SIZ;

    struct Stat stat;


    struct ACL CREATE_ONLY_ACL_AUTH[] = {{ZOO_PERM_CREATE, ZOO_AUTH_IDS}};
    struct ACL_vector CREATE_ONLY = {1, CREATE_ONLY_ACL_AUTH};

    char *test_root_node = "/auth_test";
    char *test_node_value = "auth_value";
    int error = zoo_create(zh, 
                           test_root_node, 
                           test_node_value, strlen(test_node_value), 
                           &CREATE_ONLY, 
                           ZOO_EPHEMERAL, 
                           buffer, sizeof(buffer) - 1
                           );
    if (error) {
        log_err("'%s' create failed: [%d]: %s", test_root_node, error, zerror(error));
        return;
    }
    log_info("%s create succ", test_root_node);
}

void lets_test()
{
    test_unsafe_create_and_get();
    
    if (passwd != NULL) {
        test_auth_create_and_get();
    }

}

void close_zk()
{
    if (zh != NULL) {
        zookeeper_close(zh);
    }
}

int main(int argc, char *argv[])
{
    init_zk();

    lets_test();

    close_zk();

	return 0;
}
