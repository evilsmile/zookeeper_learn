#include <stdarg.h>
#include <stdio.h>

void log_api(char* tag, char *file, int line, const char *function, char *msg, ...) 
{
    char buf[1024]; 
    int n = snprintf(buf, sizeof(buf), "[%s][%s:%d][%s]: ", tag, file, line, function);
    va_list argp;    
    va_start(argp, msg); 
    int ret = vsnprintf(buf + n, sizeof(buf) - n, msg, argp);  
    va_end(argp);           

    printf("%s\n", buf);
}
