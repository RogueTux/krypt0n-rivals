#ifndef MOUSE_CONTROL_H
#define MOUSE_CONTROL_H

bool InitMouseControl();
void MoveMouseRelative(int dx, int dy);
void MouseButtonEvent(int button, bool isPressed);
void CleanupMouseControl();

#endif