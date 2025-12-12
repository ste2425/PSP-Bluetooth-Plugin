#include "btCtr.h"
#include "sioDriver.h"
#include "systemctrl.h"
#include <string.h>
#include <stdio.h>

#define BTCTR_CONTROLLER_COUNT 1
#define LOG_PATH "ms0:/SEPLUGINS/btr_ctr_driver.log"

//
// Globals
//
static BTCtr liveControllers[BTCTR_CONTROLLER_COUNT];
static SceUID commandSemaId = -1;

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
int write(const char *filename, const char *text);

int	snprintf (char *__restrict, size_t, const char *__restrict, ...)
               _ATTRIBUTE ((__format__ (__printf__, 3, 4)));

void removeLog(const char *filename) {
    sceIoRemove(filename);
}

void BTCtrSetup() {
    removeLog(LOG_PATH);

    commandSemaId = sceKernelCreateSema("BTCTRSEMA", 0, 1, 1, NULL);

    pspUARTInit(57600);
}

void BTCtrTerminate() {
    sceKernelDeleteSema(commandSemaId);
    pspUARTTerminate();
}

void BTCtrUpdate() {
    for (uint8_t i = 0; i < BTCTR_CONTROLLER_COUNT; ++i) {
        loadControllerData(i);
    }
}

BTCtr BTCtrGetControllerState(uint8_t controllerIndex) {
    return liveControllers[controllerIndex];
}

uint8_t BTCtrNewConnectionsEnabled() {
    int responseBuffer[1] = {0};
    uint8_t responseBufferSize = 1;

    uint8_t response = sendCommand(
        COMMAND_NEWCONNECTIONSENABLED,
        nullptr, 0,
        RESPONSE_NEWCONNECONNECTIONSENABLED,
        responseBuffer, responseBufferSize
    ); 

    if (response == RESPONSE_NEWCONNECONNECTIONSENABLED) {
        return responseBuffer[0];
    } else {
        return 0;
    }
}

uint8_t BTCtrLoadControllerInfo(uint8_t controllerIndex, ControllerInfo *info) {
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
        info->connected = true;
        info->controllerModel = responseBuffer[0];
        info->batteryLevel = responseBuffer[1];
        
        return 1;
    } else if (response == RESPONSE_CONTROLLER_NOT_FOUND) {
        info->connected = false;
        info->batteryLevel = 0;
        info->controllerModel = CONTROLLER_TYPE_None;

        return 1;
    } else {
        return 0;
    }
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

    sceKernelWaitSema(commandSemaId, 1, NULL);

    pspUARTResetRingBuffer();

    // send command and any command arguments
    pspUARTWrite(command);
    if (commandArguments != nullptr && commandArgumentsSize != 0) {
        for (size_t i = 0; i < commandArgumentsSize; ++i) {
            pspUARTWrite(commandArguments[i]);
        }
    }

    sceKernelDelayThread(6000);

    int status = pspUARTRead();
    //TODO hadle -1

    // There was an error, or we dont expect any more data, just return the status repsonse
    if (status != successResponseCode ||
        responseBuffer == nullptr ||
        responseSize == 0
    ) {
        sceKernelSignalSema(commandSemaId, 1);

        return status;
    }

    int recievedDataCount = pspUARTAvailable();

    // not enough response data returned, error
    if (recievedDataCount < responseSize) {
        sceKernelSignalSema(commandSemaId, 1);
        return RESONSE_NOT_ENOUGH_DATA_RETURNED;
    }

    // Read recieved data from SIO buffer
    for (int i = 0; i < responseSize; ++i) {
        responseBuffer[i] = pspUARTRead();
    }

    sceKernelSignalSema(commandSemaId, 1);

    return status;
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

// Used for debugging. For some reason does not run when method execute from Overlay plugin.
int write(const char *filename, const char *text) {
    SceUID fd = sceIoOpen(filename, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_APPEND, 0777);
    if (fd < 0) {
        return fd; // error opening file
    }

    int result = sceIoWrite(fd, text, strlen(text));
    sceIoClose(fd);
    
    return result;
}
