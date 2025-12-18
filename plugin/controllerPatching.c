/**
 * Controller Patching implementation based on the great work done here:
 * 
 * https://github.com/crozone/PSP-EmulatedControllerTest
 */

#include "controllerPatching.h"

static SceUInt g_button_state = 0;

uint32_t migrate_dpad(int buttons, uint8_t dpad)
{
    if (dpad & BT_DPAD_UP) buttons |= SCE_CTRL_UP;  
    if (dpad & BT_DPAD_DOWN) buttons |= SCE_CTRL_DOWN;  
    if (dpad & BT_DPAD_LEFT) buttons |= SCE_CTRL_LEFT;  
    if (dpad & BT_DPAD_RIGHT) buttons |= SCE_CTRL_RIGHT;  

    return buttons;
}

// TODO add extra buttons to the mapping (home etc)
// TODO make bit masks same as PSP so no need to map
uint32_t migrate_buttons(int buttons, uint8_t btButtons)
{
    if (btButtons & BT_BUTTON_X) buttons |= SCE_CTRL_CROSS; 
    if (btButtons & BT_BUTTON_CIRCLE) buttons |= SCE_CTRL_CIRCLE;  
    if (btButtons & BT_BUTTON_SQUARE) buttons |= SCE_CTRL_SQUARE;  
    if (btButtons & BT_BUTTON_TRIANGLE) buttons |= SCE_CTRL_TRIANGLE;  

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
    // TODO investigate migrating controller GYRO data
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

int ControllerPatching_thread(SceSize args, void *argp)
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