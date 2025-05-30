#ifndef ACCURACY_ASSIST_H
#define ACCURACY_ASSIST_H

#include "GameData.h"
#include <GLFW/glfw3.h>

class FeatureManager;

struct AccuracySettings {
    bool enabled{};
    float fov{};
    float smoothing{};
    int boneTarget{};
    int aimKey = GLFW_MOUSE_BUTTON_LEFT;
    bool visibilityCheck{};
    bool targetAllies = false;
    bool enablePrediction = false;
    float projectileSpeed = 70000.0f;
    float projectileGravity = 980.0f;
    bool useFallbackAiming = true;
};

struct PotentialTarget {
    Actors actor_data;
    FVector world_aim_point;
    float screen_distance_from_center;
    bool is_valid = false;
};

bool IsTargetVisible(const AccuracySettings& settings, uintptr_t targetMesh);
void* AccuracyThreadFunc(void* param);
PotentialTarget GetClosestTarget_Optimized(const AccuracySettings &settings, const FVector &localPlayerWorldPos);
FVector EstimateHeadPosition(const Actors &actor);

#endif