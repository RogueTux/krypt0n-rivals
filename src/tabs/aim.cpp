#include "../../include/tabs/aim.h"
#include "../../include/FeatureManager.h"
#include <imgui.h>
#include <algorithm>
#include <string>
#include <cstring>
#include <GLFW/glfw3.h>
#include <map>
#include <X11/Xlib.h>
#include <X11/keysym.h>

std::map<int, KeySym> GlfwToX11KeySym_BinderCheck_AimTab = {
    { GLFW_KEY_SPACE, XK_space }, { GLFW_KEY_APOSTROPHE, XK_apostrophe },
    { GLFW_KEY_COMMA, XK_comma }, { GLFW_KEY_MINUS, XK_minus },
    { GLFW_KEY_PERIOD, XK_period }, { GLFW_KEY_SLASH, XK_slash },
    { GLFW_KEY_0, XK_0 }, { GLFW_KEY_1, XK_1 }, { GLFW_KEY_2, XK_2 },
    { GLFW_KEY_3, XK_3 }, { GLFW_KEY_4, XK_4 }, { GLFW_KEY_5, XK_5 },
    { GLFW_KEY_6, XK_6 }, { GLFW_KEY_7, XK_7 }, { GLFW_KEY_8, XK_8 },
    { GLFW_KEY_9, XK_9 }, { GLFW_KEY_SEMICOLON, XK_semicolon },
    { GLFW_KEY_EQUAL, XK_equal },
    { GLFW_KEY_A, XK_a }, { GLFW_KEY_B, XK_b }, { GLFW_KEY_C, XK_c },
    { GLFW_KEY_D, XK_d }, { GLFW_KEY_E, XK_e }, { GLFW_KEY_F, XK_f },
    { GLFW_KEY_G, XK_g }, { GLFW_KEY_H, XK_h }, { GLFW_KEY_I, XK_i },
    { GLFW_KEY_J, XK_j }, { GLFW_KEY_K, XK_k }, { GLFW_KEY_L, XK_l },
    { GLFW_KEY_M, XK_m }, { GLFW_KEY_N, XK_n }, { GLFW_KEY_O, XK_o },
    { GLFW_KEY_P, XK_p }, { GLFW_KEY_Q, XK_q }, { GLFW_KEY_R, XK_r },
    { GLFW_KEY_S, XK_s }, { GLFW_KEY_T, XK_t }, { GLFW_KEY_U, XK_u },
    { GLFW_KEY_V, XK_v }, { GLFW_KEY_W, XK_w }, { GLFW_KEY_X, XK_x },
    { GLFW_KEY_Y, XK_y }, { GLFW_KEY_Z, XK_z },
    { GLFW_KEY_LEFT_BRACKET, XK_bracketleft },
    { GLFW_KEY_BACKSLASH, XK_backslash },
    { GLFW_KEY_RIGHT_BRACKET, XK_bracketright },
    { GLFW_KEY_GRAVE_ACCENT, XK_grave },
    { GLFW_KEY_ESCAPE, XK_Escape }, { GLFW_KEY_ENTER, XK_Return },
    { GLFW_KEY_TAB, XK_Tab }, { GLFW_KEY_BACKSPACE, XK_BackSpace },
    { GLFW_KEY_INSERT, XK_Insert }, { GLFW_KEY_DELETE, XK_Delete },
    { GLFW_KEY_RIGHT, XK_Right }, { GLFW_KEY_LEFT, XK_Left },
    { GLFW_KEY_DOWN, XK_Down }, { GLFW_KEY_UP, XK_Up },
    { GLFW_KEY_PAGE_UP, XK_Page_Up }, { GLFW_KEY_PAGE_DOWN, XK_Page_Down },
    { GLFW_KEY_HOME, XK_Home }, { GLFW_KEY_END, XK_End },
    { GLFW_KEY_CAPS_LOCK, XK_Caps_Lock }, { GLFW_KEY_SCROLL_LOCK, XK_Scroll_Lock },
    { GLFW_KEY_NUM_LOCK, XK_Num_Lock }, { GLFW_KEY_PRINT_SCREEN, XK_Print },
    { GLFW_KEY_PAUSE, XK_Pause },
    { GLFW_KEY_F1, XK_F1 }, { GLFW_KEY_F2, XK_F2 }, { GLFW_KEY_F3, XK_F3 },
    { GLFW_KEY_F4, XK_F4 }, { GLFW_KEY_F5, XK_F5 }, { GLFW_KEY_F6, XK_F6 },
    { GLFW_KEY_F7, XK_F7 }, { GLFW_KEY_F8, XK_F8 }, { GLFW_KEY_F9, XK_F9 },
    { GLFW_KEY_F10, XK_F10 }, { GLFW_KEY_F11, XK_F11 }, { GLFW_KEY_F12, XK_F12 },
    { GLFW_KEY_KP_0, XK_KP_0 }, { GLFW_KEY_KP_1, XK_KP_1 }, { GLFW_KEY_KP_2, XK_KP_2 },
    { GLFW_KEY_KP_3, XK_KP_3 }, { GLFW_KEY_KP_4, XK_KP_4 }, { GLFW_KEY_KP_5, XK_KP_5 },
    { GLFW_KEY_KP_6, XK_KP_6 }, { GLFW_KEY_KP_7, XK_KP_7 }, { GLFW_KEY_KP_8, XK_KP_8 },
    { GLFW_KEY_KP_9, XK_KP_9 }, { GLFW_KEY_KP_DECIMAL, XK_KP_Decimal },
    { GLFW_KEY_KP_DIVIDE, XK_KP_Divide }, { GLFW_KEY_KP_MULTIPLY, XK_KP_Multiply },
    { GLFW_KEY_KP_SUBTRACT, XK_KP_Subtract }, { GLFW_KEY_KP_ADD, XK_KP_Add },
    { GLFW_KEY_KP_ENTER, XK_KP_Enter }, { GLFW_KEY_KP_EQUAL, XK_KP_Equal },
    { GLFW_KEY_LEFT_SHIFT, XK_Shift_L }, { GLFW_KEY_LEFT_CONTROL, XK_Control_L },
    { GLFW_KEY_LEFT_ALT, XK_Alt_L }, { GLFW_KEY_LEFT_SUPER, XK_Super_L },
    { GLFW_KEY_RIGHT_SHIFT, XK_Shift_R }, { GLFW_KEY_RIGHT_CONTROL, XK_Control_R },
    { GLFW_KEY_RIGHT_ALT, XK_Alt_R }, { GLFW_KEY_RIGHT_SUPER, XK_Super_R },
    { GLFW_KEY_MENU, XK_Menu }
};


