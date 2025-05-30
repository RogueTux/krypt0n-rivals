#pragma once
#include "ITabContent.h"
#include <ctime>
#include <atomic>

class FeatureManager;

class Dashboard final : public ITabContent {
public:
    explicit Dashboard(FeatureManager* featureManager = nullptr, std::atomic<bool>* g_running_ptr = nullptr)
        : ITabContent(featureManager),
          sessionStart(std::time(nullptr)),
          sessionHours(0), sessionMinutes(0), sessionSeconds(0),
          g_Running_ptr(g_running_ptr) {}

    ~Dashboard() override = default;
    void render() override;

private:
    time_t sessionStart;
    int sessionHours;
    int sessionMinutes;
    int sessionSeconds;
    std::atomic<bool>* g_Running_ptr;


    void updateSessionTime() {
        const time_t now = std::time(nullptr);
        const time_t diff = now - sessionStart;

        sessionHours = diff / 3600;
        sessionMinutes = (diff % 3600) / 60;
        sessionSeconds = diff % 60;
    }
};