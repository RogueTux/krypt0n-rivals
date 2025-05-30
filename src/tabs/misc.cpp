#include "../../include/tabs/misc.h"
#include <imgui.h>

void Misc::render() {
    ImGui::BeginChild("MovementSettings", ImVec2(0, 105), true);
    ImGui::Text("Movement Settings");
    ImGui::Separator();

    ImGui::EndChild();

    ImGui::Spacing();

    ImGui::BeginChild("OtherSettings", ImVec2(0, 0), true);
    ImGui::Text("Other Settings");
    ImGui::Separator();

    ImGui::Checkbox("Spectator List", &spectatorList);

    ImGui::EndChild();
}