std::string getKeyName(const int keyCode) {
    if (keyCode >= 1 && keyCode <= 8) {
        if (keyCode - 1 >= GLFW_MOUSE_BUTTON_1 && keyCode - 1 <= GLFW_MOUSE_BUTTON_LAST) {
             return "MOUSE" + std::to_string(keyCode);
        }
        return "MOUSE?";
    }
    if (keyCode > 8) {
        if (const char* glfwName = glfwGetKeyName(keyCode, 0)) {
            std::string name = glfwName;
            std::transform(name.begin(), name.end(), name.begin(), toupper);
            return name;
        }
        switch(keyCode) {
            case GLFW_KEY_LEFT_SHIFT: return "LSHIFT"; case GLFW_KEY_RIGHT_SHIFT: return "RSHIFT";
            case GLFW_KEY_LEFT_CONTROL: return "LCTRL"; case GLFW_KEY_RIGHT_CONTROL: return "RCTRL";
            case GLFW_KEY_LEFT_ALT: return "LALT"; case GLFW_KEY_RIGHT_ALT: return "RALT";
            case GLFW_KEY_LEFT_SUPER: return "LSUPER"; case GLFW_KEY_RIGHT_SUPER: return "RSUPER";
            case GLFW_KEY_CAPS_LOCK: return "CAPSLOCK"; case GLFW_KEY_ESCAPE: return "ESC";
            case GLFW_KEY_TAB: return "TAB"; case GLFW_KEY_ENTER: return "ENTER";
            case GLFW_KEY_BACKSPACE: return "BKSPACE"; case GLFW_KEY_DELETE: return "DEL";
            case GLFW_KEY_INSERT: return "INS"; case GLFW_KEY_SPACE: return "SPACE";
            case GLFW_KEY_F1: return "F1"; case GLFW_KEY_F2: return "F2"; case GLFW_KEY_F3: return "F3";
            case GLFW_KEY_F4: return "F4"; case GLFW_KEY_F5: return "F5"; case GLFW_KEY_F6: return "F6";
            case GLFW_KEY_F7: return "F7"; case GLFW_KEY_F8: return "F8"; case GLFW_KEY_F9: return "F9";
            case GLFW_KEY_F10: return "F10"; case GLFW_KEY_F11: return "F11"; case GLFW_KEY_F12: return "F12";
            case GLFW_KEY_KP_ENTER: return "NUM_ENTER"; case GLFW_KEY_UP: return "UP";
            case GLFW_KEY_DOWN: return "DOWN"; case GLFW_KEY_LEFT: return "LEFT"; case GLFW_KEY_RIGHT: return "RIGHT";
            default: return "KEY?" + std::to_string(keyCode);
        }
    }
    return "None";
}

