#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <pthread.h>
#include <zookeeper/zookeeper.h>

#include "logger.h"
#include "helper.h"

static void* handle_cmd(void *arg)
{
    help_args* p_args = (help_args*)arg;

    int cmd = p_args->cmd;

    switch(cmd) {
    case CREATE_NODE:
        p_args->zw->CreateNode(p_args->path, p_args->value);
        break;
    case SET_VALUE:
        p_args->zw->SetValue(p_args->path, p_args->value);
        break;
        
    default:
        log_err("unknown cmd: %d", cmd);
        break;
    }

    return arg;
}

void request_theother(help_args* args)
{
    pthread_t tid;
    pthread_create(&tid, NULL, handle_cmd, (void*)args);
    int *res = 0;
    pthread_join(tid, (void**)&res);
}
