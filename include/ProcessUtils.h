#ifndef PROCESS_UTILS_H
#define PROCESS_UTILS_H

#include <sys/types.h>

pid_t FindProcess(const char* processName);
long FindBaseImage();

#endif