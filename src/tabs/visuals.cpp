#include "../../include/tabs/visuals.h"
#include "../../include/FeatureManager.h"
#include <imgui.h>

void Visuals::render() {
    if (!featureManager) {
        ImGui::Text("Feature manager not initialized!");
        return;
    }

    ImGui::BeginChild("VisualsSettingsContainer", ImVec2(0, 0), false);

    ImGui::Text("Enemy Awareness Settings"); ImGui::Separator();
    {
        AwarenessSettings& settings = featureManager->enemyAwarenessSettings;
        ImGui::PushID("EnemyAwareness");

        ImGui::Checkbox("Enable Enemy Awareness", &settings.enabled);
        ImGui::Checkbox("Show Box", &settings.showBox);
        ImGui::SameLine(250);
        ImGui::Checkbox("Show Health", &settings.showHealth);
        ImGui::Checkbox("Show Name", &settings.showName);
        ImGui::SameLine(250);
        ImGui::Checkbox("Show Distance", &settings.showDistance);

        ImGui::PushItemWidth(150);
        ImGui::ColorEdit4("Visible##Enemy", settings.visibleColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);
        ImGui::SameLine();
        ImGui::ColorEdit4("Occluded##Enemy", settings.invisibleColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);
        ImGui::PopItemWidth();

        float maxDistMeters = settings.maxDistance / 100.0f;
        if (ImGui::SliderFloat("Max Distance (m)##Enemy", &maxDistMeters, 0.0f, 500.0f, "%.0f m")) {
            settings.maxDistance = maxDistMeters * 100.0f;
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Maximum visual awareness distance for enemies in meters");
        }
        ImGui::PopID();
    }

    ImGui::Spacing(); ImGui::Spacing();

    ImGui::Text("Team Awareness Settings"); ImGui::Separator();
    {
        auto&[enabled, showBox, showHealth, showName, showDistance, visibleColor, invisibleColor, maxDistance] = featureManager->teamAwarenessSettings;
        ImGui::PushID("TeamAwareness");

        ImGui::Checkbox("Enable Team Awareness", &enabled);
        ImGui::Checkbox("Show Box", &showBox);
        ImGui::SameLine(250);
        ImGui::Checkbox("Show Health", &showHealth);
        ImGui::Checkbox("Show Name", &showName);
        ImGui::SameLine(250);
        ImGui::Checkbox("Show Distance", &showDistance);

        ImGui::PushItemWidth(150);
        ImGui::ColorEdit4("Visible##Team", visibleColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);
        ImGui::SameLine();
        ImGui::ColorEdit4("Occluded##Team", invisibleColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);
        ImGui::PopItemWidth();

        float maxDistMeters = maxDistance / 100.0f;
        if (ImGui::SliderFloat("Max Distance (m)##Team", &maxDistMeters, 0.0f, 500.0f, "%.0f m")) {
            maxDistance = maxDistMeters * 100.0f;
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Maximum visual awareness distance for teammates in meters");
        }
        ImGui::PopID();
    }

    ImGui::Spacing(); ImGui::Spacing();

    ImGui::Text("General Visual Settings"); ImGui::Separator();
    {
        ImGui::Checkbox("Show Aim FOV Circle", &featureManager->showFovCircle);
        ImGui::SameLine();
        ImGui::PushItemWidth(150);
        ImGui::ColorEdit4("FOV Circle Color", featureManager->fovCircleColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);
        ImGui::PopItemWidth();
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Shows a circle representing the accuracy assist Field of View. Radius configured in Aim tab.");
        }
    }

    ImGui::EndChild();
}