#include "../include/FeatureManager.h"
#include "../include/EntityScanner.h"
#include "../include/ProcessUtils.h"
#include "../include/Memory.h"
#include "../include/GameData.h"
#include "../include/AccuracyAssist.h"
#include "../include/Overlay.h"

#include <iostream>
#include <chrono>
#include <csignal>
#include <thread>
#include <atomic>
#include <ctime>
#include <filesystem>

FeatureManager::FeatureManager()
    : gameFound(false), baseAddressValid(false),
      entityScanRunning(false), accuracyRunning(false), overlayRunning(false),
      frameTime(0.0),
      entityThreadId(0), accuracyThreadId(0), overlayThreadId(0) {

    enemyAwarenessSettings.enabled = false;
    enemyAwarenessSettings.showBox = false;
    enemyAwarenessSettings.showHealth = false;
    enemyAwarenessSettings.showName = false;
    enemyAwarenessSettings.showDistance = false;
    enemyAwarenessSettings.visibleColor[0] = 0.0f;
    enemyAwarenessSettings.visibleColor[1] = 1.0f;
    enemyAwarenessSettings.visibleColor[2] = 0.0f;
    enemyAwarenessSettings.visibleColor[3] = 1.0f;
    enemyAwarenessSettings.invisibleColor[0] = 1.0f;
    enemyAwarenessSettings.invisibleColor[1] = 0.0f;
    enemyAwarenessSettings.invisibleColor[2] = 0.0f;
    enemyAwarenessSettings.invisibleColor[3] = 1.0f;
    enemyAwarenessSettings.maxDistance = 30000.0f;

    teamAwarenessSettings.enabled = false;
    teamAwarenessSettings.showBox = false;
    teamAwarenessSettings.showHealth = false;
    teamAwarenessSettings.showName = false;
    teamAwarenessSettings.showDistance = false;
    teamAwarenessSettings.visibleColor[0] = 0.0f;
    teamAwarenessSettings.visibleColor[1] = 0.0f;
    teamAwarenessSettings.visibleColor[2] = 1.0f;
    teamAwarenessSettings.visibleColor[3] = 1.0f;
    teamAwarenessSettings.invisibleColor[0] = 1.0f;
    teamAwarenessSettings.invisibleColor[1] = 1.0f;
    teamAwarenessSettings.invisibleColor[2] = 0.0f;
    teamAwarenessSettings.invisibleColor[3] = 1.0f;
    teamAwarenessSettings.maxDistance = 30000.0f;

    showFovCircle = false;
    fovCircleColor[0] = 1.0f; fovCircleColor[1] = 1.0f; fovCircleColor[2] = 1.0f; fovCircleColor[3] = 1.0f;

    accuracySettings.enabled = false;
    accuracySettings.fov = 60.0f;
    accuracySettings.smoothing = 10.0f;
    accuracySettings.boneTarget = 8;
    accuracySettings.aimKey = 1;
    accuracySettings.visibilityCheck = false;
    accuracySettings.targetAllies = false;
    accuracySettings.enablePrediction = false;
    accuracySettings.projectileSpeed = 70000.0f;
    accuracySettings.projectileGravity = 980.0f;
    accuracySettings.useFallbackAiming = true;

    if (const std::string configDir = "../../configs"; !std::filesystem::exists(configDir)) {
        try {
            std::filesystem::create_directory(configDir);
        } catch (const std::filesystem::filesystem_error&) {
        }
    }
}

FeatureManager::~FeatureManager() {
    shutdown();
}

bool FeatureManager::initialize() {
    if (!findGameProcess()) {
        return false;
    }

    if (!findBaseAddress()) {
        return false;
    }

    {
        std::lock_guard lock(GameDataMutex);
        GameAddrs.UWorld = ReadMemory<uintptr_t>(BaseAddress + Offsets::UWorld);
        if (!GameAddrs.UWorld) {
        } else {
        }
    }

    srand(static_cast<unsigned int>(time(nullptr)));

    if (!initializeThreads()) {
        return false;
    }

    return true;
}

void FeatureManager::update() {
    static auto lastFrameTimePoint = std::chrono::high_resolution_clock::now();
    const auto currentTimePoint = std::chrono::high_resolution_clock::now();
    frameTime.store(std::chrono::duration<double, std::milli>(currentTimePoint - lastFrameTimePoint).count());
    lastFrameTimePoint = currentTimePoint;

    static auto lastSystemCheckTime = std::chrono::steady_clock::now();
    const auto currentSteadyTime = std::chrono::steady_clock::now();
    const auto timeSinceLastCheck = std::chrono::duration_cast<std::chrono::seconds>(
        currentSteadyTime - lastSystemCheckTime).count();

    if (timeSinceLastCheck >= 5) {
        if (ProcessId == 0 || kill(ProcessId, 0) != 0) {
            gameFound = false;
            baseAddressValid = false;
            { std::lock_guard lock(GameDataMutex); GameAddrs = {}; }
            BaseAddress = 0;
            ProcessId = 0;

            if (findGameProcess()) {
                findBaseAddress();
            }
        } else if (baseAddressValid.load()) {
             uintptr_t testUWorld = 0;
             { std::lock_guard lock(GameDataMutex); testUWorld = ReadMemory<uintptr_t>(BaseAddress + Offsets::UWorld); }
             if (testUWorld == 0 && GameAddrs.UWorld != 0) {
                 baseAddressValid = false;
                 { std::lock_guard lock(GameDataMutex); GameAddrs.UWorld = 0; }
                 BaseAddress = 0;
                 findBaseAddress();
             }
         }
        lastSystemCheckTime = currentSteadyTime;
    }
}

void FeatureManager::shutdown() {
    if (entityScanRunning.load()) {
        if (entityThreadId != 0) {
            pthread_join(entityThreadId, nullptr);
        }
        entityScanRunning = false;
    }

    if (accuracyRunning.load()) {
         if (accuracyThreadId != 0) {
            pthread_join(accuracyThreadId, nullptr);
        }
        accuracyRunning = false;
    }

    if (overlayRunning.load()) {
         if (overlayThreadId != 0) {
            pthread_join(overlayThreadId, nullptr);
        }
        overlayRunning = false;
    }
}

bool FeatureManager::findGameProcess() {
    ProcessId = FindProcess("Marvel-Win64-Shipping.exe");
    const bool found = (ProcessId != 0);
    gameFound.store(found);
    return found;
}

bool FeatureManager::findBaseAddress() {
    if (!gameFound.load()) return false;
    BaseAddress = FindBaseImage();
    const bool found = (BaseAddress != 0);
    baseAddressValid.store(found);
    return found;
}

bool FeatureManager::initializeThreads() {
    entityScanRunning = true;
    if (pthread_create(&entityThreadId, nullptr, EntityThreadFunc, nullptr) != 0) {
        entityScanRunning = false;
        return false;
    }

    accuracyRunning = true;
    if (pthread_create(&accuracyThreadId, nullptr, AccuracyThreadFunc, this) != 0) {
        accuracyRunning = false;
        return false;
    }

    overlayRunning = true;
    if (pthread_create(&overlayThreadId, nullptr, OverlayThreadFunc, this) != 0) {
        overlayRunning = false;
        return false;
    }

    return true;
}

int FeatureManager::getPlayerCount() {
     return GetEntityList().size();
}