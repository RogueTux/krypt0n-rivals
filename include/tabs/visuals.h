#pragma once
#include "ITabContent.h"

class FeatureManager;

class Visuals final : public ITabContent {
public:
    explicit Visuals(FeatureManager* featureManager = nullptr)
        : ITabContent(featureManager) {}

    void render() override;
};