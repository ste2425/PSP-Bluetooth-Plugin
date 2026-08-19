#ifndef PSP_BT_PLUGIN_KERNEL_H
#define PSP_BT_PLUGIN_KERNEL_H

#if defined (__cplusplus)
extern "C" {
#endif

//#include <pspctrl.h>
//#include <pspiofilemgr.h>

#include <stdint.h>
#include <stdbool.h>


// Kernel function prototypes
typedef struct {
    uint8_t controllerModel;
    uint8_t batteryLevel;
    uint8_t connected;
} ControllerInfo;

//sio
extern void btCtrEnableNewConnections(uint8_t enable);
extern uint8_t btCtrNewConnectionsEnabled();
extern ControllerInfo btCtrLoadControllerInfo(uint8_t controllerIndex);
extern void btCtrSetControllerInfoPolling(bool poll);
extern bool btCtrConnected();
extern void btCtrDisconnectController(uint8_t controllerIndex);
extern int BTCtrTEST();

#if defined (__cplusplus)
}
#endif

#endif /* PSP_BT_PLUGIN_KERNEL_H */