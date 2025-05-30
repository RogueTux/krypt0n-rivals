#include "../include/AccuracyAssist.h"
#include "../include/Memory.h"
#include "../include/GameData.h"
#include "../include/FeatureManager.h"

#include <cfloat>
#include <cmath>
#include <mutex>
#include <unistd.h>
#include <vector>
#include <atomic>
#include <iomanip>
#include <algorithm>
#include <fstream>

#include "EntityScanner.h"

extern std::atomic<bool> g_Running;
bool isAccuracyInitialized_AA_Full = false;


float GetCharacterHeadOffset(const Actors &actor) {
    if (!actor.PlayerState) return 75.0f;
    static const std::unordered_map<std::string, float> characterOffsets = {
        {"Jeff", 1.0f}, {"Rocket Raccoon", 20.0f}, {"Groot", 75.0f}, {"Hulk", 90.0f},
        {"The Thing", 85.0f}, {"Squirrel Girl", 65.0f}, {"Colossus", 90.0f}, {"Magik", 56.0f},
        {"Venom", 55.0f}, {"Magneto", 90.0f}, {"Wolverine", 55.0f}, {"Invisible Woman", 60.0f},
        {"Storm", 85.0f}, {"Loki", 80.0f}, {"Mantis", 55.0f}, {"Black Panther", 40.0f},
        {"Iron Man", 90.0f}, {"Spider-Man", 60.0f}, {"Thor", 80.0f}, {"Iron Fist", 68.0f},
        {"Scarlet Witch", 60.0f}, {"Hawkeye", 78.0f}, {"Human Torch", 82.0f}, {"Hela", 70.0f}, {"Moon Knight", 70.0f}
    };
    const std::string characterName = GetHeroName(actor.PlayerState);
    auto it = characterOffsets.find(characterName);
    return (it != characterOffsets.end()) ? it->second : 70.0f;
}

FVector EstimateHeadPosition(const Actors &actor) {
    if (!actor.PlayerActor) return FVector(0, 0, 0);
    const PlayerLocation baseLoc = GetPlayerLocation(actor.PlayerActor);
    if (!baseLoc.isValid) return FVector(0, 0, 0);
    FVector headPos = baseLoc.location;
    headPos.z += GetCharacterHeadOffset(actor);
    return headPos;
}

bool InitializeAccuracyAssist_Full() {
    if (isAccuracyInitialized_AA_Full) return true;
    isAccuracyInitialized_AA_Full = true;
    return true;
}

bool IsValidTargetInFOV(const AccuracySettings &settings, const Actors &actor, FVector localPlayerWorldPos, float &outScreenDistanceToCenter, FVector &outWorldAimPoint) {
    if (!actor.PlayerActor) return false;

    FVector targetWorldPos;
    const PlayerLocation actorRootLoc = GetPlayerLocation(actor.PlayerActor);

    if (settings.boneTarget > 0) {
        targetWorldPos = GetBone(actor, settings.boneTarget);
        if (targetWorldPos.Magnitude() <= 0.1) {
            if (settings.useFallbackAiming) {
                if (!actorRootLoc.isValid) return false;
                if (settings.boneTarget == 8) targetWorldPos = EstimateHeadPosition(actor);
                else if (settings.boneTarget == 5) {
                    targetWorldPos = actorRootLoc.location;
                    targetWorldPos.z += GetCharacterHeadOffset(actor) * 0.6f;
                } else targetWorldPos = actorRootLoc.location;

                if (targetWorldPos.Magnitude() <= 0.1) return false;
            } else {
                return false;
            }
        }
    } else {
        if (!actorRootLoc.isValid) return false;
        targetWorldPos = actorRootLoc.location;
    }

    if (targetWorldPos.Magnitude() <= 0.1) return false;

    const FVector targetScreenPos = WorldToScreen(targetWorldPos);
    if (targetScreenPos.x == -1 && targetScreenPos.y == -1) return false;
    if (targetScreenPos.x < 0 || targetScreenPos.y < 0 || targetScreenPos.x >= GameScreen.Width || targetScreenPos.y >= GameScreen.Height) return false;

    if (GameScreen.Width <= 0 || GameScreen.Height <= 0) return false;
    const float centerX = GameScreen.Width / 2.0f;
    const float centerY = GameScreen.Height / 2.0f;
    const float dx = targetScreenPos.x - centerX;
    const float dy = targetScreenPos.y - centerY;
    outScreenDistanceToCenter = sqrtf(dx * dx + dy * dy);

    if (outScreenDistanceToCenter < settings.fov) {
        outWorldAimPoint = targetWorldPos;
        return true;
    }
    return false;
}

