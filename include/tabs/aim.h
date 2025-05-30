#pragma once
#include "ITabContent.h"
#include <string>

class FeatureManager;

std::string getKeyName(int keyCode);

class Aim final : public ITabContent {
public:
    explicit Aim(FeatureManager* featureManager = nullptr)
        : ITabContent(featureManager) {}

    void render() override;
};

bool DrawKeyBinder(const char* label, int* key_code_ptr, const char* tooltip = nullptr);