#ifndef PSP_UART_H
#define PSP_UART_H

#include <psptypes.h>

#ifdef __cplusplus
extern "C" {
#endif

void pspUARTInit(int baud);
void pspUARTTerminate(void);
void pspUARTSetBaud(int baud);
int  pspUARTAvailable(void);
int  pspUARTRead(void);
void pspUARTWrite(int ch);
void pspUARTWriteBuffer(const char *data, int len);
void pspUARTPrint(const char *str);
void pspUARTWaitForData(unsigned int timeout);
void pspUARTResetRingBuffer(void);

#ifdef __cplusplus
}
#endif

#endif // PSP_UART_H