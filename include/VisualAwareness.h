#ifndef VISUAL_AWARENESS_H
#define VISUAL_AWARENESS_H

#include <imgui.h>
#include "../include/GameData.h"
#include "../include/FeatureManager.h"

class VisualAwareness {
public:
    VisualAwareness(FeatureManager* featureManager);
    void render();

private:
    FeatureManager* featureManager;

    static void drawBox(ImDrawList* drawList, const FVector& topLeft, float width, float height,
                        ImU32 color, float thickness);

    static void drawCorneredBox(ImDrawList* drawList, const FVector& topLeft, float width, float height,
                                ImU32 color, float thickness);

    static void drawHealthBar(ImDrawList* drawList, const FVector& position, float width, float height,
                              float healthPercent, ImU32 backgroundColor, ImU32 foregroundColor);

    static void drawText(ImDrawList* drawList, const FVector& position, ImU32 color,
                         const char* text, bool centered = false);

    static void drawLine(ImDrawList* drawList, const FVector& start, const FVector& end,
                         ImU32 color, float thickness);

    static ImU32 colorToImU32(const float color[4]);

    static bool shouldSkipActor(const Actors& actor);

    static bool isEnemyValid(const PlayerLocation& enemy);
    void drawEntityAwareness(ImDrawList* drawList, const Actors& actor, const PlayerLocation& enemy, ImU32 color, const AwarenessSettings& settings);
};

#endif