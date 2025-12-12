#include <pspkernel.h>
#include <pspdisplay.h>
#include <psptypes.h>
#include <psprtc.h>
#include <pspctrl.h>
#include <string.h>
#include <pspiofilemgr.h>

#include "kernel.h"

#include "blit.h"
#include "kubridge.h"
#include "menu.h"

/// Checks whether a result code indicates success.
#define R_SUCCEEDED(res) ((res) >= 0)
/// Checks whether a result code indicates failure.
#define R_FAILED(res)    ((res) < 0)

#define PRX_PATH "ms0:/SEPLUGINS/bt_ctr_driver.PRX"

#define SECOND	   1000000
#define PAUSE (2 * SECOND)

PSP_MODULE_INFO("bt_ctr_overlay", 1, 0, 1);
PSP_MAIN_THREAD_ATTR(1);

static int LoadModule(const char *path) {
    SceUID modID = -1;

    modID = kuKernelLoadModule(path, 0, nullptr);
    
    return modID;
}

static int file_exists(const char *path) {
    SceUID fd = sceIoOpen(path, PSP_O_RDONLY, 0);
    
    sceIoClose(fd);
    
    return fd;
}

int thid;
int controllerInfoThid;
int running;

// Thread that runs in background every 2 seconds to check status on connected controllers.
// TODO make it only run when overlay open, unless we want a low battery icon to appear?
// TODO maybe use VTimers rather than loop and sleep?
int controllerPolling_thread(SceSize args, void *argp) {
    MenuState *menuState = (MenuState*)menu_getPointer();

    while (running) {            
        BtCtrDriverLoadControllerInfo(0, &menuState->controllers[0]);
        BtCtrDriverLoadControllerInfo(1, &menuState->controllers[1]);  
        BtCtrDriverLoadControllerInfo(2, &menuState->controllers[2]);   
        BtCtrDriverLoadControllerInfo(3, &menuState->controllers[3]);    
        
        sceKernelDelayThread(PAUSE);
    }

    return sceKernelExitDeleteThread(0);
}

// main thread
int main_thread(SceSize args, void *argp){
    MenuState *menuState = (MenuState*)menu_getPointer();

    SceCtrlData pad;
    SceCtrlData prevPad = {0};

    sceCtrlSetSamplingCycle(0);
    sceCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);

    SceModule btModule;

    // Check if the bt_ctr_driver module is running and the prx exists wher we expect
    if (R_SUCCEEDED(kuKernelFindModuleByName("BTControllerModule", &btModule)))
        menuState->btModuleLoaded = 1;

    if (R_SUCCEEDED(file_exists(PRX_PATH)))
        menuState->btModuleFound = 1;
        
    if (menuState->btModuleFound && menuState->btModuleLoaded)
        LoadModule(PRX_PATH);   

    while (running){
        if (sceDisplayWaitVblankStartCB() < 0)
            break;

        blit_setup();

        // IF the menu is open steal the input
        if (menuState->menuOpen) {
            sceCtrlReadBufferPositive(&pad, 1);
        } else {
            sceCtrlPeekBufferPositive(&pad, 1);
        }

        // Toggle the menu
        if (pad.Buttons & PSP_CTRL_TRIANGLE && !(prevPad.Buttons & PSP_CTRL_TRIANGLE)) {
            // If we are opening the menu get the initial state of new connections.
            if (!menuState->menuOpen)
                menuState->newConnectionsEnabled = BtCtrDriverNewConnectionsEnabled();
            
            menu_toggle();
        }

        menuState->padState = pad;
        menuState->prevPadState = prevPad;

        menu_render();

        prevPad = pad;
    }

    return sceKernelExitDeleteThread(0);
}

int module_start(SceSize args, void *argp){
    running = 0;

    thid = sceKernelCreateThread("main", main_thread, 0x10, 4*1024, PSP_THREAD_ATTR_USER, NULL);
    //controllerInfoThid = sceKernelCreateThread("controller_polling", controllerPolling_thread, 0x10, 4*1024, PSP_THREAD_ATTR_USER, NULL);
    
    if (thid >= 0 ){//&& controllerInfoThid >= 0){
        running = 1;

        sceKernelStartThread(thid, args, argp);
        //sceKernelStartThread(controllerInfoThid, args, argp);
    }
    return 0;
}

int module_stop(SceSize args, void *argp){
    if (running){
        running = 0;
        SceUInt time = 200*1000;

        int infoRet = sceKernelWaitThreadEnd(controllerInfoThid, &time);
        if (infoRet < 0)
            sceKernelTerminateDeleteThread(controllerInfoThid);

        int ret = sceKernelWaitThreadEnd(thid, &time);
        if (ret < 0)
            sceKernelTerminateDeleteThread(thid);        
    }

    return 0;
}
