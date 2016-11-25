#ifndef __HELPER_H__
#define __HELPER_H__

#include "zk_wrapper.h"

enum {
    CREATE_NODE = 0,
    SET_VALUE = 1
};

typedef struct _help_args {
    int cmd;

    ZkWrapper *zw;
    std::string path;
    std::string value;
} help_args;

void request_theother(help_args* args);

#endif
