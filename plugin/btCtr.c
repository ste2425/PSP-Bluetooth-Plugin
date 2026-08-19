#include "btCtr.h"
#include "sioDriver.h"
#include "systemctrl.h"
#include <string.h>
#include <stdio.h>

#include "util.h"
#include "scepaf.h"

#define BTCTR_CONTROLLER_COUNT 4

//
// Globals
//
static BTCtr liveControllers[BTCTR_CONTROLLER_COUNT];
static ControllerInfo liveControllerState[BTCTR_CONTROLLER_COUNT];
bool enableNewConnectionsPending = false;
bool disableNewConnectionsPending = false;
bool connectionsCurrentlyEnabled = false;
bool getConnectionsState = false;
bool pollControllerInfo = false;
bool boardConnected = false;

//
// Hoists
//
uint8_t sendCommand(
    uint8_t command, 
    uint8_t* commandArguments, 
    uint8_t commandArgumentsSize, 
    uint8_t successResponseCode, 
    int* responseBuffer, 
    int responseSize
);
void loadControllerData(uint8_t controllerIndex);

static void setDefaultLiveControllerState(void) {
    for (uint8_t i = 0; i < BTCTR_CONTROLLER_COUNT; ++i) {
        liveControllerState[i].controllerModel = (uint8_t)CONTROLLER_TYPE_None;
        liveControllerState[i].batteryLevel = 12;
        liveControllerState[i].connected = 0;
    }
}

int	snprintf (char *__restrict, size_t, const char *__restrict, ...)
               _ATTRIBUTE ((__format__ (__printf__, 3, 4)));

void BTCtrSetControllerInfoPolling(bool poll) {
    pollControllerInfo = poll;
}

void BTCtrSetup() {
    setDefaultLiveControllerState();
    pspUARTInit(38400);
}

void BTCtrTerminate() {
    pspUARTTerminate();
}

void BTCtrUpdate() {
    for (uint8_t i = 0; i < BTCTR_CONTROLLER_COUNT; ++i) {
        loadControllerData(i);
    }
}

bool BTCtrBoardConnected() {
    return boardConnected;
}

void BTCTRTriggerNewConnections() {
    enableNewConnectionsPending = true;
    disableNewConnectionsPending = false;
}
void BTCTRTriggerNoNewConnetions() {
    enableNewConnectionsPending = false;
    disableNewConnectionsPending = true;
}

BTCtr BTCtrGetControllerState(uint8_t controllerIndex) {
    return liveControllers[controllerIndex];
}

ControllerInfo BTCtrGetControllerInfo(uint8_t controllerIndex) {
    return liveControllerState[controllerIndex];
}

uint8_t BTCtrNewConnectionsEnabled() {
    return connectionsCurrentlyEnabled;
}

void BTCtrNewConnectionsEnabledInternal() {
    int responseBuffer[1] = {0};
    uint8_t responseBufferSize = 1;

    uint8_t response = sendCommand(
        COMMAND_NEWCONNECTIONSENABLED,
        nullptr, 0,
        RESPONSE_NEWCONNECONNECTIONSENABLED,
        responseBuffer, responseBufferSize
    ); 

    connectionsCurrentlyEnabled = response == RESPONSE_NEWCONNECONNECTIONSENABLED;
}

uint8_t BTCtrLoadControllerInfo(uint8_t controllerIndex) {
    int responseBuffer[2] = {0};
    uint8_t responseBufferSize = 2;
    uint8_t commandArgs[1] = {controllerIndex};

    uint8_t response = sendCommand(
        COMMAND_GETCONTROLLERINFO,
        commandArgs, 1,
        RESPONSE_INFO_OK,
        responseBuffer, responseBufferSize
    );

    if (response == RESPONSE_INFO_OK) {
        liveControllerState[controllerIndex].connected = 1;
        liveControllerState[controllerIndex].controllerModel = responseBuffer[0];
        liveControllerState[controllerIndex].batteryLevel = responseBuffer[1];
    } else if (response == RESPONSE_CONTROLLER_NOT_FOUND) {
        liveControllerState[controllerIndex].connected = 0;
        liveControllerState[controllerIndex].batteryLevel = 0;
        liveControllerState[controllerIndex].controllerModel = CONTROLLER_TYPE_None;
    }

    return response;
}

uint8_t BTCtrPing() {
    uint8_t response = sendCommand(
        COMMAND_PING,
        nullptr, 0,
        RESPONSE_PING,
        nullptr, 0
    ); 

    return response == RESPONSE_PING;
}

