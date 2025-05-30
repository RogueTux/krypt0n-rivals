#include "../include/Overlay.h"
#include "../include/GameData.h"
#include "../include/Memory.h"
#include "../include/FeatureManager.h"

#include <iostream>
#include <unistd.h>
#include <cstring>
#include <vector>
#include <string>
#include <cmath>
#include <atomic>
#include <algorithm>
#include <X11/extensions/shape.h>
#include <X11/extensions/Xfixes.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <chrono>

#include "EntityScanner.h"

OverlayWindow overlay;
extern std::atomic<bool> g_Running;

unsigned long GetColor(const char* colorName) {
    if (!overlay.display || !overlay.colormap) return 0;
    XColor color, exact_def;
    if (XAllocNamedColor(overlay.display, overlay.colormap, colorName, &color, &exact_def)) return color.pixel;
    if (strcmp(colorName, "white") == 0) return 0xFFFFFF; if (strcmp(colorName, "black") == 0) return 0x000000;
    if (strcmp(colorName, "red") == 0) return 0xFF0000; if (strcmp(colorName, "green") == 0) return 0x00FF00;
    if (strcmp(colorName, "blue") == 0) return 0x0000FF; if (strcmp(colorName, "yellow") == 0) return 0xFFFF00;
    if (strcmp(colorName, "cyan") == 0) return 0x00FFFF; if (strcmp(colorName, "magenta") == 0) return 0xFF00FF;
    return 0x000000;
}

unsigned long ConvertFloatColorToX11(const float rgba[4]) {
    const float r = std::max(0.0f, std::min(1.0f, rgba[0])); float g = std::max(0.0f, std::min(1.0f, rgba[1]));
    const float b = std::max(0.0f, std::min(1.0f, rgba[2]));
    const unsigned long red_val = r * 255.0f;
    const unsigned long green_val = g * 255.0f;
    const unsigned long blue_val = b * 255.0f;
    return (red_val << 16) | (green_val << 8) | blue_val;
}

void DrawHealthBar(const int x, const int y, const int w, const int h, float healthPercent, const unsigned long bgColor, const unsigned long fgColor) {
    if (!overlay.display || !overlay.window || !overlay.gc || h <= 0 || w <= 0) return;
    healthPercent = std::max(0.0f, std::min(1.0f, healthPercent));
    XSetForeground(overlay.display, overlay.gc, bgColor);
    XFillRectangle(overlay.display, overlay.window, overlay.gc, x, y, w, h);
    int fgHeight = static_cast<int>(h * healthPercent);
    const int fgY = y + (h - fgHeight);
    if (fgHeight > 0) {
        XSetForeground(overlay.display, overlay.gc, fgColor);
        XFillRectangle(overlay.display, overlay.window, overlay.gc, x, fgY, w, fgHeight);
    }
}

void DrawCorneredBox(const int x, const int y, const int w, const int h, int cornerSize, const unsigned long color) {
    if (!overlay.display || !overlay.window || !overlay.gc) return;
    cornerSize = std::min({cornerSize, w / 2, h / 2});
    if (cornerSize <= 0) return;
    XSetForeground(overlay.display, overlay.gc, color);
    XDrawLine(overlay.display, overlay.window, overlay.gc, x, y, x + cornerSize, y);
    XDrawLine(overlay.display, overlay.window, overlay.gc, x, y, x, y + cornerSize);
    XDrawLine(overlay.display, overlay.window, overlay.gc, x + w - cornerSize, y, x + w, y);
    XDrawLine(overlay.display, overlay.window, overlay.gc, x + w, y, x + w, y + cornerSize);
    XDrawLine(overlay.display, overlay.window, overlay.gc, x, y + h - cornerSize, x, y + h);
    XDrawLine(overlay.display, overlay.window, overlay.gc, x, y + h, x + cornerSize, y + h);
    XDrawLine(overlay.display, overlay.window, overlay.gc, x + w - cornerSize, y + h, x + w, y + h);
    XDrawLine(overlay.display, overlay.window, overlay.gc, x + w, y + h - cornerSize, x + w, y + h);
}

void DrawCircle(const int centerX, const int centerY, const int radius, const unsigned long color) {
    if (!overlay.display || !overlay.window || !overlay.gc || radius <= 0) return;
    XSetForeground(overlay.display, overlay.gc, color);
    const int x = centerX - radius;
    const int y = centerY - radius;
    const unsigned int width = radius * 2;
    const unsigned int height = radius * 2;
    XDrawArc(overlay.display, overlay.window, overlay.gc, x, y, width, height, 0, 360 * 64);
}

