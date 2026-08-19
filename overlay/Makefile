#PSPDEV = ./SDK
#PSPSDK = $(PSPDEV)/psp/sdk

TARGET = bt_ctr_overlay
OBJS = main.o graphic.o exports.o libs/bt_ctr_driver.o
IMAGE_HEADER = generated/image_assets.h
IMAGE_SOURCES = $(wildcard assets/*.png)

USE_KERNEL_LIBC = 1
USE_KERNEL_LIBS = 1

BUILD_PRX = 1
PRX_EXPORTS = exports.exp

INCDIR = 
CFLAGS = -O2 -G0 -Wall -fno-builtin-printf
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LIBDIR = 
LIBS = -lpspkernel -lc -lpspuser -lpsprtc -lpsppower -lpspkubridge

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build_prx.mak

all:
	mv bt_ctr_overlay.prx "../release/"

main.o graphic.o: $(IMAGE_HEADER)

$(IMAGE_HEADER): $(IMAGE_SOURCES) tools/png_to_5551.py
	python3 tools/png_to_5551.py $@ $(IMAGE_SOURCES)

clean-images:
	rm -f $(IMAGE_HEADER)