void loadControllerData(uint8_t controllerIndex) {    
    int responseBuffer[9] = {0};
    uint8_t responseBufferSize = 9;
    uint8_t commandArgs[1] = {controllerIndex};

    uint8_t response = sendCommand(
        COMMAND_LOADCONTROLLERDATA,
        commandArgs, 1,
        RESPONSE_CONTROLLERDATA_OK,
        responseBuffer, responseBufferSize
    );

    if (response == RESPONSE_CONTROLLERDATA_OK) {
        liveControllers[controllerIndex].connected = true;
        liveControllers[controllerIndex].index = controllerIndex;
        liveControllers[controllerIndex].analogRX = responseBuffer[0];
        liveControllers[controllerIndex].analogRY = responseBuffer[1];
        liveControllers[controllerIndex].analogLX = responseBuffer[2];
        liveControllers[controllerIndex].analogLY = responseBuffer[3];
        liveControllers[controllerIndex].dpad = responseBuffer[4];
        liveControllers[controllerIndex].buttons = responseBuffer[5] << 8;
        liveControllers[controllerIndex].buttons |= responseBuffer[6];
        liveControllers[controllerIndex].miscButtons = responseBuffer[7] << 8;
        liveControllers[controllerIndex].miscButtons |= responseBuffer[8];

        return;
    }

    liveControllers[controllerIndex].connected = false;
}   

uint8_t sendCommand(
    uint8_t command, 
    uint8_t* commandArguments, 
    uint8_t commandArgumentsSize, 
    uint8_t successResponseCode, 
    int* responseBuffer, 
    int responseSize
) {
   // boardConnected = false;

    //sceKernelWaitSema(commandSemaId, 1, NULL);
    pspUARTResetRingBuffer();

    // send command and any command arguments
    pspUARTWrite(command);
    if (commandArguments != nullptr && commandArgumentsSize != 0) {
        for (size_t i = 0; i < commandArgumentsSize; ++i) {
            pspUARTWrite(commandArguments[i]);
        }
    }

    sceKernelDelayThread(6000);
    /*
        TODO use pspUARTWaitForData to be more efficient.
        However tried, first usage works, subsiquent timeout with no data. Need to investigate.
    */

    int status = pspUARTRead();
    //TODO hadle -1

    // There was an error, or we dont expect any more data, just return the status repsonse
    if (status != successResponseCode ||
        responseBuffer == nullptr ||
        responseSize == 0
    ) {
        // TODO - maybe handle unexpected data, some random serial device connected?
        boardConnected = status != -1;
        //sceKernelSignalSema(commandSemaId, 1);

        if (!boardConnected)
            setDefaultLiveControllerState();
            
        return status;
    }

    int recievedDataCount = pspUARTAvailable();

    // not enough response data returned, error
    if (recievedDataCount < responseSize) {
        //sceKernelSignalSema(commandSemaId, 1);
        return RESONSE_NOT_ENOUGH_DATA_RETURNED;
    }

    // Read recieved data from SIO buffer
    for (int i = 0; i < responseSize; ++i) {
        responseBuffer[i] = pspUARTRead();
    }

    boardConnected = true;

    //sceKernelSignalSema(commandSemaId, 1);
    return status;
}

int BTCtrTEST() {
    return 98;
}


uint8_t BTCtrDisconnectController(uint8_t controllerIndex) {
    uint8_t commandArgs[1] = {controllerIndex};

    uint8_t response = sendCommand(
        COMMAND_DISCONNECTCONTROLLER,
        commandArgs, 1,
        RESPONSE_DISCONNECT_OK,
        nullptr, 0
    );

    return response;
}

uint8_t BTCtrEnableConnections() {
    uint8_t response = sendCommand(
        COMMAND_ENABLENEWCONNECTIONS,
        nullptr, 0,
        RESPONSE_NEWCON_OK,
        nullptr, 0
    );

    return response;
}

uint8_t BTCtrDisableConnections() {    
    uint8_t response = sendCommand(
        COMMAND_DISABLENEWCONNECTIONS,
        nullptr, 0,
        RESPONSE_DISNEWCON_OK,
        nullptr, 0
    );

    return response;
}

void BTCtrLoop() 
{
    // update controller state
    BTCtrUpdate();

    if (pollControllerInfo) {
        // update controller info state
        for (uint8_t i = 0; i < BTCTR_CONTROLLER_COUNT; ++i) {
            BTCtrLoadControllerInfo(i);
        }
    }

    if (getConnectionsState) {
        BTCtrNewConnectionsEnabledInternal();
        getConnectionsState = false;
    }

    if (enableNewConnectionsPending) {
        auto res = BTCtrEnableConnections();

        if (res == RESPONSE_NEWCON_OK) {
            connectionsCurrentlyEnabled = true;
            enableNewConnectionsPending = false;
        }
    }

    if (disableNewConnectionsPending) {
        auto res = BTCtrDisableConnections();

        if (res == RESPONSE_DISNEWCON_OK) {
            connectionsCurrentlyEnabled = false;
            disableNewConnectionsPending = false;
        }
    }
}

void BTCtrWaitLoopDelay() {
    sceKernelDelayThread(10000); // 10 ms
}