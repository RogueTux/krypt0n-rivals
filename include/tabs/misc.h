#pragma once
#include "ITabContent.h"

class FeatureManager;

class Misc final : public ITabContent {
public:
    explicit Misc(FeatureManager* featureManager = nullptr)
        : ITabContent(featureManager),
          spectatorList(false) {}

    void render() override;

private:
    bool spectatorList;
};