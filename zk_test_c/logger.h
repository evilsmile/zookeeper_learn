#ifndef __LOGGER_H__
#define __LOGGER_H__
    
#define log_err(...) log_api("error", __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)
#define log_info(...) log_api("info", __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)
#define log_debug(...) log_api("debug", __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)

extern void log_api(char* tag, char *file, int line, const char *function, char *msg, ...) ;

#endif
