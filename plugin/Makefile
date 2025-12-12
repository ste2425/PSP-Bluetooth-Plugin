TARGET = bt_ctr_driver

OBJS = main.o exports.o imports.o sioDriver.o btCtr.o

BUILD_PRX = 1
PRX_EXPORTS = exports.exp

# Use the kernel's small inbuilt libc
USE_KERNEL_LIBC = 1
# Use only kernel libraries
USE_KERNEL_LIBS = 1

INCDIR = ../libs/include includes
CFLAGS = -Os -G0 -Wall -fno-builtin-printf
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LIBS = -lpsphprm_driver

LDFLAGS = -nostartfiles

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build_prx.mak

all:
	psp-build-exports -s $(PRX_EXPORTS)
	mv bt_ctr_driver.S "../overlay/libs/"
	mv bt_ctr_driver.prx "../release/"
