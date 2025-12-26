#pragma once

#include <stdint.h>

#define BT_DPAD_UP    1
#define BT_DPAD_DOWN  2
#define BT_DPAD_RIGHT  4
#define BT_DPAD_LEFT  8

#define BIT(nr)                 (1UL << (nr))

#define BT_BUTTON_X BIT(0)
#define BT_BUTTON_CIRCLE BIT(1)
#define BT_BUTTON_SQUARE BIT(2)
#define BT_BUTTON_TRIANGLE BIT(3)
#define BT_BUTTON_L1 BIT(4)
#define BT_BUTTON_R1 BIT(5)
#define BT_BUTTON_L2 BIT(6)
#define BT_BUTTON_R2 BIT(7)
#define BT_BUTTON_L3 BIT(8)
#define BT_BUTTON_R3 BIT(9)

#define BT_MISC_BUTTON_SYSTEM BIT(0)
#define BT_MISC_BUTTON_CAPTURE BIT(3)
#define BT_MISC_BUTTON_START BIT(2)
#define BT_MISC_BUTTON_SELECT BIT(1)

#define RESPONSE_DATA_NOT_SENT 0xA
#define RESPONSE_CONTROLLER_NOT_FOUND 0xB
#define RESPONSE_CONTROLLERDATA_OK 0xC
#define RESPONSE_LED_OK 0xD
#define RESPONSE_VIBRATE_OK 0xE
#define RESPONSE_INFO_OK 0xF
#define RESPONSE_PING 0x10
#define RESPONSE_NEWCON_OK 0x11
#define RESPONSE_DISNEWCON_OK 0x12
#define RESPONSE_DISCONNECT_OK 0x13
#define RESPONSE_FORGETKEYS_OK 0x14
#define RESPONSE_PLAYERLED_OK 0x15
#define RESPONSE_NEWCONNECONNECTIONSENABLED 0x18
#define RESPONSE_TIMEOUT 0xFF
#define RESONSE_NOT_ENOUGH_DATA_RETURNED 0xFE

#define COMMAND_PING 0x02
#define COMMAND_LOADCONTROLLERDATA 0x03
#define COMMAND_ENABLENEWCONNECTIONS 0x04
#define COMMAND_DISABLENEWCONNECTIONS 0x05
#define COMMAND_DISCONNECTCONTROLLER 0x06
#define COMMAND_SETLEDCOLOUR 0x07
#define COMMAND_SETVIBRATION 0x08
#define COMMAND_GETCONTROLLERINFO 0x09
#define COMMAND_FORGETBLUETOOTHKEYS 0xA
#define COMMAND_SETPLAYERLED 0xB
#define COMMAND_NEWCONNECTIONSENABLED 0xD

#define CONTROLLER_TYPE_None  -1
#define CONTROLLER_TYPE_Unknown  0

#define CONTROLLER_TYPE_UnknownSteamController  1
#define CONTROLLER_TYPE_SteamController  2
#define CONTROLLER_TYPE_SteamControllerV2  3
#define CONTROLLER_TYPE_UnknownNonSteamController  30
#define CONTROLLER_TYPE_XBox360Controller  31
#define CONTROLLER_TYPE_XBoxOneController  32
#define CONTROLLER_TYPE_PS3Controller  33
#define CONTROLLER_TYPE_PS4Controller  34
#define CONTROLLER_TYPE_WiiController  35
#define CONTROLLER_TYPE_AppleController  36
#define CONTROLLER_TYPE_AndroidController  37
#define CONTROLLER_TYPE_SwitchProController  38
#define CONTROLLER_TYPE_SwitchJoyConLeft  39
#define CONTROLLER_TYPE_SwitchJoyConRight  40
#define CONTROLLER_TYPE_SwitchJoyConPair  41
#define CONTROLLER_TYPE_SwitchInputOnlyController  42
#define CONTROLLER_TYPE_MobileTouch  43
#define CONTROLLER_TYPE_PS5Controller 45

typedef struct {
    uint8_t analogRX;
    uint8_t analogRY;
    uint8_t analogLX;
    uint8_t analogLY;
    uint8_t dpad;
    uint16_t buttons;
    uint16_t miscButtons;
    uint8_t index;
    uint8_t connected;
} BTCtr;

typedef struct {
    uint8_t controllerModel;
    uint8_t batteryLevel;
    bool connected;
} ControllerInfo;

void BTCtrSetup();
void BTCtrTerminate();
void BTCtrUpdate();
BTCtr BTCtrGetControllerState(uint8_t controllerIndex);
uint8_t BTCtrPing();
uint8_t BTCtrEnableConnections();
uint8_t BTCtrDisableConnections();
uint8_t BTCtrNewConnectionsEnabled();
uint8_t BTCtrLoadControllerInfo(uint8_t controllerIndex);
ControllerInfo BTCtrGetControllerInfo(uint8_t controllerIndex);
void BTCTRTriggerNewConnections();
void BTCTRTriggerNoNewConnetions();
void BTCtrLoop();
void BTCtrWaitLoopDelay();
void BTCtrSetControllerInfoPolling(bool poll);