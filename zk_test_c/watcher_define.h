#ifndef __WATCHER_DEFINE_H__
#define __WATCHER_DEFINE_H__

#include <zookeeper/zookeeper.h>

void global_watcher(zhandle_t *zh, int type, int state, const char *path, void *watcherCtx);
void watcher_root_cb(zhandle_t* zh, int type, int state, const char* path, void *watchCtx);
void watcher_child_cb(zhandle_t* zh, int type, int state, const char* path, void *watchCtx);
void watcher_root_exist_cb(zhandle_t* zh, int type, int state, const char* path, void *watchCtx);

#endif
