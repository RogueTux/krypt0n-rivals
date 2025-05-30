#ifndef FEATURE_MANAGER_H
#define FEATURE_MANAGER_H

#include <atomic>
#include "GameData.h"
#include "AccuracyAssist.h"

extern void* OverlayThreadFunc(void* param);

struct AwarenessSettings {
    bool enabled = false;
    bool showBox = false;
    bool showHealth = false;
    bool showName = false;
    bool showDistance = false;
    float visibleColor[4] = {0.0f, 1.0f, 0.0f, 1.0f};
    float invisibleColor[4] = {1.0f, 0.0f, 0.0f, 1.0f};
    float maxDistance = 300.0f;
};

class FeatureManager {
public:
    FeatureManager();
    ~FeatureManager();

    bool initialize();
    void update();
    void shutdown();

    std::atomic<float> accuracyDeltaX{0.0f};
    std::atomic<float> accuracyDeltaY{0.0f};
    std::atomic<bool> isAssistKeyPressed{false};

    AccuracySettings accuracySettings;
    AwarenessSettings enemyAwarenessSettings;
    AwarenessSettings teamAwarenessSettings;
    bool showFovCircle = false;
    float fovCircleColor[4] = {0.0f, 1.0f, 1.0f, 1.0f};

    AccuracySettings& getAccuracySettings() { return accuracySettings; }

    [[nodiscard]] bool isGameFound() const { return gameFound.load(); }
    [[nodiscard]] bool isBaseAddressValid() const { return baseAddressValid.load(); }
    [[nodiscard]] bool isEntityScanRunning() const { return entityScanRunning.load(); }
    [[nodiscard]] bool isAccuracyRunning() const { return accuracyRunning.load(); }
    [[nodiscard]] bool isOverlayRunning() const { return overlayRunning.load(); }
    [[nodiscard]] static int getPlayerCount();
    [[nodiscard]] double getFrameTime() const { return frameTime.load(); }

private:
    std::atomic<bool> gameFound;
    std::atomic<bool> baseAddressValid;
    std::atomic<bool> entityScanRunning;
    std::atomic<bool> accuracyRunning;
    std::atomic<bool> overlayRunning;
    std::atomic<double> frameTime;

    pthread_t entityThreadId;
    pthread_t accuracyThreadId;
    pthread_t overlayThreadId;

    bool findGameProcess();
    bool findBaseAddress();
    bool initializeThreads();
};

#endif