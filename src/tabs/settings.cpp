#include "../../include/tabs/settings.h"
#include "../../include/FeatureManager.h"
#include <imgui.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <cstring>

Settings::Settings(FeatureManager* featureManager)
    : ITabContent(featureManager) {
    configDirectory = "../../configs";

    if (!std::filesystem::exists(configDirectory)) {
        try {
            std::filesystem::create_directory(configDirectory);
        } catch (const std::filesystem::filesystem_error&) {
        }
    }
    strcpy(configNameBuffer, "default");
    configNameBuffer[sizeof(configNameBuffer)-1] = '\0';
    refreshConfigFilesList();
}

void Settings::saveCurrentConfig(const std::string& name) {
    if (!featureManager) {
        return;
    }
    if (name.empty()) {
        return;
    }

    std::filesystem::path filePath = std::filesystem::path(configDirectory) / (name + configExtension);
    std::ofstream configFileStream(filePath, std::ios::binary | std::ios::trunc);

    if (!configFileStream.is_open()) {
        return;
    }

    try {
        AccuracySettings tempAccuracySettings = featureManager->accuracySettings;

        configFileStream.write(reinterpret_cast<const char*>(&tempAccuracySettings), sizeof(AccuracySettings));
        configFileStream.write(reinterpret_cast<const char*>(&featureManager->enemyAwarenessSettings), sizeof(AwarenessSettings));
        configFileStream.write(reinterpret_cast<const char*>(&featureManager->teamAwarenessSettings), sizeof(AwarenessSettings));
        configFileStream.write(reinterpret_cast<const char*>(&featureManager->showFovCircle), sizeof(bool));
        configFileStream.write(reinterpret_cast<const char*>(&featureManager->fovCircleColor), sizeof(featureManager->fovCircleColor));


        configFileStream.close();
        refreshConfigFilesList();
    } catch (const std::exception&) {
        if (configFileStream.is_open()) configFileStream.close();
    }
}

void Settings::loadSelectedConfig(const std::string& name) const {
    if (!featureManager) {
        return;
    }
    if (name.empty()) {
        return;
    }

    std::filesystem::path filePath = std::filesystem::path(configDirectory) / name;
    std::ifstream configFileStream(filePath, std::ios::binary);

    if (!configFileStream.is_open()) {
        return;
    }

    try {
        AccuracySettings tempAccuracySettings;
        configFileStream.read(reinterpret_cast<char*>(&tempAccuracySettings), sizeof(AccuracySettings));

        featureManager->accuracySettings = tempAccuracySettings;

        configFileStream.read(reinterpret_cast<char*>(&featureManager->enemyAwarenessSettings), sizeof(AwarenessSettings));
        configFileStream.read(reinterpret_cast<char*>(&featureManager->teamAwarenessSettings), sizeof(AwarenessSettings));
        configFileStream.read(reinterpret_cast<char*>(&featureManager->showFovCircle), sizeof(bool));
        configFileStream.read(reinterpret_cast<char*>(&featureManager->fovCircleColor), sizeof(featureManager->fovCircleColor));


        configFileStream.close();
    } catch (const std::exception&) {
         if (configFileStream.is_open()) configFileStream.close();
    }
}

void Settings::deleteSelectedConfig(const std::string& name) {
    if (name.empty()) {
        return;
    }
    const std::filesystem::path filePath = std::filesystem::path(configDirectory) / name;
    try {
        if (std::filesystem::remove(filePath)) {
            if (selectedConfigFile == name) {
                selectedConfigFile.clear();
                 strcpy(configNameBuffer, "default");
                 configNameBuffer[sizeof(configNameBuffer)-1] = '\0';
            }
            refreshConfigFilesList();
        }
    } catch (const std::filesystem::filesystem_error&) {
    }
}

void Settings::refreshConfigFilesList() {
    configFiles.clear();
    try {
        for (const auto& entry : std::filesystem::directory_iterator(configDirectory)) {
            if (entry.is_regular_file() && entry.path().extension() == configExtension) {
                configFiles.push_back(entry.path().filename().string());
            }
        }
    } catch (const std::filesystem::filesystem_error&) {
    }
    if (!selectedConfigFile.empty() && std::find(configFiles.begin(), configFiles.end(), selectedConfigFile) == configFiles.end()) {
        selectedConfigFile.clear();
    }
}


void Settings::render() {
    ImGui::BeginChild("ConfigManagementRegion", ImVec2(0, 0), true);
    ImGui::Text("Configuration Management");
    ImGui::Separator();

    ImGui::InputText("Config Name##Input", configNameBuffer, IM_ARRAYSIZE(configNameBuffer));

    ImGui::SameLine();
    if (ImGui::Button("Save##Config")) {
        if (strlen(configNameBuffer) > 0) {
            saveCurrentConfig(configNameBuffer);
        }
    }

    ImGui::Text("Available Configs");
    ImGui::PushItemWidth(-1);
    if (ImGui::BeginListBox("##configsListBox", ImVec2(0, ImGui::GetTextLineHeightWithSpacing() * 8))) {
        for (const auto& cfgFile : configFiles) {
            const bool isSelected = (selectedConfigFile == cfgFile);
            if (ImGui::Selectable(cfgFile.c_str(), isSelected)) {
                selectedConfigFile = cfgFile;
                std::string baseName = selectedConfigFile.substr(0, selectedConfigFile.length() - configExtension.length());
                strncpy(configNameBuffer, baseName.c_str(), sizeof(configNameBuffer) - 1);
                configNameBuffer[sizeof(configNameBuffer) -1] = '\0';
            }
            if (isSelected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndListBox();
    }
    ImGui::PopItemWidth();


    if (ImGui::Button("Load##Config") && !selectedConfigFile.empty()) {
        loadSelectedConfig(selectedConfigFile);
    }
    ImGui::SameLine();
    if (ImGui::Button("Delete##Config") && !selectedConfigFile.empty()) {
        deleteSelectedConfig(selectedConfigFile);
    }
    ImGui::SameLine();
    if (ImGui::Button("Refresh##ConfigList")) {
        refreshConfigFilesList();
    }
    ImGui::EndChild();
}