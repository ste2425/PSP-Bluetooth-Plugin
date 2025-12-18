#include "util.h"
#include <pspkernel.h>
#include "scepaf.h"

#define LOG_PATH "ms0:/SEPLUGINS/btr_ctr_driver.log"

void Util_writeLog(const char *text) {
    SceUID fd = sceIoOpen(LOG_PATH, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_APPEND, 0777);
    if (fd < 0) {
        return; // error opening file
    }

    int result = sceIoWrite(fd, text, scePaf_strlen(text));
    sceIoClose(fd);
    
    return;
}

void Util_removeLog() {
    sceIoRemove(LOG_PATH);
}