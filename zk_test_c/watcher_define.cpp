#include <string>

#include "logger.h"
#include "watcher_define.h"
#include "global.h"

extern int watcher_notified;

std::string EVENT[] = {
    "ZOO_SESSION_EVENT",
    "ZOO_NOTWATCHING_EVENT",
    "ZOO_CREATED_EVENT",
    "ZOO_DELETED_EVENT",
    "ZOO_CHANGED_EVENT",
    "ZOO_CHILD_EVENT"                
};

void global_watcher(zhandle_t *zh, int type, int state, const char *path, void *watcherCtx) 
{
	log_info("state: %d, path: %s, type: %s", state, (char*)path, EVENT[type + 2].c_str());
}

void watcher_root_cb(zhandle_t* zh, int type, int state, const char* path, void *watchCtx)
{
    log_info("test root notified by: type:%s state:%d path:%s",  EVENT[type + 2].c_str(), state, (char*)path);

    watcher_notified = 1;
}

void watcher_child_cb(zhandle_t* zh, int type, int state, const char* path, void *watchCtx)
{
    log_info("watcher child notified by: type:%s state:%d path:%s", EVENT[type + 2].c_str(), state, (char*)path);

    watcher_notified = 1;
}

void watcher_root_exist_cb(zhandle_t* zh, int type, int state, const char* path, void *watchCtx)
{
    log_info("root exists notified by: type:%s state:%d path:%s", EVENT[type + 2].c_str(), state, (char*)path);

    watcher_notified = 1;
}
