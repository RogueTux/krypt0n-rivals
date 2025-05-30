#pragma once
#include "ITabContent.h"
#include <string>
#include <vector>
#include <filesystem>

class FeatureManager;

class Settings final : public ITabContent {
public:
    explicit Settings(FeatureManager* featureManager = nullptr);
    void render() override;

private:
    std::vector<std::string> configFiles;
    std::string selectedConfigFile;
    char configNameBuffer[128]{};
    std::string configDirectory;
    const std::string configExtension = ".kcfg";

    void saveCurrentConfig(const std::string& name);
    void loadSelectedConfig(const std::string& name) const;
    void deleteSelectedConfig(const std::string& name);
    void refreshConfigFilesList();
};