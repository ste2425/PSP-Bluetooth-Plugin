#include <inttypes.h>
#include <stdbool.h>
#include "btCtr.h"

void btCtrEnableNewConnections(uint8_t enable) {
    if (enable) {
        BTCTRTriggerNewConnections();
    } else {
        BTCTRTriggerNoNewConnetions();
    }
}

uint8_t btCtrNewConnectionsEnabled() {
    return BTCtrNewConnectionsEnabled();
}

ControllerInfo btCtrLoadControllerInfo(uint8_t controllerIndex) {
    return BTCtrGetControllerInfo(controllerIndex);
}

void btCtrDisconnectController(uint8_t controllerIndex) {
    BTCtrDisconnectController(controllerIndex);
}

void btCtrSetControllerInfoPolling(bool poll) {
    BTCtrSetControllerInfoPolling(poll);
}

bool btCtrConnected() {
    return BTCtrBoardConnected();
}