bool SetupOverlayWindow() {
    overlay.display = XOpenDisplay(nullptr);
    if (!overlay.display) { return false; }
    const int defaultScreen = DefaultScreen(overlay.display);
    const Window rootWindow = RootWindow(overlay.display, defaultScreen);
    overlay.screenWidth = GameScreen.Width; overlay.screenHeight = GameScreen.Height;
    if (overlay.screenWidth <= 0 || overlay.screenHeight <= 0) {
         XCloseDisplay(overlay.display); overlay.display = nullptr; return false;
    }
    XVisualInfo vinfo;
    if (!XMatchVisualInfo(overlay.display, defaultScreen, 32, TrueColor, &vinfo)) {
        vinfo.visual = DefaultVisual(overlay.display, defaultScreen); vinfo.depth = DefaultDepth(overlay.display, defaultScreen);
    }
    overlay.colormap = XCreateColormap(overlay.display, rootWindow, vinfo.visual, AllocNone);
    XSetWindowAttributes attrs; attrs.colormap = overlay.colormap; attrs.background_pixel = 0; attrs.border_pixel = 0;
    attrs.override_redirect = True; attrs.event_mask = ExposureMask;
    overlay.window = XCreateWindow(overlay.display, rootWindow, 0, 0, overlay.screenWidth, overlay.screenHeight, 0, vinfo.depth, InputOutput, vinfo.visual, CWColormap | CWBorderPixel | CWBackPixel | CWOverrideRedirect | CWEventMask, &attrs);
    if (!overlay.window) {
        if (overlay.colormap) XFreeColormap(overlay.display, overlay.colormap); XCloseDisplay(overlay.display); overlay.display = nullptr; return false;
    }
    int event_base, error_base;
    if (XFixesQueryExtension(overlay.display, &event_base, &error_base)) {
        if (const XserverRegion region = XFixesCreateRegion(overlay.display, nullptr, 0)) { XFixesSetWindowShapeRegion(overlay.display, overlay.window, ShapeInput, 0, 0, region); XFixesDestroyRegion(overlay.display, region); }
    }
    XStoreName(overlay.display, overlay.window, "Krypt0n Overlay");
    const Atom wmState = XInternAtom(overlay.display, "_NET_WM_STATE", False); Atom wmStateAbove = XInternAtom(overlay.display, "_NET_WM_STATE_ABOVE", False);
    Atom wmStateSticky = XInternAtom(overlay.display, "_NET_WM_STATE_STICKY", False); Atom wmStateSkipTaskbar = XInternAtom(overlay.display, "_NET_WM_STATE_SKIP_TASKBAR", False);
    Atom wmStateSkipPager = XInternAtom(overlay.display, "_NET_WM_STATE_SKIP_PAGER", False);
    const Atom windowType = XInternAtom(overlay.display, "_NET_WM_WINDOW_TYPE", False);
	Atom typeDialog = XInternAtom(overlay.display, "_NET_WM_WINDOW_TYPE_DIALOG", False);
    if (wmStateAbove) XChangeProperty(overlay.display, overlay.window, wmState, XA_ATOM, 32, PropModeReplace, reinterpret_cast<unsigned char *>(&wmStateAbove), 1);
    if (wmStateSticky) XChangeProperty(overlay.display, overlay.window, wmState, XA_ATOM, 32, PropModeAppend, reinterpret_cast<unsigned char *>(&wmStateSticky), 1);
    if (wmStateSkipTaskbar) XChangeProperty(overlay.display, overlay.window, wmState, XA_ATOM, 32, PropModeAppend, reinterpret_cast<unsigned char *>(&wmStateSkipTaskbar), 1);
    if (wmStateSkipPager) XChangeProperty(overlay.display, overlay.window, wmState, XA_ATOM, 32, PropModeAppend, reinterpret_cast<unsigned char *>(&wmStateSkipPager), 1);
    if (typeDialog) XChangeProperty(overlay.display, overlay.window, windowType, XA_ATOM, 32, PropModeReplace, reinterpret_cast<unsigned char *>(&typeDialog), 1);
    XMapRaised(overlay.display, overlay.window); XFlush(overlay.display);
    overlay.gc = XCreateGC(overlay.display, overlay.window, 0, nullptr);
    if (!overlay.gc) {
        XDestroyWindow(overlay.display, overlay.window); XFreeColormap(overlay.display, overlay.colormap); XCloseDisplay(overlay.display); overlay.display = nullptr; return false;
    }
    auto fontname = "-*-helvetica-medium-r-normal-*-12-*-*-*-*-*-iso8859-1"; XFontStruct *font_info = XLoadQueryFont(overlay.display, fontname);
    if (!font_info) { fontname = "fixed"; font_info = XLoadQueryFont(overlay.display, fontname); }
    if (font_info) { XSetFont(overlay.display, overlay.gc, font_info->fid); XFreeFont(overlay.display, font_info); }
    return true;
}

