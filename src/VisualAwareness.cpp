#include "../include/VisualAwareness.h"
#include "../include/EntityScanner.h"
#include "../include/GameData.h"

#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <imgui.h>
#include <iomanip>

VisualAwareness::VisualAwareness(FeatureManager* featureManager) : featureManager(featureManager) {
    if (!featureManager) {
    }
}

void VisualAwareness::render() {
    if (!featureManager) {
        return;
    }

    const AwarenessSettings& currentSettings = featureManager->enemyAwarenessSettings;

    if (!currentSettings.enabled) {
        return;
    }


    uintptr_t localPawnAddr;
    uintptr_t cameraAddr;
    {
        std::lock_guard lock(GameDataMutex);
        localPawnAddr = GameAddrs.AcknowledgedPawn;
        cameraAddr = GameAddrs.PlayerCamera;
    }

    if (!localPawnAddr || !cameraAddr) {
        return;
    }

    if (!ImGui::GetCurrentContext()) {
        return;
    }

    try {
        ImDrawList* drawList = ImGui::GetForegroundDrawList();

        if (!drawList) {
             return;
        }

        const PlayerLocation localPlayer = GetPlayerLocation(localPawnAddr);
        if (!localPlayer.isValid) {
             return;
        }

        const std::vector<Actors> currentActors = GetEntityList();

        for (const Actors& actor : currentActors) {
            if (shouldSkipActor(actor)) {
                continue;
            }

            if (!isEnemy(actor.PlayerState)) {
                continue;
            }

            PlayerLocation enemy = GetPlayerLocation(actor.PlayerActor);
            if (!isEnemyValid(enemy)) {
                continue;
            }

            if (float distance = localPlayer.location.Distance(enemy.location); distance > currentSettings.maxDistance) {
                continue;
            }

            bool isVisible = true;

            const ImU32 color = colorToImU32(currentSettings.visibleColor);

            drawEntityAwareness(drawList, actor, enemy, color, currentSettings);
        }

    } catch (const std::exception&) {
    } catch (...) {
    }
}

ImU32 VisualAwareness::colorToImU32(const float color[4]) {
    return ImGui::ColorConvertFloat4ToU32(ImVec4(color[0], color[1], color[2], color[3]));
}

bool VisualAwareness::shouldSkipActor(const Actors& actor) {
    if (!actor.PlayerActor) {
        return true;
    }
    if (!actor.PlayerState) {
        return true;
    }
    return false;
}

bool VisualAwareness::isEnemyValid(const PlayerLocation& enemy) {
    if (!enemy.isValid) {
        return false;
    }
    if (enemy.screenPos.x < 0 || enemy.screenPos.y < 0) {
        return false;
    }
    if (std::isnan(enemy.screenPos.x) || std::isnan(enemy.screenPos.y)) {
        return false;
    }
    return true;
}

void VisualAwareness::drawEntityAwareness(ImDrawList* drawList, const Actors& actor, const PlayerLocation& enemy, ImU32 color, const AwarenessSettings& settings) {
    FVector headPosition = enemy.location;
    headPosition.z += 75.0f;

    const FVector headScreen = WorldToScreen(headPosition);
    const FVector feetScreen = enemy.screenPos;

    if (headScreen.x < 0 || headScreen.y < 0 || feetScreen.x < 0 || feetScreen.y < 0) {
        return;
    }

    if (std::isnan(headScreen.x) || std::isnan(headScreen.y) || std::isnan(feetScreen.x) || std::isnan(feetScreen.y)) {
        return;
    }

    float boxHeight = abs(headScreen.y - feetScreen.y);
    const float boxWidth = boxHeight * 0.5f;
    float boxTop = headScreen.y;
    float boxBottom = feetScreen.y;
    if (boxTop > boxBottom) std::swap(boxTop, boxBottom);
    boxHeight = boxBottom - boxTop;

    if (boxHeight < 1.0f) return;

    const float boxLeft = headScreen.x - boxWidth / 2.0f;

    const ImVec2 screenSize = ImGui::GetIO().DisplaySize;
    if (boxHeight > screenSize.y * 1.5f ) return;
    if (boxWidth < 1.0f || boxWidth > screenSize.x * 1.5f) return;

    if (settings.showBox) {
        drawCorneredBox(drawList, FVector(boxLeft, boxTop, 0), boxWidth, boxHeight, color, 1.5f);
    }

    if (settings.showHealth) {
        auto [current_health, max_health] = GetHealthInfo(actor);
        float healthPercent = 0.0f;

        if (max_health > 0.0f) {
            healthPercent = current_health / max_health;
        } else {
            healthPercent = (current_health > 0.0f) ? 1.0f : 0.0f;
        }

        drawHealthBar(drawList, FVector(boxLeft - 7.0f, boxTop, 0), 4.0f, boxHeight, healthPercent, IM_COL32(100, 0, 0, 200), IM_COL32(0, 255, 0, 255));
    }

    if (settings.showName) {
        std::string playerName = "Player";
        if (featureManager) {
             playerName = GetHeroName(actor.PlayerState);
        }
        drawText(drawList, FVector(headScreen.x, boxTop - 14.0f, 0), color, playerName.c_str(), true);
    }

    if (settings.showDistance) {
         uintptr_t localPawnAddr = 0;
         { std::lock_guard<std::mutex> lock(GameDataMutex); localPawnAddr = GameAddrs.AcknowledgedPawn; }
         if(localPawnAddr) {
             if (const PlayerLocation localPlayer = GetPlayerLocation(localPawnAddr); localPlayer.isValid) {
                 if (const float distance = localPlayer.location.Distance(enemy.location); !std::isnan(distance)) {
                     const std::string distanceStr = std::to_string(static_cast<int>(distance / 100.0f)) + "m"; // Distance in cm to m
                    drawText(drawList, FVector(headScreen.x, boxBottom + 5.0f, 0), color, distanceStr.c_str(), true);
                 }
             }
         }
    }
}

