#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <iostream>
#include <unistd.h>
#include <pthread.h>
#include <csignal>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <map>
#include <cmath>
#include <thread>
#include <atomic>

#include "../include/menu.h"
#include "../include/theme.h"
#include "../include/EntityScanner.h"
#include "../include/Memory.h"
#include "../include/GameData.h"
#include "../include/AccuracyAssist.h"
#include "../include/Utils.h"
#include "../include/FeatureManager.h"
#include "../include/MouseControl.h"

std::atomic<bool> g_Running = true;

static void glfw_error_callback(int error, const char* description) {
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

void initializeImGuiStyle() {
    Theme::applyTheme();
}

std::map<int, KeySym> GlfwToX11KeySym = {
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

bool isX11KeyPressed(Display* display, KeySym ks) {
    if (!display) return false;
    KeyCode kc = XKeysymToKeycode(display, ks);
    if (kc == 0) return false;
    char keys_return[32];
    XQueryKeymap(display, keys_return);
    return (keys_return[kc / 8] & (1 << (kc % 8))) != 0;
}

bool isX11MouseButtonPressed(Display* display, unsigned int button) {
     if (!display) return false;
    Window root_return, child_return;
    int root_x_return, root_y_return, win_x_return, win_y_return;
    unsigned int mask_return;
    XQueryPointer(display, DefaultRootWindow(display), &root_return, &child_return,
                  &root_x_return, &root_y_return, &win_x_return, &win_y_return,
                  &mask_return);
    unsigned int button_mask = 0;
    switch(button) {
        case 1: button_mask = Button1Mask; break;
        case 2: button_mask = Button3Mask; break;
        case 3: button_mask = Button2Mask; break;
        default: return false;
    }
    return (mask_return & button_mask) != 0;
}

int main(int, char**) {
    ProcessId = 0;
    BaseAddress = 0;

    Display* mainDisplay = XOpenDisplay(nullptr);
    if (!mainDisplay) {
        return 1;
    }

    if (getuid() != 0) {
        XCloseDisplay(mainDisplay);
        sleep(3);
        return 1;
    }

    if (!InitMouseControl()) {
        XCloseDisplay(mainDisplay);
        return 1;
    }

    signal(SIGINT, SignalHandler);
    signal(SIGTERM, SignalHandler);

    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) {
         CleanupMouseControl();
         XCloseDisplay(mainDisplay);
         return 1;
    }

    const auto glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_FLOATING, GLFW_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
    glfwWindowHint(GLFW_FOCUS_ON_SHOW, GLFW_TRUE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_FALSE);
    glfwWindowHint(GLFW_MOUSE_PASSTHROUGH, GLFW_FALSE);


    GLFWwindow* window = glfwCreateWindow(800, 600, "Krypt0n Menu", nullptr, nullptr);
    if (window == nullptr) {
        CleanupMouseControl();
        XCloseDisplay(mainDisplay);
        glfwTerminate();
        return 1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    if (glewInit() != GLEW_OK) {
        CleanupMouseControl();
        XCloseDisplay(mainDisplay);
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags &= ~ImGuiConfigFlags_NoMouseCursorChange;

    initializeImGuiStyle();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    FeatureManager featureManager;
    bool initialized = false;
    try {
        if (GLFWmonitor* primary = glfwGetPrimaryMonitor()) {
            if (const GLFWvidmode* mode = glfwGetVideoMode(primary)) {
                GameScreen.Width = mode->width;
                GameScreen.Height = mode->height;
            }
        }

        if (featureManager.initialize()) {
             initialized = true;
        } else {
             CleanupMouseControl();
             XCloseDisplay(mainDisplay);
             glfwDestroyWindow(window);
             glfwTerminate();
             return 1;
        }
    } catch (const std::exception&) {
         CleanupMouseControl();
         XCloseDisplay(mainDisplay);
         glfwDestroyWindow(window);
         glfwTerminate();
         return 1;
    } catch (...) {
         CleanupMouseControl();
         XCloseDisplay(mainDisplay);
         glfwDestroyWindow(window);
         glfwTerminate();
         return 1;
    }

    CustomMenu menu(&featureManager, &g_Running);

    while (g_Running.load() && !glfwWindowShouldClose(window)) {
        try {
            glfwPollEvents();

            if (initialized) {
                int aimKey = featureManager.getAccuracySettings().aimKey;
                bool currentlyPressed = false;
                if (aimKey != 0) {
                    if (aimKey >= 1 && aimKey <= 8) {
                        currentlyPressed = isX11MouseButtonPressed(mainDisplay, aimKey);
                    } else if (aimKey > 8) {
                        if (auto it = GlfwToX11KeySym.find(aimKey); it != GlfwToX11KeySym.end()) {
                            currentlyPressed = isX11KeyPressed(mainDisplay, it->second);
                        }
                    }
                }
                featureManager.isAssistKeyPressed.store(currentlyPressed);
            }

            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            if (initialized) {
                featureManager.update();
                const float dx = featureManager.accuracyDeltaX.exchange(0.0f);
                if (const float dy = featureManager.accuracyDeltaY.exchange(0.0f); (std::abs(dx) > 0.1f || std::abs(dy) > 0.1f) && featureManager.isAssistKeyPressed.load()) {
                    const int moveX = static_cast<int>(roundf(dx));
                    if (const int moveY = static_cast<int>(roundf(dy)); moveX != 0 || moveY != 0) {
                         MoveMouseRelative(moveX, moveY);
                    }
                }
            }

            menu.render();

            ImGui::Render();
            int display_w, display_h;
            glfwGetFramebufferSize(window, &display_w, &display_h);
            glViewport(0, 0, display_w, display_h);
            ImVec4 menuBgColor = ImGui::GetStyle().Colors[ImGuiCol_WindowBg];
            glClearColor(menuBgColor.x * menuBgColor.w, menuBgColor.y * menuBgColor.w, menuBgColor.z * menuBgColor.w, menuBgColor.w);
            glClear(GL_COLOR_BUFFER_BIT);
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            glfwSwapBuffers(window);

            std::this_thread::sleep_for(std::chrono::milliseconds(5));

        } catch (const std::exception&) {
             std::this_thread::sleep_for(std::chrono::milliseconds(100));
        } catch (...) {
             std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }

    g_Running.store(false);

    if (initialized) {
         try { featureManager.shutdown(); } catch (...) {}
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    CleanupMouseControl();

    if (mainDisplay) {
        XCloseDisplay(mainDisplay);
    }

    return 0;
}