void CleanupOverlayWindow() {
    if (overlay.display) {
        if (overlay.gc) XFreeGC(overlay.display, overlay.gc);
        if (overlay.window) XDestroyWindow(overlay.display, overlay.window);
        if (overlay.colormap) XFreeColormap(overlay.display, overlay.colormap);
        XCloseDisplay(overlay.display);
        overlay.display = nullptr; overlay.window = 0; overlay.gc = nullptr; overlay.colormap = 0;
    }
}
void DrawLine(const int x1, const int y1, const int x2, const int y2, const unsigned long color) {
    if (!overlay.display || !overlay.window || !overlay.gc) return;
    XSetForeground(overlay.display, overlay.gc, color);
    XDrawLine(overlay.display, overlay.window, overlay.gc, x1, y1, x2, y2);
}
void DrawBox(const int x, const int y, const int w, const int h, const unsigned long color) {
     if (!overlay.display || !overlay.window || !overlay.gc || w <= 0 || h <= 0) return;
     XSetForeground(overlay.display, overlay.gc, color);
     XDrawRectangle(overlay.display, overlay.window, overlay.gc, x, y, w, h);
}
void DrawString(const int x, const int y, const char* text, const unsigned long color) {
     if (!overlay.display || !overlay.window || !overlay.gc || !text) return;
     XSetForeground(overlay.display, overlay.gc, color);
     XDrawString(overlay.display, overlay.window, overlay.gc, x, y, text, strlen(text));
}
int GetTextWidth(const char* text) {
    if (!overlay.display || !overlay.gc || !text || !strlen(text)) return 0;
    XFontStruct* font_info = nullptr;
    const GC gc = overlay.gc; XGCValues values;
    if (XGetGCValues(overlay.display, gc, GCFont, &values)) {
        font_info = XQueryFont(overlay.display, values.font);
    }
    int width = 0;
    if (font_info) {
        width = XTextWidth(font_info, text, strlen(text));
        XFreeFontInfo(nullptr, font_info, 0);
    } else {
        width = strlen(text) * 8;
    }
    return width;
}

