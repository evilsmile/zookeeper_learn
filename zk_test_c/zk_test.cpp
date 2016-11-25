#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <errno.h>
#include <stdarg.h>
#include <unistd.h>

#include <zookeeper/zookeeper.h>

#include <vector>

#include "global.h"
#include "logger.h"
#include "helper.h"
#include "watcher_define.h"
#include "zk_wrapper.h"

static ZkWrapper zk_wrapper;

int watcher_notified = 0;

void test_unsafe_create_and_get()
{
    log_info("--------------- [ TEST unsafe functions ] -------------------");

    std::string root_node = "/test2";
    do {
        // Create root node
        std::string value = "test value";

        if (zk_wrapper.CreateNode(root_node, value) < 0) {
            log_err("'%s' create failed: [%s]", root_node.c_str(), zk_wrapper.GetErrstr().c_str());
        } else {
            log_info("create node [%s] succ.", root_node.c_str());
        }

        // Get root node value
        std::string get_value = zk_wrapper.GetValue(root_node, DISABLE_WATCHER);
        if (get_value.empty()) {
            log_err("Get node '%s' failed: [%s]", 
                    root_node.c_str(),
                    zk_wrapper.GetErrstr().c_str()
                   );
            break;
        } 

        log_info("Get node '%s' succ. Value: [%s]", 
                root_node.c_str(), value.c_str());

        if (get_value != value) {
            log_err("Get node value != value created!");
            break;
        }

        // test Ephemeral node
        std::string ephemeral_node = "/test2/ephemeral";
        if (zk_wrapper.CreateEphemeralNode(ephemeral_node , "Ephemeral value") < 0) {
            log_err("create ephemeral node failed: [%s]", zk_wrapper.GetErrstr().c_str() );
            break;
        } else { 
            log_info("ephemeral node create succ");
        }

        value = zk_wrapper.GetValue(ephemeral_node, DISABLE_WATCHER);
        if (value.empty()) {
            log_err("Get ephemeral node '%s' failed: [%s]", ephemeral_node.c_str(), zk_wrapper.GetErrstr().c_str());
            break;
        } 

        // test Sequence node
        std::string test_seq_node_path = "/test2/seq_";
        if (zk_wrapper.CreateSequenceNode(test_seq_node_path, "seq value") < 0) {
            log_err("sequence node '%s' create failed: [%s]", test_seq_node_path.c_str(), zk_wrapper.GetErrstr().c_str());
            break;
        }

        log_info("sequence node create node [%s] succ", test_seq_node_path.c_str());

    } while(0);

//    zk_wrapper.RemoveNode(root_node);
}

void check_watcher(const std::string& header)
{
    sleep(1);

    if (watcher_notified == 1) {
        log_info("[%s] get notified succ", header.c_str());
        watcher_notified = 0;
    } else {
        log_info("[%s] get notified failed", header.c_str());
    }           
}

void test_watcher()
{
    log_info("------------------- [ test watcher functions ] -------------------");

    std::string watcher_root = "/test_watcher2";
    do {
        int error = zk_wrapper.ExistsW(watcher_root, watcher_root_exist_cb, NULL);
        if (error) {
            log_err("check '%s' exist: %s", 
                    watcher_root.c_str(), zk_wrapper.GetErrstr().c_str());
        } else {

        }

        help_args args;

        args.cmd = CREATE_NODE;
        args.zw = &zk_wrapper;
        args.path = watcher_root;
        args.value = "root init value";
        request_theother(&args);
        check_watcher("root");

        std::string value = zk_wrapper.GetValueW(watcher_root, watcher_root_cb, NULL);
        if (value.empty()) {
            log_err("'%s' get failed: [%s]", watcher_root.c_str(), zk_wrapper.GetErrstr().c_str());
            break;
        } else {
            log_info("'%s' get succ", watcher_root.c_str());
        }                  

        args.cmd = SET_VALUE;
        args.zw = &zk_wrapper;
        args.path = watcher_root;
        args.value = "root new value";
        request_theother(&args);
        check_watcher("root");

        // test child watcher
        std::vector<std::string> children;
        error = zk_wrapper.GetChildrenW(watcher_root, watcher_child_cb, NULL, children);
        if (error) {
            log_err("get children of '%s' failed: [%s]", watcher_root.c_str(), zk_wrapper.GetErrstr().c_str());
            break;
        }

        args.cmd = CREATE_NODE;
        args.zw = &zk_wrapper;
        args.path = "/test_watcher2/watcher_child1";
        args.value = "watcher child 1";
        request_theother(&args);
        check_watcher("child of root");
    } while(0);
 
//    zk_wrapper.RemoveNode(watcher_root);
}

void lets_test()
{
    test_unsafe_create_and_get();
    
    test_watcher();
}

int main(int argc, char *argv[])
{
#define TEST_AUTH 1
#if TEST_AUTH
    std::string scheme = "digest";
    std::string username = "sss";
    std::string passwd = "2ss";
#else
    std::string scheme;
    std::string username;
    std::string passwd;
#endif

    if (zk_wrapper.Init("192.168.0.40:2181,192.168.0.40:2182,192.168.0.40:2183", global_watcher, scheme, username, passwd) < 0) {
        log_err("zk init failed.");
        return -1;
    }

    std::string cert = username + ":" + passwd;
    if (zk_wrapper.Auth(scheme, cert) < 0) {
        log_err("auth failed! [%s]", zk_wrapper.GetErrstr().c_str());
    } else {
        log_err("auth OK!");
    }

    lets_test();

	return 0;
}
