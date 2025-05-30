#pragma once
#include <string>
#include <vector>
#include <memory>
#include "tabs/ITabContent.h"
#include "FeatureManager.h"
#include <atomic>

class CustomMenu {
public:
    explicit CustomMenu(FeatureManager* featureManager = nullptr, std::atomic<bool>* g_running_ptr = nullptr);
    void render();

private:
    struct Tab {
        std::string name;
        bool selected;
        std::unique_ptr<ITabContent> content;

        Tab(Tab&& other) noexcept
            : name(std::move(other.name))
            , selected(other.selected)
            , content(std::move(other.content)) {}

        Tab& operator=(Tab&& other) noexcept {
            if (this != &other) {
                name = std::move(other.name);
                selected = other.selected;
                content = std::move(other.content);
            }
            return *this;
        }

        Tab(std::string name, const bool isSelected, std::unique_ptr<ITabContent> c)
            : name(std::move(name))
            , selected(isSelected)
            , content(std::move(c)) {}

        Tab(const Tab&) = delete;
        Tab& operator=(const Tab&) = delete;
    };

    size_t currentTab;
    std::vector<Tab> tabs;
    FeatureManager* featureManager;
    std::atomic<bool>* g_Running_ptr;

    static void renderTitleBar();
    void renderTabs();
    void renderContent() const;
    void renderStatusBar() const;
};