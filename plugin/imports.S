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

IMPORT_START "scePaf",0x00090011
IMPORT_FUNC  "scePaf",0xAF067FA2,scePaf_wcslen
IMPORT_FUNC  "scePaf",0xA138A376,scePaf_sprintf
IMPORT_FUNC  "scePaf",0x7B7133D5,scePaf_snprintf
IMPORT_FUNC  "scePaf",0xB4652CFE,scePaf_memcpy
IMPORT_FUNC  "scePaf",0xB05D9677,scePaf_memcmp
IMPORT_FUNC  "scePaf",0xD9E2D6E1,scePaf_memset
IMPORT_FUNC  "scePaf",0xD7DCB972,scePaf_strlen
IMPORT_FUNC  "scePaf",0x726DFBA9,scePaf_strcpy
IMPORT_FUNC  "scePaf",0x706ABBFF,scePaf_strncpy
IMPORT_FUNC  "scePaf",0x4CF09BA2,scePaf_strcmp
IMPORT_FUNC  "scePaf",0xE0B32AE8,scePaf_strncmp
IMPORT_FUNC  "scePaf",0xC3B2D310,scePaf_strchr
IMPORT_FUNC  "scePaf",0x22420CC7,scePaf_strrchr
IMPORT_FUNC  "scePaf",0x4CE9C8D7,scePaf_strpbrk
IMPORT_FUNC  "scePaf",0xA4B8A4E3,scePaf_strtoul
