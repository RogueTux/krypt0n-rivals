#ifndef OVERLAY_H
#define OVERLAY_H

#include <X11/Xlib.h>

struct OverlayWindow {
    Display* display = nullptr;
    Window window = 0;
    GC gc = 0;
    Colormap colormap = 0;
    int screenWidth = 0;
    int screenHeight = 0;
};

extern OverlayWindow overlay;

void* OverlayThreadFunc(void* param);
bool SetupOverlayWindow();
void CleanupOverlayWindow();
void DrawLine(int x1, int y1, int x2, int y2, unsigned long color);
void DrawBox(int x, int y, int w, int h, unsigned long color);
void DrawString(int x, int y, const char* text, unsigned long color);
unsigned long GetColor(const char* colorName);
unsigned long ConvertFloatColorToX11(const float rgba[4]);
void DrawHealthBar(int x, int y, int w, int h, float healthPercent, unsigned long bgColor, unsigned long fgColor);
void DrawCorneredBox(int x, int y, int w, int h, int cornerSize, unsigned long color);
void DrawCircle(int centerX, int centerY, int radius, unsigned long color);
int GetTextWidth(const char* text);

#endif