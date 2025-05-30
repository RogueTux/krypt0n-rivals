#pragma once

class FeatureManager;

class ITabContent {
public:
    explicit ITabContent(FeatureManager* featureManager = nullptr) : featureManager(featureManager) {}
    virtual ~ITabContent() = default;
    virtual void render() = 0;

protected:
    FeatureManager* featureManager;
};