bool IsTargetVisible(const AccuracySettings &settings, uintptr_t targetMesh) {
    if (!settings.visibilityCheck) return true;
    return true;
}

PotentialTarget GetClosestTarget_Optimized(const AccuracySettings &settings, const FVector &localPlayerWorldPos) {
    PotentialTarget best_target;
    best_target.screen_distance_from_center = FLT_MAX;
    best_target.is_valid = false;

    const std::vector<Actors> currentActors = GetEntityList();

    for (const Actors &actor: currentActors) {
        FVector currentWorldAimPoint;

        if (float currentScreenDistance; IsValidTargetInFOV(settings, actor, localPlayerWorldPos, currentScreenDistance, currentWorldAimPoint)) {
            auto [current_health, max_health] = GetHealthInfo(actor);
            if (current_health <= 0.1f) {
                continue;
            }

            if (bool shouldTarget = settings.targetAllies || isEnemy(actor.PlayerState)) {
                if (!IsTargetVisible(settings, actor.Mesh)) {
                    continue;
                }
                if (currentScreenDistance < best_target.screen_distance_from_center) {
                    best_target.screen_distance_from_center = currentScreenDistance;
                    best_target.actor_data = actor;
                    best_target.world_aim_point = currentWorldAimPoint;
                    best_target.is_valid = true;
                }
            }
        }
    }
    return best_target;
}

void *AccuracyThreadFunc(void *param) {
    auto *featureManager = static_cast<FeatureManager *>(param);
    if (!featureManager) {
        return nullptr;
    }

    if (!InitializeAccuracyAssist_Full()) {
    }

    while (g_Running.load()) {
        AccuracySettings &currentSettings = featureManager->accuracySettings;
        bool assistActiveThisTick = false;
        uintptr_t localPawn = 0;
        {
            std::lock_guard lock(GameDataMutex);
            localPawn = GameAddrs.AcknowledgedPawn;
        }

        if (currentSettings.enabled && featureManager->isAssistKeyPressed.load() && isAccuracyInitialized_AA_Full && GameScreen.Width > 0 && localPawn != 0) {
            try {
                const PlayerLocation localPlayerLoc = GetPlayerLocation(localPawn);
                if (!localPlayerLoc.isValid) {
                    featureManager->accuracyDeltaX.store(0.0f);
                    featureManager->accuracyDeltaY.store(0.0f);
                    usleep(50000);
                    continue;
                }
                FVector localPos = localPlayerLoc.location;

                if (PotentialTarget currentTargetInfo = GetClosestTarget_Optimized(currentSettings, localPos); currentTargetInfo.is_valid) {
                    Actors& targetActor = currentTargetInfo.actor_data;
                    const FVector rawBoneWorldPos = currentTargetInfo.world_aim_point;
                    FVector finalAimWorldPos = rawBoneWorldPos;


                    if (currentSettings.enablePrediction && currentSettings.projectileSpeed > 1.0f) {
                        if (const float distance = localPos.Distance(rawBoneWorldPos); distance > 1.0f) {
                            float timeToTarget = distance / currentSettings.projectileSpeed;
                            timeToTarget = std::min(timeToTarget, 0.5f);
                            finalAimWorldPos += targetActor.Velocity * timeToTarget;
                            if (currentSettings.projectileGravity > 0.0f) {
                                const float gravityEffect = 0.5f * currentSettings.projectileGravity * timeToTarget * timeToTarget;
                                finalAimWorldPos.z += gravityEffect;
                            }
                        }
                    }

                    if (const FVector targetScreen = WorldToScreen(finalAimWorldPos); targetScreen.x >= 0 && targetScreen.y >= 0) {
                        const float centerX = GameScreen.Width / 2.0f;
                        const float centerY = GameScreen.Height / 2.0f;
                        float diffX = targetScreen.x - centerX; float diffY = targetScreen.y - centerY;
                        if (currentSettings.smoothing > 1.0f) { diffX /= currentSettings.smoothing; diffY /= currentSettings.smoothing; }

                        featureManager->accuracyDeltaX.store(diffX);
                        featureManager->accuracyDeltaY.store(diffY);
                        assistActiveThisTick = true;
                    }
                }

            } catch (const std::exception &) {
                 usleep(100000);
            } catch (...) {
                 usleep(100000);
            }
        }

        if (!assistActiveThisTick) {
            featureManager->accuracyDeltaX.store(0.0f);
            featureManager->accuracyDeltaY.store(0.0f);
        }
        usleep(currentSettings.enabled && featureManager->isAssistKeyPressed.load() ? 3000 : 30000);
    }

    featureManager->accuracyDeltaX.store(0.0f);
    featureManager->accuracyDeltaY.store(0.0f);
    return nullptr;
}