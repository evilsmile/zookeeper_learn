#include <cstring>

#include "zk_wrapper.h"
#include "util.h"
#include "logger.h"

ZkWrapper::~ZkWrapper()
{
    if (_zh != NULL) {
        zookeeper_close(_zh);
    }
}

int ZkWrapper::Init(const std::string& server, watcher_fn watcher_cb, const std::string& scheme, const std::string& username, const std::string& passwd, ZooLogLevel log_level)
{
    zoo_set_debug_level(log_level);

    _watcher_cb = watcher_cb;
    _server = server;
    _scheme = scheme;
    _username = username;
    _passwd = passwd;

    _passwd_enc = util::base64_encode(util::sha1sum(username + ":" + passwd));    // Base64(Sha1sum('username:passwd'))

    _zh = zookeeper_init(_server.c_str(), _watcher_cb, 100000, 0, 0, 0);
    if (!_zh) {
        return -1;
    }

    return 0;
}

int ZkWrapper::Auth(const std::string& scheme, const std::string& cert)
{
    int error = zoo_add_auth(_zh, scheme.c_str(), cert.c_str(), cert.size(), 0, 0);
    if (error) {
        _errstr = zerror(error);
        return -1;
    }

    return 0;
}

std::string ZkWrapper::GetErrstr() const
{
    return _errstr;
}

int ZkWrapper::Exists(const std::string& path, int watch_flag)
{
    int error = zoo_exists(_zh, path.c_str(), watch_flag, &_stat);  
    if (error < 0) {
        _errstr = zerror(error);
        return -1;
    }

    return 0;
}

int ZkWrapper::ExistsW(const std::string& path, watcher_fn watcher_cb, void *watcher_ctx)
{
    int error = zoo_wexists(_zh, path.c_str(), watcher_cb, watcher_ctx, &_stat);  
    if (error < 0) {
        _errstr = zerror(error);
        return -1;
    }

    return 0;
}

int ZkWrapper::CreateNode(const std::string& path, const std::string& value)
{
    return _create_node(path, value, 0);
}

int ZkWrapper::CreateEphemeralNode(const std::string& path, const std::string& value)
{
    return _create_node(path, value, ZOO_EPHEMERAL);
}
int ZkWrapper::CreateSequenceNode(const std::string& path, const std::string& value)
{
    return _create_node(path, value, ZOO_SEQUENCE);
}

std::string ZkWrapper::GetValue(const std::string& path, int watch_flag)
{
    char buffer[512];
    int buflen = 512;

    int error = zoo_get(_zh, path.c_str(), watch_flag, buffer, &buflen, &_stat);
    if (error) {
        _errstr = zerror(error);
        return "";
    } 

    return std::string(buffer, buflen);
}

std::string ZkWrapper::GetValueW(const std::string& path, watcher_fn watcher_cb, void *watch_ctx)
{
    char buffer[512];
    int buflen = 512;

    int error = zoo_wget(_zh, path.c_str(), watcher_cb, watch_ctx, buffer, &buflen, &_stat);
    if (error) {
        _errstr = zerror(error);
        return "";
    } 

    return std::string(buffer, buflen);
}

int ZkWrapper::SetValue(const std::string& path, const std::string& new_value)
{
    int error = zoo_set(_zh, path.c_str(), new_value.c_str(), new_value.size(), -1);
    if (error) {
        _errstr = zerror(error);
        return -1;
    } 

    return 0;
}

int ZkWrapper::RemoveNode(const std::string& path)
{
    struct String_vector children;

    int error = zoo_get_children(_zh, path.c_str(), 0, &children);
    if (error) {
        _errstr = zerror(error);
        return -1;
    }

    std::string whole_path;
    for (int i = 0; i < children.count; ++i) {
        whole_path = path;
        whole_path += "/";
        whole_path += std::string(children.data[i], strlen(children.data[i]));

        error = zoo_delete(_zh, whole_path.c_str(), -1);
        if (error) {
            _errstr = zerror(error);
            return -1;
        }
    }

    error = zoo_delete(_zh, path.c_str(), -1);
    if (error) {
        _errstr = zerror(error);
        return -1;
    }

    return 0;
}

int ZkWrapper::GetChildren(const std::string& path, int watch_flag, std::vector<std::string>& children_out)
{
    struct String_vector children;

    int error = zoo_get_children(_zh, path.c_str(), watch_flag, &children);
    if (error) {
        _errstr = zerror(error);
        return -1;
    }

    std::string whole_path;
    for (int i = 0; i < children.count; ++i) {
        children_out.push_back(std::string(children.data[i], strlen(children.data[i])));
    }

    return 0;
}

int ZkWrapper::GetChildrenW(const std::string& path, watcher_fn watcher_cb, void *watch_ctx, std::vector<std::string>& children_out)
{
    struct String_vector children;

    int error = zoo_wget_children(_zh, path.c_str(), watcher_cb, watch_ctx, &children);
    if (error) {
        _errstr = zerror(error);
        return -1;
    }

    std::string whole_path;
    for (int i = 0; i < children.count; ++i) {
        children_out.push_back(std::string(children.data[i], strlen(children.data[i])));
    }

    return 0;
}

int ZkWrapper::_create_node(const std::string& path, const std::string& value, int node_type)
{
    char buffer[512];

//    struct ACL CREATE_ONLY_ACL_UNSAFE[] = {{ZOO_PERM_CREATE, ZOO_ANYONE_ID_UNSAFE}};
//    struct ACL_vector CREATE_ONLY = {1, CREATE_ONLY_ACL_UNSAFE};

    struct ACL_vector acl;
    if (!_scheme.empty() && !_username.empty() && !_passwd.empty()) {
        struct Id id = {
            (char*)_scheme.c_str(), 
            (char*)(_username + ":" + _passwd_enc).c_str()
        };
        struct ACL all_acl[] = {{ZOO_PERM_ALL, id}};
        acl = {1, all_acl};
    } else {
        acl = ZOO_OPEN_ACL_UNSAFE;
    }

    int error = zoo_create(_zh,
            path.c_str(),
            value.c_str(), value.size(),
            &acl,
            node_type,
            buffer, sizeof(buffer) - 1
            );
    if (error < 0) {
        _errstr = zerror(error);
        return -1;
    }

    return 0;
}
