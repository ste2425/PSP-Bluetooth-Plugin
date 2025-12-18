#include "btCtr.h"
#include "sioDriver.h"
#include "systemctrl.h"
#include <string.h>
#include <stdio.h>

#include "util.h"
#include "scepaf.h"

#define BTCTR_CONTROLLER_COUNT 1

//
// Globals
//
static BTCtr liveControllers[BTCTR_CONTROLLER_COUNT];
static SceUID commandSemaId = -1;
static SceUID commandEventId = -1;
#define COMMAND_READY_EVENT  		0x02
static uint8_t inProgress = 0;
static int servicingNext = 0;
static int queue = 0;

void WaitForCommand() {
    unsigned int k1 = pspSdkSetK1(0);

    int resp = sceKernelWaitEventFlag(commandEventId, COMMAND_READY_EVENT, PSP_EVENT_WAITOR | PSP_EVENT_WAITCLEAR, nullptr, nullptr);

    pspSdkSetK1(k1);
}

void WaitForTicket(int ticket) {
    while(ticket != servicingNext) {
        WaitForCommand();
    }
}

void TriggerCommandDone() {       
    sceKernelSetEventFlag(commandEventId, COMMAND_READY_EVENT);
}

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

int	snprintf (char *__restrict, size_t, const char *__restrict, ...)
               _ATTRIBUTE ((__format__ (__printf__, 3, 4)));


void BTCtrSetup() {
	commandEventId = sceKernelCreateEventFlag("TESTTTT", 0, 0, 0);
    commandSemaId = sceKernelCreateSema("BTCTRSEMA", 0, 1, 1, NULL);

    pspUARTInit(38400);
}

void BTCtrTerminate() {
    sceKernelDeleteEventFlag(commandEventId);
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
    } else if (response == RESPONSE_CONTROLLER_NOT_FOUND) {
        info->connected = false;
        info->batteryLevel = 0;
        info->controllerModel = CONTROLLER_TYPE_None;
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

uint8_t _sendCommand(
    uint8_t command, 
    uint8_t* commandArguments, 
    uint8_t commandArgumentsSize, 
    uint8_t successResponseCode, 
    int* responseBuffer, 
    int responseSize
) {

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
    //sceKernelSignalSema(commandSemaId, 1);
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

    //sceKernelSignalSema(commandSemaId, 1);
    return status;
}

uint8_t sendCommand(
    uint8_t command, 
    uint8_t* commandArguments, 
    uint8_t commandArgumentsSize, 
    uint8_t successResponseCode, 
    int* responseBuffer, 
    int responseSize
) 
{
    auto ticket = queue + 1;
    queue++;

    if (inProgress) {
        char bufff[54];
        scePaf_sprintf(bufff, "Waiting for: %d", ticket);
        Util_writeLog(bufff);
        WaitForTicket(ticket);
    }

        inProgress = true;
    char buff[54];
    scePaf_sprintf(buff, "servicing: %d", servicingNext);
    Util_writeLog(buff);
        auto result = _sendCommand(
            command,
            commandArguments,
            commandArgumentsSize,
            successResponseCode,
            responseBuffer,
            responseSize
        );
        servicingNext++;
        inProgress = false;
        TriggerCommandDone();

    return result;
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