bool DrawKeyBinder(const char* label, int* key_code_ptr, const char* tooltip) {
    static int* waiting_for_input_ptr = nullptr;
    const bool is_waiting = (waiting_for_input_ptr == key_code_ptr);
    bool value_changed = false;
    static bool show_invalid_key_tooltip = false;

    ImGui::PushID(label);
    ImGui::TextUnformatted(label);
    ImGui::SameLine();

    std::string button_text = "[...]";
    if (is_waiting) {
        button_text = "[Press key/button]";
    } else if (key_code_ptr && *key_code_ptr != 0) {
        button_text = "[" + getKeyName(*key_code_ptr) + "]";
    }

    if (ImGui::Button(button_text.c_str(), ImVec2(120, 0))) {
        waiting_for_input_ptr = key_code_ptr;
        show_invalid_key_tooltip = false;
    }

    if ((tooltip && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) || (is_waiting && show_invalid_key_tooltip)) {
        if (show_invalid_key_tooltip) {
            ImGui::SetTooltip("Key/Button cannot be reliably checked by X11.\nPlease choose another (e.g., Letters, Numbers, F-Keys, Mouse 1-3).");
        } else if (tooltip) {
            ImGui::SetTooltip("%s", tooltip);
        }
    }

    if (is_waiting) {
        if (GLFWwindow* window = glfwGetCurrentContext(); window && key_code_ptr) {
            bool key_assigned_or_cancelled = false;
            bool key_pressed_but_invalid = false;

            for (int glfw_mouse_button = GLFW_MOUSE_BUTTON_LEFT; glfw_mouse_button <= GLFW_MOUSE_BUTTON_LAST; ++glfw_mouse_button) {
                if (glfwGetMouseButton(window, glfw_mouse_button) == GLFW_PRESS) {
                    if (const int assigned_code = glfw_mouse_button + 1; assigned_code >= 1 && assigned_code <= 3) {
                        if (*key_code_ptr != assigned_code) {
                            *key_code_ptr = assigned_code;
                            value_changed = true;
                        }
                        key_assigned_or_cancelled = true;
                    } else {
                        key_pressed_but_invalid = true;
                    }
                    break;
                }
            }

            if (!key_assigned_or_cancelled && !key_pressed_but_invalid) {
                for (int glfw_key_code = GLFW_KEY_SPACE; glfw_key_code <= GLFW_KEY_LAST; ++glfw_key_code) {
                    if (glfw_key_code == GLFW_KEY_ESCAPE || glfw_key_code == GLFW_KEY_UNKNOWN)
                    {
                        continue;
                    }

                    if (glfwGetKey(window, glfw_key_code) == GLFW_PRESS) {
                         if (GlfwToX11KeySym_BinderCheck_AimTab.count(glfw_key_code)) {
                             if (*key_code_ptr != glfw_key_code) {
                                 *key_code_ptr = glfw_key_code;
                                 value_changed = true;
                             }
                             key_assigned_or_cancelled = true;
                         } else {
                             key_pressed_but_invalid = true;
                         }
                        break;
                    }
                }
            }

            if (!key_assigned_or_cancelled && !key_pressed_but_invalid &&
                glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            {
                key_assigned_or_cancelled = true;
            }

            if (key_assigned_or_cancelled) {
                waiting_for_input_ptr = nullptr;
                show_invalid_key_tooltip = false;
            }
            else if (key_pressed_but_invalid) {
                 show_invalid_key_tooltip = true;
            }

        } else {
            waiting_for_input_ptr = nullptr;
            show_invalid_key_tooltip = false;
        }
    } else {
        if (show_invalid_key_tooltip && waiting_for_input_ptr != key_code_ptr){
             show_invalid_key_tooltip = false;
        }
    }

    ImGui::PopID();
    return value_changed;
}

void Aim::render() {
    if (!featureManager) {
        ImGui::Text("Feature manager not initialized!");
        return;
    }

    AccuracySettings& settings = featureManager->getAccuracySettings();

    ImGui::BeginChild("Aim", ImVec2(0, 0), false);

    ImGui::BeginChild("General", ImVec2(0, 440), true);
    ImGui::Text("General Settings");
    ImGui::Separator();

    ImGui::Checkbox("Enable Accuracy Assist", &settings.enabled);

    DrawKeyBinder("Aim Key", &settings.aimKey, "Click the button and press the desired key/mouse button (Mouse 1-3, standard keys).");

    ImGui::SliderFloat("FOV", &settings.fov, 1.0f, 180.0f, "%.1f (pixels)");
    ImGui::SliderFloat("Smooth Factor", &settings.smoothing, 1.0f, 30.0f, "%.1f");

    const char* targets[] = { "Head", "Body" };
    int currentBoneTarget = (settings.boneTarget == 8) ? 0 : 1;
    if (ImGui::Combo("Target Priority", &currentBoneTarget, targets, IM_ARRAYSIZE(targets))) {
        settings.boneTarget = (currentBoneTarget == 0) ? 8 : 5;
    }

    ImGui::Checkbox("Target Allies", &settings.targetAllies);
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("When enabled, the accuracy assist will target allies in addition to enemies");
    }

    ImGui::Separator();

    ImGui::Text("Advanced Settings");
    ImGui::Checkbox("Use Fallback Aiming", &settings.useFallbackAiming);

    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("When enabled, falls back to estimated head/body position when bone targeting fails");
    }

    ImGui::EndChild();
    ImGui::Spacing();
    ImGui::EndChild();
}