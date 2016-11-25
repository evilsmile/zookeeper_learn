#ifndef __ZK_WRAPPER_H__
#define __ZK_WRAPPER_H__

#include <zookeeper/zookeeper.h>
#include <string>
#include <vector>

class ZkWrapper {
public:
    ZkWrapper(){}
    ~ZkWrapper();
    int Init(const std::string& server, watcher_fn watcher_cb, ZooLogLevel log_level = ZOO_LOG_LEVEL_ERROR);

    int Exists(const std::string& path, int watch_flag);
    int ExistsW(const std::string& path, watcher_fn watcher_cb, void *watcher_ctx);

    int CreateNode(const std::string& path, const std::string& value);
    int CreateEphemeralNode(const std::string& path, const std::string& value);
    int CreateSequenceNode(const std::string& path, const std::string& value);

    std::string GetValue(const std::string& path, int watch_flag);
    std::string GetValueW(const std::string& path, watcher_fn watcher_cb, void *watch_ctx);

    int GetChildren(const std::string& path, int watch_flag, std::vector<std::string>& children_out);
    int GetChildrenW(const std::string& path, watcher_fn watcher_cb, void *watch_ctx, std::vector<std::string>& children_out);

    int SetValue(const std::string& path, const std::string& value);

    std::string GetErrstr() const;

    int RemoveNode(const std::string& path);
    int Auth(const std::string& username, const std::string& passwd);

private:
    int _create_node(const std::string& path, const std::string& value, int node_type);

    zhandle_t *_zh;
    std::string _username;
    std::string _passwd;
    std::string _server;
    std::string _errstr;

    watcher_fn _watcher_cb;
    struct Stat _stat;

};

#endif
