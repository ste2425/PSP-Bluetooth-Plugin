#ifndef PSP_BT_PLUGIN_KERNEL_H
#define PSP_BT_PLUGIN_KERNEL_H

#if defined (__cplusplus)
extern "C" {
#endif

//#include <pspctrl.h>
//#include <pspiofilemgr.h>

#include <stdint.h>

// Kernel function prototypes
typedef struct {
    uint8_t controllerModel;
    uint8_t batteryLevel;
    bool connected;
} ControllerInfo;

//sio
extern uint8_t BtCtrDriverEnableNewConnections(uint8_t enable);
extern uint8_t BtCtrDriverNewConnectionsEnabled();
extern uint8_t BtCtrDriverLoadControllerInfo(uint8_t controllerIndex, ControllerInfo *info);

#if defined (__cplusplus)
}
#endif

#endif /* PSP_BT_PLUGIN_KERNEL_H */