#pragma once
#include <imgui.h>
#include <map>

struct CustomTheme {
    ImVec2 windowPadding;
    ImVec2 framePadding;
    ImVec2 itemSpacing;
    ImVec2 itemInnerSpacing;

    float windowRounding;
    float frameRounding;
    float popupRounding;
    float scrollbarRounding;
    float grabRounding;

    std::map<ImGuiCol_, ImVec4> colors;
};

class Theme {
public:
    static void applyTheme();
    static void applyCustomTheme(const CustomTheme& theme);
};