.set noreorder

#include "pspimport.s"

IMPORT_START "sceSuspendForKernel",0x40010000 
IMPORT_FUNC    "sceSuspendForKernel",0x91a77137,sceKernelRegisterSuspendHandler 
IMPORT_FUNC    "sceSuspendForKernel",0xb43d1a8c,sceKernelRegisterResumeHandler
 
IMPORT_START "sceCtrl_driver",0x00010000
IMPORT_FUNC	"sceCtrl_driver",0x3A622550,sceCtrlPeekBufferPositive
IMPORT_FUNC	"sceCtrl_driver",0x5130DAE3,sceCtrlSetButtonEmulation
IMPORT_FUNC	"sceCtrl_driver",0xDB76878D,sceCtrlSetAnalogEmulation
IMPORT_FUNC	"sceCtrl_driver",0xF6E94EA3,sceCtrlSetSamplingMode
IMPORT_FUNC	"sceCtrl_driver",0xE467BEC8,sceCtrl_driver_E467BEC8
IMPORT_FUNC	"sceCtrl_driver",0x6C86AF22,sceCtrl_driver_6C86AF22

IMPORT_START "ThreadManForUser",0x40010000
IMPORT_FUNC   "ThreadManForUser",0xB7D098C6,sceKernelCreateMutex
IMPORT_FUNC   "ThreadManForUser",0xF8170FBE,sceKernelDeleteMutex
IMPORT_FUNC  "ThreadManForUser",0xB011B11F,sceKernelLockMutex
IMPORT_FUNC  "ThreadManForUser",0x6B30100F,sceKernelUnlockMutex
