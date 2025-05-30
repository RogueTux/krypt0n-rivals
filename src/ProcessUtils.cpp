#include "../include/ProcessUtils.h"
#include "../include/Memory.h"

#include <iostream>
#include <cstring>
#include <cstdio>
#include <string>

pid_t FindProcess(const char* processName)
{
    std::string cmd = "pidof " + std::string(processName);
    FILE* cmd_pipe = popen(cmd.c_str(), "r");

    if (!cmd_pipe)
        return 0;

    char buffer[128];
    std::string result;

    if (fgets(buffer, sizeof(buffer), cmd_pipe) != nullptr)
        result = buffer;

    pclose(cmd_pipe);

    if (result.empty())
    {
        cmd = "pgrep -f " + std::string(processName);
        cmd_pipe = popen(cmd.c_str(), "r");

        if (!cmd_pipe)
            return 0;

        if (fgets(buffer, sizeof(buffer), cmd_pipe) != nullptr)
            result = buffer;

        pclose(cmd_pipe);
    }

    pid_t pid = 0;
    if (!result.empty())
    {
        try {
            pid = std::stoi(result);
        } catch (...) {
            return 0;
        }
    }

    return pid;
}

long FindBaseImage()
{
    if (ProcessId == 0)
        return 0;

    char mapsPath[64];
    snprintf(mapsPath, sizeof(mapsPath), "/proc/%d/maps", ProcessId);

    FILE* maps = fopen(mapsPath, "r");
    if (!maps)
        return 0;

    char line[512];
    long baseAddress = 0;

    while (fgets(line, sizeof(line), maps))
    {
        if (strstr(line, "Marvel-Win64-Shipping.exe") != nullptr ||
            strstr(line, "marvel-win64-shipping") != nullptr)
        {
            unsigned long start;
            if (sscanf(line, "%lx-", &start) == 1)
            {
                baseAddress = static_cast<long>(start);
                break;
            }
        }
    }

    fclose(maps);
    return baseAddress;
}