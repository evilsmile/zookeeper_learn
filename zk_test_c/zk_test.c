#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <stdarg.h>
#include <unistd.h>
#include <pthread.h>

#include <zookeeper/zookeeper.h>

#include "logger.h"

#define BUF_SIZ 512
#define DISABLE_WATCHER 0
#define ENABLE_WATCHER 1

static char *username = NULL;
static char *passwd = NULL;
static zhandle_t *zh;

static int watcher_notified = 0;

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

int clean_node(char *root_path) 
{
    struct String_vector children;
    int i = 0;

    int error = zoo_get_children(zh, root_path, DISABLE_WATCHER, &children);
    if (error) {
        log_err("get root node children of '%s' failed: [%d]: %s", root_path, error, zerror(error));
        return;
    }

    int root_path_len = strlen(root_path);
    char wholepath[1024] = {0};
    for (i = 0; i < children.count; ++i) {
        memset(wholepath, 0, sizeof(wholepath));
        memcpy(wholepath, root_path, root_path_len);
        memcpy(wholepath + root_path_len, "/", 1);
        memcpy(wholepath + root_path_len + 1, children.data[i], strlen(children.data[i]));

        log_info("Delete [%d]: %s", i, wholepath);
        error = zoo_delete(zh, wholepath, -1);
        if (error) {
            log_err("delete children '%s' failed: [%d]: %s", children.data[i], error, zerror(error));
            continue;
        }
    }

    // clean root node
    error = zoo_delete(zh, root_path, -1);
    if (error) {
        log_err("delete node '%s' create failed: [%d]: %s", root_path, error, zerror(error));
        return -1;
    }

    return 0;
}

void test_unsafe_create_and_get()
{
    log_info("test unsafe functions -------------------");

	char buffer[BUF_SIZ];
    int buflen = BUF_SIZ;

    struct Stat stat;
    struct String_vector children;

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
    error = zoo_get(zh, test_root_node, DISABLE_WATCHER, buffer, &buflen, &stat);
    if (error) {
        log_err("Get node '%s' failed: [%d]:%s", test_root_node, error, zerror(error));
        goto clean;
    } 

    log_info("Get node '%s' succ. ctime: %s. mtime: %s. Value: [%s]", 
            test_root_node, 
            ctime((time_t*)&(stat.ctime)), 
            ctime((time_t*)&(stat.mtime)), 
            buffer
            );

    if (strncmp(buffer, test_node_value, buflen) != 0) {
        log_err("Get node value != value created!");
        goto clean;
    }

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

    error = zoo_get(zh, test_eph_node_path, DISABLE_WATCHER, buffer, &buflen, &stat);
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
    clean_node(test_root_node);
}



void test_auth_create_and_get()
{
    if (passwd != NULL) {
        if (zoo_add_auth(zh, username, passwd, strlen(passwd), 0, 0) != ZOK) {
            return;
        }
    }

    log_info("test auth functions -------------------");

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

enum cmd {
    SET_ROOT_VALUE = 0,
    ADD_CHILD = 1
};

typedef struct _help_args {
    int cmd;

    zhandle_t *zh;
    char *path;
    char *value;
} help_args;

void handle_set_root_value(zhandle_t* zh, char *root_path, char *new_value)
{
    if (zoo_set(zh, root_path, new_value, strlen(new_value), -1)) {
        log_err("[%s] set to new value failed.", root_path);
    } else {
        log_info("[%s] set to new value succ.", root_path);
    }
}

void handle_add_child(zhandle_t* zh, char *child_path, char *value)
{
    char buffer[512];
    if (zoo_create(zh, 
                  child_path, 
                  value, strlen(value), 
                  &ZOO_OPEN_ACL_UNSAFE,
                  0,
                  buffer, sizeof(buffer) - 1
                  )) {
        log_err("create [%s] failed.", child_path);
    } else {
        log_info("create [%s] succ.", child_path);
    }
}

void* handle_cmd(void *arg)
{
    help_args* p_args = (help_args*)arg;

    int cmd = p_args->cmd;

    switch(cmd) {
    case SET_ROOT_VALUE:
        handle_set_root_value(p_args->zh, p_args->path, p_args->value);
        break;
    case ADD_CHILD:
        handle_add_child(p_args->zh, p_args->path, p_args->value);
        break;
        
    default:
        log_err("unknown cmd: %d", cmd);
        break;
    }

}
void request_theother(help_args* args)
{
    pthread_t tid;
    pthread_create(&tid, NULL, handle_cmd, (void*)args);
    int *res = 0;
    pthread_join(tid, (void**)&res);
}

void watcher_root_cb(zhandle_t* zh, int type, int state, const char* path, void *watchCtx)
{
    log_info("test root notified by: type:%d state:%d path:%s", type, state, path);

    watcher_notified = 1;
}

void watcher_child_cb(zhandle_t* zh, int type, int state, const char* path, void *watchCtx)
{
    log_info("watcher child notified by: type:%d state:%d path:%s", type, state, path);

    watcher_notified = 1;
}

void test_watcher()
{
    log_info("test watcher functions -------------------");

    char buffer[BUF_SIZ];
    int buflen = BUF_SIZ;
    struct String_vector children;

    char *watcher_root = "/test_watcher";
    char *node_value = "test watcher";

    int error = zoo_create(zh,
            watcher_root,
            node_value, strlen(node_value),
            &ZOO_OPEN_ACL_UNSAFE,
            0,
            buffer, sizeof(buffer) - 1
            );
    if (error) {
        log_err("'%s' create failed: [%d]: %s", watcher_root, error, zerror(error));
    } else {
        log_info("'%s' create succ", watcher_root);
    }

    error = zoo_wget(zh, watcher_root, watcher_root_cb, NULL, buffer, &buflen, NULL);
    if (error) {
        log_err("'%s' get failed: [%d]: %s", watcher_root, error, zerror(error));
        goto clean;
    } else {
        log_info("'%s' get succ", watcher_root);
    }                  

    help_args args;
    args.cmd = SET_ROOT_VALUE;
    args.zh = zh;
    args.path = watcher_root;
    args.value = "root new value";
    request_theother(&args);

    // wait for theother done
    sleep(1);

    if (watcher_notified == 1) {
        log_info("root watcher get notified succ");
        watcher_notified = 0;
    } else {
        log_info("root watcher get notified failed");
        goto clean;
    }

    // test child watcher
    error = zoo_wget_children(zh, watcher_root, watcher_child_cb, NULL, &children);

    args.cmd = ADD_CHILD;
    args.zh = zh;
    args.path = "/test_watcher/watcher_child1";
    args.value = "watcher child 1";
    request_theother(&args);

    sleep(1);
 
    if (watcher_notified == 1) {
        log_info("child watcher get notified succ");
    } else {
        log_info("child watcher get notified failed");
        goto clean;
    }           

clean:
    clean_node(watcher_root);
}

void lets_test()
{
    test_unsafe_create_and_get();
    
    if (passwd != NULL) {
        test_auth_create_and_get();
    }

    test_watcher();
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
