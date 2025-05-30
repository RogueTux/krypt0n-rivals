#include "../include/MouseControl.h"
#include <libevdev/libevdev.h>
#include <libevdev/libevdev-uinput.h>
#include <unistd.h>

static libevdev *dev = nullptr;
static libevdev_uinput *uidev = nullptr;

bool InitMouseControl() {
    dev = libevdev_new();
    if (!dev) {
        return false;
    }

    libevdev_set_name(dev, "Krypt0n Virtual Mouse");
    libevdev_enable_event_type(dev, EV_REL);
    libevdev_enable_event_code(dev, EV_REL, REL_X, nullptr);
    libevdev_enable_event_code(dev, EV_REL, REL_Y, nullptr);

    libevdev_enable_event_type(dev, EV_KEY);
    libevdev_enable_event_code(dev, EV_KEY, BTN_LEFT, nullptr);
    libevdev_enable_event_code(dev, EV_KEY, BTN_MIDDLE, nullptr);
    libevdev_enable_event_code(dev, EV_KEY, BTN_RIGHT, nullptr);

    libevdev_set_id_bustype(dev, BUS_VIRTUAL);
    libevdev_set_id_vendor(dev, 0x1234);
    libevdev_set_id_product(dev, 0x5678);
    libevdev_set_id_version(dev, 0x0100);

    if (const int err = libevdev_uinput_create_from_device(dev, LIBEVDEV_UINPUT_OPEN_MANAGED, &uidev); err != 0) {
        libevdev_free(dev);
        dev = nullptr;
        return false;
    }
    usleep(50000);
    return true;
}

void MoveMouseRelative(const int dx, const int dy) {
    if (!uidev) {
        return;
    }
    if (dx == 0 && dy == 0) {
        return;
    }

    int err = 0;
    if (dx != 0) {
        err = libevdev_uinput_write_event(uidev, EV_REL, REL_X, dx);
        if (err < 0) {
        }
    }
    if (dy != 0) {
        err = libevdev_uinput_write_event(uidev, EV_REL, REL_Y, dy);
         if (err < 0) {
        }
    }

    err = libevdev_uinput_write_event(uidev, EV_SYN, SYN_REPORT, 0);
    if (err < 0) {
    }
}

void MouseButtonEvent(const int button, const bool isPressed) {
    if (!uidev) {
        return;
    }

    int btnCode;
    switch (button) {
        case 1: btnCode = BTN_LEFT; break;
        case 2: btnCode = BTN_MIDDLE; break;
        case 3: btnCode = BTN_RIGHT; break;
        default:
            return;
    }

    int err = libevdev_uinput_write_event(uidev, EV_KEY, btnCode, isPressed ? 1 : 0);
    if (err < 0) {
    }

    err = libevdev_uinput_write_event(uidev, EV_SYN, SYN_REPORT, 0);
     if (err < 0) {
    }
}

void CleanupMouseControl() {
    if (uidev) {
        libevdev_uinput_destroy(uidev);
        uidev = nullptr;
    }

    if (dev) {
        dev = nullptr;
    }
}