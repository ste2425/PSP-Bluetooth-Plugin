#include <pspsdk.h>
#include <psptypes.h>
#include <pspkerror.h>
#include <pspkerneltypes.h>
#include <pspthreadman.h>

#include <stdbool.h>
#include <inttypes.h>

#include "btCtr.h"
#include "ctrl_imports.h"

#define str(s) #s // For stringizing defines
#define xstr(s) str(s)

#define MODULE_NAME "BTControllerModule"
#define MAJOR_VER 1
#define MINOR_VER 1

#define MODULE_OK       0
#define MODULE_ERROR    1

//
// PSP SDK
//
// We are building a kernel mode prx plugin
PSP_MODULE_INFO(MODULE_NAME, PSP_MODULE_KERNEL, MAJOR_VER, MINOR_VER);

// We don't allocate any heap memory, so set this to 0.
PSP_HEAP_SIZE_KB(0);

// We don't need a main thread since we only do basic setup during module start and won't stall module loading.
// This will make us be called from the module loader thread directly, instead of a secondary kernel thread.
PSP_NO_CREATE_MAIN_THREAD();

// We don't need any of the newlib features since we're not calling into stdio or stdlib etc
PSP_DISABLE_NEWLIB();

//
// Forward declarations
//
static int main_thread(SceSize args, void *argp);
static int start_main_thread(void);
static int stop_main_thread(void);
int module_start(SceSize args, void *argp);
int module_stop(SceSize args, void *argp);

//
// Globals
//
static SceUID g_mainThreadId = -1;
static SceUID g_sioThreadId = -1;
static SceUInt g_button_state = 0;
static uint8_t g_sio_running = 0;
static uint8_t loaded = 0;

//
// Threads
//

uint8_t BtCtrDriverEnableNewConnections(uint8_t enable) {
    if (enable) {
        return BTCtrEnableConnections();
    } else {
        return BTCtrDisableConnections();
    }
}

uint8_t BtCtrDriverNewConnectionsEnabled() {
    return BTCtrNewConnectionsEnabled();
}

uint8_t BtCtrDriverLoadControllerInfo(uint8_t controllerIndex, ControllerInfo *info) {
    return BTCtrLoadControllerInfo(controllerIndex, info);
}

uint32_t migrate_dpad(int buttons, uint8_t dpad)
{
    if (dpad & BT_DPAD_UP) buttons |= SCE_CTRL_UP;  // up
    if (dpad & BT_DPAD_DOWN) buttons |= SCE_CTRL_DOWN;  // down
    if (dpad & BT_DPAD_LEFT) buttons |= SCE_CTRL_LEFT;  // left
    if (dpad & BT_DPAD_RIGHT) buttons |= SCE_CTRL_RIGHT;  // right

    return buttons;
}

uint32_t migrate_buttons(int buttons, uint8_t btButtons)
{
    if (btButtons & BT_BUTTON_X) buttons |= SCE_CTRL_CROSS;  // up
    if (btButtons & BT_BUTTON_CIRCLE) buttons |= SCE_CTRL_CIRCLE;  // down
    if (btButtons & BT_BUTTON_SQUARE) buttons |= SCE_CTRL_SQUARE;  // left
    if (btButtons & BT_BUTTON_TRIANGLE) buttons |= SCE_CTRL_TRIANGLE;  // right

    return buttons;
}

//
// Controller callback function based on padsvc decomp
//
static
s32 ctrl_input_data_handler_func(void *pSrc, SceCtrlData2 *pDst)
{
    SceUInt* p_new_buttons = (SceUInt*)pSrc;
    SceUInt new_buttons = p_new_buttons != NULL ? *p_new_buttons : 0;

    auto aX = 0;
    auto aY = 0;
    auto lX = 0;
    auto lY = 0;
    BTCtr controllerState = BTCtrGetControllerState(0);

    if (controllerState.connected) {
        aX = controllerState.analogLX;
        aY = controllerState.analogLY;

        lX = controllerState.analogRX;
        lY = controllerState.analogRY;

        new_buttons =  migrate_dpad(new_buttons, controllerState.dpad);
        new_buttons = migrate_buttons(new_buttons, controllerState.buttons);
    }

    pDst->buttons = new_buttons;
    pDst->DPadSenseA = 0;
    pDst->DPadSenseB = 0;
    pDst->GPadSenseA = 0;
    pDst->GPadSenseB = 0;
    pDst->AxisSenseA = 0;
    pDst->AxisSenseB = 0;
    pDst->TiltA = 0;
    pDst->TiltB = 0;
    pDst->aX = aX;
    pDst->aY = aY;
    pDst->rsrv[0] = lX;
    pDst->rsrv[1] = lY;

    // Success
    return 0;
}

static
int sio_thread(SceSize args, void *argp)
{
    while (g_sio_running) {
        BTCtrUpdate();
        sceKernelDelayThread(10000); // 10 ms
    }

    return 0;
}

