#pragma once

#include <pspctrl.h>
#include <stdint.h>
#include "kernel.h"

typedef struct _menuState{
    uint8_t menuOpen;
    uint8_t btModuleLoaded;
    uint8_t btModuleFound;
    uint8_t boardConnected;
    uint8_t newConnectionsEnabled;
    SceCtrlData padState;
    SceCtrlData prevPadState;
    ControllerInfo controllers[4];
    uint8_t activeController;
    uint8_t focussedController;
}MenuState;

MenuState* menu_getPointer(void);

void menu_show();
void menu_hide();
void menu_render();
void menu_toggle();
void menuAddError();
void menuAddTry();
void menuSetError(int count);