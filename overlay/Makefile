TARGET = bt_ctr_overlay

BUILD_PRX = 1

OBJS = main.o exports.o blit.o libs/bt_ctr_driver.o menu.o fonts.o stubkk.o

INCDIR   = includes
CFLAGS = -Os -G0 -Wall
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)
LIBDIR =

LIBS = -lpspkubridge -lpspctrl

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build_prx.mak

all:
	mv bt_ctr_overlay.prx "../release/"