// The main thread.
static
int main_thread(SceSize args, void *argp)
{
    SceUID ctrl_input_handler_id = -1;
    //
    // Setup
    //

    // sceCtrl_driver_6C86AF22() enables passing through controller state from a specific external controller port buffer
    // into the global controller buffer.
    //
    // This allows it to be read with the basic controller read methods that return SceCtrlData, such as
    // sceCtrlReadBufferPositive.
    //
    // The argument is a bit field with bits set corresponding to the port.
    // 0x01 enables SCE_CTRL_PORT_DS3 passthrough
    // 0x02 enables SCE_CTRL_PORT_UNKNOWN_2
    // 0x00 disables passthrough such that it can only be read by the extended/extra methods that return SceCtrlData2,
    // such as sceCtrlReadBufferPositive2(), that takes the specific port number as an argument 
    sceCtrl_driver_6C86AF22(SCE_CTRL_PORT_DS3);

    // Setup sceCtrl_driver_E467BEC8() external controller port input handler.
    // This effectively sets up an additional source for controller input polling, similar to how the DS3 controller
    // is wired up internally to padsvc (Bluetooth -> DS3) on PSP Go.
    //
    // The copyInputData function is type SceCtrlInputDataTransferHandler and is called on every polling loop.
    SceCtrlInputDataTransferHandler controller_data_transfer_handler = {
        .unk1 = sizeof(SceCtrlInputDataTransferHandler),
        .copyInputData = ctrl_input_data_handler_func
    };

    // sceCtrl_driver_E467BEC8(u8 externalPort, SceCtrlInputDataTransferHandler *transferHandler, void *inputSource)
    // The inputSource ptr is passed through into the handler function as the first argument.
    ctrl_input_handler_id = sceCtrl_driver_E467BEC8(SCE_CTRL_PORT_DS3, &controller_data_transfer_handler, &g_button_state);

    //if(ctrl_input_handler_id != SCE_ERROR_OK) {
      //  DEBUG_PRINT("Failed to register controller input handler: ret 0x%08x\n", ctrl_input_handler_id);
   // }

    sceCtrlSetSamplingMode(SCE_CTRL_INPUT_DIGITAL_ANALOG);
    //
    // Sleep and process callbacks until we get woken up
    //
    sceKernelSleepThreadCB();

    //
    // Cleanup
    //

    if(ctrl_input_handler_id >= 0) {
        sceCtrl_driver_E467BEC8(SCE_CTRL_PORT_DS3, NULL, NULL);
        //if(result < 0) {
        //    DEBUG_PRINT("Failed to deregister controller input handler: ret 0x%08x\n", result);
       // }
    }

    return 0;
}

//
// Thread management
//
static
int start_sio_thread(void) 
{
    int result;
    SceUID thid;

    BTCtrSetup();

    g_sio_running = 1;

    //return 0;

    // name, entry, initPriority, stackSize, PspThreadAttributes, SceKernelThreadOptParam
    thid = sceKernelCreateThread(MODULE_NAME "SioThread", sio_thread, 0x11, 0x800, 0, 0);
    if (thid >= 0) {
        result = sceKernelStartThread(thid, 0, 0);
        if(result < 0) {
           // DEBUG_PRINT("Failed to start sio thread: ret 0x%08x\n", result);
        }

        g_sioThreadId = thid;
    }
    else {
        result = thid;
    }

    return result;
}

static 
int stop_sio_thread(void) 
{
    int result = 0;
    SceUID thid = g_sioThreadId;

    BTCtrTerminate();

    if(thid >= 0) {

        g_sio_running = 0;

        // Wait for the main thread to clean up and exit
        result = sceKernelWaitThreadEnd(thid, NULL);
        if(result < 0) {
            // Thread did not stop, force terminate and delete it
            result = sceKernelTerminateDeleteThread(thid);
            if(result >= 0) {
                g_sioThreadId = -1;
            }
        }
        else {
            // Thead stopped cleanly, delete it
            result = sceKernelDeleteThread(thid);
            if(result >= 0) {
                g_sioThreadId = -1;
            }
        }
    }

    return result;
}

static
int start_main_thread(void)
{
    int result;
    SceUID thid;

    // name, entry, initPriority, stackSize, PspThreadAttributes, SceKernelThreadOptParam
    thid = sceKernelCreateThread(MODULE_NAME "MainThread", main_thread, 0x11, 0x800, 1, 0);
    if (thid >= 0) {
        result = sceKernelStartThread(thid, 0, 0);
        if(result < 0) {
           // DEBUG_PRINT("Failed to start main thread: ret 0x%08x\n", result);
        }

        g_mainThreadId = thid;
    }
    else {
        result = thid;
    }

    return result;
}

static
int stop_main_thread(void)
{
    int result = 0;
    SceUID thid = g_mainThreadId;

    if(thid >= 0) {
        // Unblock sceKernelSleepThreadCB() and have thread begin cleanup
        result = sceKernelWakeupThread(thid);
        if(result < 0) {
            //DEBUG_PRINT("Failed to wakeup main thread: ret 0x%08x\n", result);
        }

        // Wait for the main thread to clean up and exit
        result = sceKernelWaitThreadEnd(thid, NULL);
        if(result < 0) {
            // Thread did not stop, force terminate and delete it
            result = sceKernelTerminateDeleteThread(thid);
            if(result >= 0) {
                g_mainThreadId = -1;
            }
        }
        else {
            // Thead stopped cleanly, delete it
            result = sceKernelDeleteThread(thid);
            if(result >= 0) {
                g_mainThreadId = -1;
            }
        }
    }

    return result;
}

//
// Module Event Handlers
//

// Called during module init
int module_start(SceSize args, void *argp)
{
    int result = start_main_thread();
    int sio_result = start_sio_thread();

    if(result < 0 || sio_result < 0) {
        return MODULE_ERROR;
    }

    loaded = 1;

    return MODULE_OK;
}

// Called during module deinit
int module_stop(SceSize args, void *argp)
{
    int result = stop_main_thread();
    int sio_result = stop_sio_thread();

    if(result < 0 || sio_result < 0) {
        return MODULE_ERROR;
    }

    return MODULE_OK;
}