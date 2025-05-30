#include "../../include/tabs/dashboard.h"
#include "../../include/FeatureManager.h"
#include <imgui.h>
#include <sys/sysinfo.h>
#include <fstream>
#include <string>
#include <cstdio>

#include "EntityScanner.h"
#include "Memory.h"

float getCpuUsage() {
    static float currentCpuUsage = 0.0f;
    static double lastCpuUpdateTime = -1.0;

    if (ImGui::GetTime() - lastCpuUpdateTime < 1.0 && lastCpuUpdateTime != -1.0) {
        return currentCpuUsage;
    }

    lastCpuUpdateTime = ImGui::GetTime();

    static unsigned long long lastTotalUser = 0, lastTotalUserLow = 0, lastTotalSys = 0, lastTotalIdle = 0;

    std::ifstream statFile("/proc/stat");
    if (!statFile.is_open()) {
        return currentCpuUsage;
    }

    std::string line;
    std::getline(statFile, line);
    statFile.close();

    unsigned long long totalUser=0, totalUserLow=0, totalSys=0, totalIdle=0, totalIoWait=0, totalIrq=0, totalSoftIrq=0, totalSteal=0;
    sscanf(line.c_str(), "cpu %llu %llu %llu %llu %llu %llu %llu %llu",
           &totalUser, &totalUserLow, &totalSys, &totalIdle, &totalIoWait, &totalIrq, &totalSoftIrq, &totalSteal);

    if (lastTotalUser == 0) {
        lastTotalUser = totalUser;
        lastTotalUserLow = totalUserLow;
        lastTotalSys = totalSys;
        lastTotalIdle = totalIdle;
        currentCpuUsage = 0.0f;
        return currentCpuUsage;
    }

    unsigned long long totalUserDiff = totalUser - lastTotalUser;
    unsigned long long totalUserLowDiff = totalUserLow - lastTotalUserLow;
    unsigned long long totalSysDiff = totalSys - lastTotalSys;
    unsigned long long totalIdleDiff = totalIdle - lastTotalIdle;

    unsigned long long totalActiveDiff = totalUserDiff + totalUserLowDiff + totalSysDiff;

    if (unsigned long long totalDiff = totalActiveDiff + totalIdleDiff; totalDiff > 0) {
        currentCpuUsage = static_cast<float>(totalActiveDiff) / totalDiff * 100.0f;
    } else {
        currentCpuUsage = 0.0f;
    }

    lastTotalUser = totalUser;
    lastTotalUserLow = totalUserLow;
    lastTotalSys = totalSys;
    lastTotalIdle = totalIdle;

    if (currentCpuUsage < 0.0f) currentCpuUsage = 0.0f;
    if (currentCpuUsage > 100.0f) currentCpuUsage = 100.0f;

    return currentCpuUsage;
}

float getMemoryUsage() {
    struct sysinfo memInfo{};
    if (sysinfo(&memInfo) != 0) {
        return 0.0f;
    }

    unsigned long long cachedMem = 0;
    std::ifstream memFile("/proc/meminfo");
    if (memFile.is_open()) {
        std::string line;
        while (std::getline(memFile, line)) {
            if (line.find("Cached:") == 0) {
                unsigned long long cached;
                sscanf(line.c_str(), "Cached: %llu", &cached);
                cachedMem = cached * 1024;
                break;
            }
        }
        memFile.close();
    }

    unsigned long long totalPhysMem = memInfo.totalram * memInfo.mem_unit;
    unsigned long long freePhysMem = memInfo.freeram * memInfo.mem_unit;
    unsigned long long bufferMem = memInfo.bufferram * memInfo.mem_unit;
    unsigned long long physMemUsed = totalPhysMem - freePhysMem - bufferMem - cachedMem;

    if (totalPhysMem == 0) return 0.0f;

    return (static_cast<float>(physMemUsed) / totalPhysMem) * 100.0f;
}

void Dashboard::render() {
    updateSessionTime();

    ImGui::BeginChild("Feature Status", ImVec2(0, 120), true);
    ImGui::Text("Feature Status");
    ImGui::Separator();

    if (featureManager) {
        ImGui::Columns(3, "status_columns", false);

        ImGui::Text("Game Process");
        ImGui::Spacing();
        if (featureManager->isGameFound()) {
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Connected (PID: %d)", (int)ProcessId);
        } else {
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Not Found");
        }
        ImGui::NextColumn();

        ImGui::Text("Memory Access");
        ImGui::Spacing();
        if (featureManager->isBaseAddressValid()) {
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Valid (Base: 0x%lX)", BaseAddress);
        } else {
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Invalid");
        }
        ImGui::NextColumn();

        ImGui::Text("Actors Found");
        ImGui::Spacing();
        ImGui::Text("%zu", GetEntityList().size());

        ImGui::Columns(1);
    } else {
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Feature manager not initialized!");
    }

    ImGui::EndChild();

    ImGui::Spacing();

    ImGui::BeginChild("SessionInfo", ImVec2(0, 150), true);
    ImGui::Text("Session Information");
    ImGui::Separator();

    ImGui::Columns(3, "session_columns", false);

    ImGui::Text("Session Time");
    ImGui::Spacing();
    ImGui::Text("%02d:%02d:%02d", sessionHours, sessionMinutes, sessionSeconds);
    ImGui::NextColumn();

    ImGui::Text("Awareness Status");
    ImGui::Spacing();
    if (featureManager && (featureManager->enemyAwarenessSettings.enabled || featureManager->teamAwarenessSettings.enabled)) {
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Enabled");
    } else {
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Disabled");
    }
    ImGui::NextColumn();

    ImGui::Text("Aimbot Status");
    ImGui::Spacing();
    if (featureManager && featureManager->getAccuracySettings().enabled) {
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Enabled");
    } else {
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Disabled");
    }

    ImGui::Columns(1);
    ImGui::Spacing();
    if (ImGui::Button("Unload Krypt0n", ImVec2(-1, 30))) {
        if (g_Running_ptr) {
            g_Running_ptr->store(false);
        }
    }

    ImGui::EndChild();

    ImGui::Spacing();

    ImGui::BeginChild("SystemInfo", ImVec2(0, 0), true);
    ImGui::Text("System Information");
    ImGui::Separator();

    float cpuUsage = getCpuUsage();
    float memoryUsage = getMemoryUsage();

    ImGui::Text("CPU Usage: %.1f%%", cpuUsage);
    ImGui::ProgressBar(cpuUsage / 100.0f, ImVec2(-1.0f, 0.0f));

    ImGui::Text("Memory Usage: %.1f%%", memoryUsage);
    ImGui::ProgressBar(memoryUsage / 100.0f, ImVec2(-1.0f, 0.0f));

    ImGui::EndChild();
}