void VisualAwareness::drawBox(ImDrawList* drawList, const FVector& topLeft, const float width, const float height,
                       const ImU32 color, const float thickness) {
    if (!drawList) return;
    drawList->AddRect(
        ImVec2(topLeft.x, topLeft.y),
        ImVec2(topLeft.x + width, topLeft.y + height),
        color,
        0.0f,
        0,
        thickness
    );
}

void VisualAwareness::drawCorneredBox(ImDrawList* drawList, const FVector& topLeft, const float width, const float height,
                              const ImU32 color, float thickness) {
    if (!drawList) return;

    constexpr float cornerRatio = 0.25f;
    float lineLength = (width < height ? width : height) * cornerRatio;

    if (lineLength < 1.0f) lineLength = 1.0f;
    if (lineLength > width / 2.0f) lineLength = width / 2.0f;
    if (lineLength > height / 2.0f) lineLength = height / 2.0f;


    const ImVec2 tl(topLeft.x, topLeft.y);
    const ImVec2 tr(topLeft.x + width, topLeft.y);
    const ImVec2 bl(topLeft.x, topLeft.y + height);
    const  ImVec2 br(topLeft.x + width, topLeft.y + height);

    drawList->AddLine(tl, ImVec2(tl.x + lineLength, tl.y), color, thickness);
    drawList->AddLine(tl, ImVec2(tl.x, tl.y + lineLength), color, thickness);

    drawList->AddLine(tr, ImVec2(tr.x - lineLength, tr.y), color, thickness);
    drawList->AddLine(tr, ImVec2(tr.x, tr.y + lineLength), color, thickness);

    drawList->AddLine(bl, ImVec2(bl.x + lineLength, bl.y), color, thickness);
    drawList->AddLine(bl, ImVec2(bl.x, bl.y - lineLength), color, thickness);

    drawList->AddLine(br, ImVec2(br.x - lineLength, br.y), color, thickness);
    drawList->AddLine(br, ImVec2(br.x, br.y - lineLength), color, thickness);
}

void VisualAwareness::drawHealthBar(ImDrawList* drawList, const FVector& position, float width, const float height,
                            float healthPercent, ImU32 backgroundColor, ImU32 foregroundColor) {
    if (!drawList || height <= 0 || width <= 0) return;

    if (healthPercent < 0.0f) healthPercent = 0.0f;
    if (healthPercent > 1.0f) healthPercent = 1.0f;

    const ImVec2 bg_min(position.x, position.y);
    const ImVec2 bg_max(position.x + width, position.y + height);

    drawList->AddRectFilled(bg_min, bg_max, backgroundColor);

    const float fgHeight = height * healthPercent;
    const ImVec2 fg_min(position.x, position.y + (height - fgHeight));
    const ImVec2 fg_max(position.x + width, position.y + height);

    if (fgHeight > 0.5f) {
        drawList->AddRectFilled(fg_min, fg_max, foregroundColor);
    }
}

void VisualAwareness::drawText(ImDrawList* drawList, const FVector& position, const ImU32 color,
                       const char* text, bool centered) {
    if (!drawList || !text || text[0] == '\0') return;
    if (std::isnan(position.x) || std::isnan(position.y)) return;

    ImVec2 pos(position.x, position.y);

    const ImU32 shadowColor = IM_COL32(0, 0, 0, 180);

    if (centered) {
        const ImVec2 textSize = ImGui::CalcTextSize(text);
        pos.x -= textSize.x * 0.5f;

        drawList->AddText(ImVec2(pos.x + 1, pos.y + 1), shadowColor, text);
        drawList->AddText(pos, color, text);
    } else {
        drawList->AddText(ImVec2(pos.x + 1, pos.y + 1), shadowColor, text);
        drawList->AddText(pos, color, text);
    }
}

void VisualAwareness::drawLine(ImDrawList* drawList, const FVector& start, const FVector& end,
                       const ImU32 color, const float thickness) {
    if (!drawList) return;
    if (std::isnan(start.x) || std::isnan(start.y) || std::isnan(end.x) || std::isnan(end.y)) return;

    drawList->AddLine(
        ImVec2(start.x, start.y),
        ImVec2(end.x, end.y),
        color,
        thickness
    );
}