void* OverlayThreadFunc(void* param) {
    if (!SetupOverlayWindow()) { return nullptr; }

    unsigned long colorText = GetColor("white");
    unsigned long colorHealthBg = GetColor("black");
    const unsigned long colorHealthFgLow = GetColor("red");
    const unsigned long colorHealthFgMed = GetColor("yellow");
    const unsigned long colorHealthFgGood = GetColor("green");

    auto* featureManager = static_cast<FeatureManager*>(param);

    while (g_Running.load()) {
        try {
            bool shouldShowFovCircle = false;
            float aimFovRadius = 0.0f;
            unsigned long colorFovCircle = 0x00FFFF;

            if (featureManager) {
                shouldShowFovCircle = featureManager->showFovCircle;
                aimFovRadius = featureManager->getAccuracySettings().fov;
                colorFovCircle = ConvertFloatColorToX11(featureManager->fovCircleColor);
            }

            while (XPending(overlay.display)) { XEvent event; XNextEvent(overlay.display, &event); }
            XClearWindow(overlay.display, overlay.window);

            if (shouldShowFovCircle && aimFovRadius > 0 && overlay.screenWidth > 0 && overlay.screenHeight > 0) {
                DrawCircle(overlay.screenWidth / 2, overlay.screenHeight / 2, static_cast<int>(aimFovRadius), colorFovCircle);
            }

            if (featureManager && ProcessId > 0 && BaseAddress > 0) {
                uintptr_t localPawnAddr;
                { std::lock_guard<std::mutex> lock(GameDataMutex); localPawnAddr = GameAddrs.AcknowledgedPawn; }

                if (!localPawnAddr) {
                    usleep(100000); continue;
                }

                PlayerLocation localPlayer = GetPlayerLocation(localPawnAddr);
                if (!localPlayer.isValid) { usleep(50000); continue; }

                std::vector<Actors> currentActors = GetEntityList();

                for (const Actors& actor : currentActors) {
                    if (!actor.PlayerActor || !actor.PlayerState) continue;

                    auto [current_health, max_health] = GetHealthInfo(actor);
                    uint8_t isAliveFlag = 0;
                    try { isAliveFlag = ReadMemory<uint8_t>(actor.PlayerState + Offsets::IsAlive); } catch(...) {}

                    if (isAliveFlag == 0 || (current_health <= 0.1f && max_health > 0.001f) ) {
                        continue;
                    }

                    bool actualIsEnemy = isEnemy(actor.PlayerState);
                    AwarenessSettings* currentSettingsToUse = actualIsEnemy ? &featureManager->enemyAwarenessSettings : &featureManager->teamAwarenessSettings;

                    if (!currentSettingsToUse->enabled) {
                        continue;
                    }

                    std::string heroName = "Unknown";
                    if(currentSettingsToUse->showName) heroName = GetHeroName(actor.PlayerState);

                    auto [location, screenPos, isValid] = GetPlayerLocation(actor.PlayerActor);
                    if (!isValid || std::isnan(screenPos.x) || std::isnan(screenPos.y)) continue;
                    if (screenPos.x < -overlay.screenWidth || screenPos.y < -overlay.screenHeight ||
                        screenPos.x >= overlay.screenWidth * 2 || screenPos.y >= overlay.screenHeight*2 ) continue;


                    const float distance = localPlayer.location.Distance(location);
                    if (std::isnan(distance) || distance > currentSettingsToUse->maxDistance) continue;

                    constexpr bool isVisibleFlag = true; // Placeholder
                    const unsigned long drawColor = ConvertFloatColorToX11(currentSettingsToUse->visibleColor);

                    FVector headPosWorld = location;
                    constexpr float headOffset = 75.0f;
                    headPosWorld.z += headOffset;
                    const FVector headScreen = WorldToScreen(headPosWorld);

                    int boxTopY = static_cast<int>(headScreen.y);
                    int boxBottomY = static_cast<int>(screenPos.y);
                    int boxCenterX = static_cast<int>(headScreen.x);

                    if(headScreen.x == -1 && headScreen.y == -1) {
                        boxCenterX = static_cast<int>(screenPos.x);
                    }
                    if (boxTopY > boxBottomY) std::swap(boxTopY, boxBottomY);

                    const int boxHeight = boxBottomY - boxTopY;

                    if (boxHeight < 5 || boxHeight > overlay.screenHeight * 0.95f) {
                        continue;
                    }
                    const int boxWidth = static_cast<int>(boxHeight * 0.55f);
                    const int boxLeftX = boxCenterX - boxWidth / 2;

                    if (currentSettingsToUse->showBox) DrawBox(boxLeftX, boxTopY, boxWidth, boxHeight, drawColor);

                    if (currentSettingsToUse->showHealth && max_health > 0.001f) {
                        const float healthPercent = current_health / max_health;
                        unsigned long currentHealthColor = colorHealthFgGood;
                        if (healthPercent < 0.66f) currentHealthColor = colorHealthFgMed; if (healthPercent < 0.33f) currentHealthColor = colorHealthFgLow;
                        DrawHealthBar(boxLeftX - 6, boxTopY, 4, boxHeight, healthPercent, colorHealthBg, currentHealthColor);
                    }
                    if (currentSettingsToUse->showName && !heroName.empty() && heroName != "Unknown") {
                        DrawString(boxCenterX - GetTextWidth(heroName.c_str()) / 2, boxTopY - 12, heroName.c_str(), colorText);
                    }
                    if (currentSettingsToUse->showDistance) {
                        std::string distStr = std::to_string(static_cast<int>(distance / 100.0f)) + "m";
                        DrawString(boxCenterX - GetTextWidth(distStr.c_str()) / 2, boxBottomY + 12, distStr.c_str(), colorText);
                    }
                }
            }
            XFlush(overlay.display);
        } catch (const std::exception&) {
             usleep(100000);
        } catch (...) {
             usleep(100000);
        }
        usleep(16000);
    }
    CleanupOverlayWindow();
    return nullptr;
}