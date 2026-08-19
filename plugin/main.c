#include <pspsdk.h>
#include <psptypes.h>
#include <pspkerror.h>
#include <pspkerneltypes.h>
#include <pspthreadman.h>

#include <stdbool.h>
#include <inttypes.h>

#include "btCtr.h"
#include "ctrl_imports.h"
#include "controllerPatching.h"

#define str(s) #s // For stringizing defines
#define xstr(s) str(s)

#define MODULE_NAME "BTControllerModule"
#define MAJOR_VER 1
#define MINOR_VER 1

#define MODULE_OK       0
#define MODULE_ERROR    1

// TODO - review all this setup, dont fully understand it all ad overlay plugin is different.
PSP_MODULE_INFO(MODULE_NAME, PSP_MODULE_KERNEL, MAJOR_VER, MINOR_VER);

// We don't allocate any heap memory, so set this to 0.
PSP_HEAP_SIZE_KB(0);

// We don't need a main thread since we only do basic setup during module start and won't stall module loading.
// This will make us be called from the module loader thread directly, instead of a secondary kernel thread.
PSP_NO_CREATE_MAIN_THREAD();

// We don't need any of the newlib features since we're not calling into stdio or stdlib etc
PSP_DISABLE_NEWLIB();

//
// Globals
//
static SceUID g_mainThreadId = -1;
static SceUID g_sioThreadId = -1;
static uint8_t g_sio_running = 0;


int sceKernelRegisterResumeHandler(int reg, int (*handler)(int unk, void *param), void *param);
int sceKernelRegisterSuspendHandler(int reg, int (*handler)(int unk, void *param), void *param);   

// -------------------------------

static
int controller_polling_thread(SceSize args, void *argp)
{
    BTCtrSetup();

    while (g_sio_running) {
        //BTCtrUpdate();
        BTCtrLoop();
        BTCtrWaitLoopDelay();
    }

    return 0;
}

//
// Thread management
//
static
int start_controller_polling_thread(void) 
{
    int result;
    SceUID thid;

	//sceKernelDelayThread(10 * 1000 * 1000);

    g_sio_running = 1;

    // name, entry, initPriority, stackSize, PspThreadAttributes, SceKernelThreadOptParam
    thid = sceKernelCreateThread(MODULE_NAME "SioThread", controller_polling_thread, 0x11, 0x800, 0, 0);
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
int stop_controller_polling_thread(void) 
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
int start_controller_patching_thread(void)
{
    int result;
    SceUID thid;

    // name, entry, initPriority, stackSize, PspThreadAttributes, SceKernelThreadOptParam
    thid = sceKernelCreateThread(MODULE_NAME "MainThread", ControllerPatching_thread, 0x11, 0x800, 1, 0);
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
int stop_controller_patching_thread(void)
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

int _ResumeHandler(int unk, void *param)
{

    start_controller_polling_thread();

  return 0;
}

int _SuspendHandler(int unk, void *param)
{
    stop_controller_polling_thread();

    return 0;
}


// Called during module init
// TODO add suspend/resume handling to restart SIO so PSP doesn't hang
int module_start(SceSize args, void *argp)
{

    sceKernelRegisterSuspendHandler(0x1F, _SuspendHandler, 0);

    sceKernelRegisterResumeHandler(0x1F, _ResumeHandler, 0);

    int result = start_controller_patching_thread();
    int polling_result = start_controller_polling_thread();

    if(result < 0 || polling_result < 0) {
        return MODULE_ERROR;
    }

    return MODULE_OK;
}

// Called during module deinit
int module_stop(SceSize args, void *argp)
{
    int result = stop_controller_patching_thread();
    int polling_result = stop_controller_polling_thread();

    if(result < 0 || polling_result < 0) {
        return MODULE_ERROR;
    }

    return MODULE_OK;
}