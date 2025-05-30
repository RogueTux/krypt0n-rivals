#include "../include/menu.h"
#include "../include/tabs/dashboard.h"
#include "../include/tabs/visuals.h"
#include "../include/tabs/aim.h"
#include "../include/tabs/misc.h"
#include "../include/tabs/settings.h"
#include "../include/FeatureManager.h"
#include <imgui.h>
#include <string>
#include <deque>
#include <numeric>
#include <vector>
#include <memory>
#include <cstdio>

#include "EntityScanner.h"

CustomMenu::CustomMenu(FeatureManager* fm, std::atomic<bool>* g_running_ptr)
    : currentTab(0), featureManager(fm), g_Running_ptr(g_running_ptr) {

    tabs.emplace_back( "Dashboard", true, std::make_unique<Dashboard>(featureManager, g_Running_ptr) );
    tabs.emplace_back( "Visuals", false, std::make_unique<Visuals>(featureManager) );
    tabs.emplace_back( "Aim", false, std::make_unique<Aim>(featureManager) );
    tabs.emplace_back( "Settings", false, std::make_unique<Settings>(featureManager) );
}

void CustomMenu::render() {
    constexpr ImGuiWindowFlags menu_content_flags =
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar;

    ImGui::SetNextWindowPos(ImVec2(0,0));
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8,8));

    if (ImGui::Begin("Krypt0n Menu Window", nullptr, menu_content_flags)) {
        renderTitleBar();
        const float statusBarHeight = ImGui::GetFrameHeightWithSpacing() + ImGui::GetStyle().ItemSpacing.y;
        const float availableHeight = ImGui::GetContentRegionAvail().y;
        const float mainContentHeight = availableHeight - statusBarHeight;

        ImGui::BeginChild("MainContent", ImVec2(0, mainContentHeight), false, ImGuiWindowFlags_NoScrollbar);
        ImGui::BeginChild("Tabs", ImVec2(120, 0), true);
        renderTabs();
        ImGui::EndChild();
        ImGui::SameLine();
        ImGui::BeginChild("Content", ImVec2(0, 0), true);
        renderContent();
        ImGui::EndChild();
        ImGui::EndChild();
        renderStatusBar();
        ImGui::End();
    }
    ImGui::PopStyleVar(3);
}

void CustomMenu::renderTitleBar() {
    static std::deque<float> fpsHistory;
    static float smoothedFps = 0.0f;

    if (const float currentFps = ImGui::GetIO().Framerate; currentFps > 0.0f) {
        fpsHistory.push_back(currentFps);
        if (constexpr size_t fpsHistorySize = 90; fpsHistory.size() > fpsHistorySize) {
            fpsHistory.pop_front();
        }
        if (!fpsHistory.empty()) {
            const float sum = std::accumulate(fpsHistory.begin(), fpsHistory.end(), 0.0f);
            smoothedFps = sum / fpsHistory.size();
        }
    }

    ImGui::BeginChild("TitleBar", ImVec2(0, 25.0f + ImGui::GetStyle().WindowPadding.y), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    ImGui::Text("Version:");
    ImGui::SameLine();
    ImGui::TextDisabled("v1.0.1");

    const float windowWidth = ImGui::GetWindowWidth();
    const auto titleText = "$ Krypt0n Marvel Rivals";
    const float titleWidth = ImGui::CalcTextSize(titleText).x;
    ImGui::SameLine((windowWidth - titleWidth) * 0.5f);
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.7f, 0.0f, 0.0f, 1.0f));
    ImGui::Text("%s", titleText);
    ImGui::PopStyleColor();

    char fpsBuffer[32];
    snprintf(fpsBuffer, sizeof(fpsBuffer), "FPS: %.1f", smoothedFps);
    const float fpsTextWidth = ImGui::CalcTextSize(fpsBuffer).x;
    ImGui::SameLine(windowWidth - fpsTextWidth - ImGui::GetStyle().WindowPadding.x);
    ImGui::TextDisabled("%s", fpsBuffer);
    ImGui::EndChild();
}

void CustomMenu::renderTabs() {
    for (size_t i = 0; i < tabs.size(); i++) {
        ImGui::PushStyleColor(ImGuiCol_Button,
            currentTab == i ? ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive)
                          : ImGui::GetStyleColorVec4(ImGuiCol_Button));
        std::string buttonLabel = tabs[i].name;
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 10));
        if (ImGui::Button(buttonLabel.c_str(), ImVec2(-1, 40))) {
            currentTab = i;
        }
        ImGui::PopStyleVar();
        ImGui::PopStyleColor();
        if (i < tabs.size() - 1) {
            ImGui::Spacing();
        }
    }
}

void CustomMenu::renderStatusBar() const {
    const float statusBarHeight = ImGui::GetFrameHeightWithSpacing();
    ImGui::BeginChild("StatusBar", ImVec2(0, statusBarHeight), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

    if (featureManager) {
        ImGui::Text("Game:");
        ImGui::SameLine();
        if (featureManager->isGameFound()) {
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Connected");
        } else {
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Not Found");
        }
        ImGui::SameLine(150);
        ImGui::Text("Memory:");
        ImGui::SameLine();
        if (featureManager->isBaseAddressValid()) {
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "OK");
        } else {
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Error");
        }
        ImGui::SameLine(280);
        ImGui::Text("Actors: %zu", GetEntityList().size());
    } else {
        ImGui::Text("Feature Manager not initialized!");
    }
    ImGui::EndChild();
}

void CustomMenu::renderContent() const {
    if (currentTab >= 0 && currentTab < tabs.size() && tabs[currentTab].content) {
        tabs[currentTab].content->render();
    } else {
        ImGui::Text("Error: Invalid tab selected or content missing.");
    }
}