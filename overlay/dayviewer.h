/*
	DayViewerForGame v7
	Copyright (C) 2012, plum

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __DAYVIEWER_FOR_GAME_H__
#define __DAYVIEWER_FOR_GAME_H__

// header
#include <pspkernel.h>
#include <pspsysmem_kernel.h>
#include <pspsystimer.h>
#include <pspdisplay.h>
#include <psppower.h>
#include <pspctrl.h>
#include <pspinit.h>
#include <psprtc.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <kubridge.h>
#include "kernel.h"
#include "generated/image_assets.h"

// patch
#define MAKE_CALL(a, f) _sw(0x0C000000 | (((u32)(f) >> 2) & 0x03FFFFFF), a);

// define
#define A(color) (u8)(color >> 24 & 0xFF)
#define B(color) (u8)(color >> 16 & 0xFF)
#define G(color) (u8)(color >> 8 & 0xFF)
#define R(color) (u8)(color & 0xFF)
#define BGR(color) (color & 0x00FFFFFF)
#define BITMAP_IMAGE_SIZE 32

// enum
enum FontMode
{
	FONT_NORMAL = 0x0000,
	FONT_SHADOW = 0x000F,
	FONT_THICK = 0x00F0,
	FONT_RIGHT = 0x0F00,
	FONT_CENTER = 0xF000,
};

enum ClearMode
{
	CLEAR_CLOCK_STRING = 0x01,
	CLEAR_WINDOW = 0x02,
	CLEAR_VOLUME_BAR = 0x04,
	CLEAR_BOTTOM_DIALOG = 0x08,
	CLEAR_CENTER_CURSOR = 0x20,
};

enum HomeStatus
{
	DISABLE_HOME_SCREEN = 0,
	ENABLE_HOME_SCREEN = 1,
};

// struct
typedef struct
{
	u32 clear_func;
	u32 draw_func;
	u32 vol_addr;
	u32 clock_addr;
	u32 icon_addr;
	u32 blit_addr;
	u32 ver_addr;
	u32 vram_addr;
	u32 stat_addr;
} PatchAddress;

typedef struct
{
	int width;
	int height;
	int bufferwidth;
	int pixelformat;
	void *vram;
} DrawBuffer;

typedef struct
{
	const u16 *pixels;
	int width;
	int height;
} BitmapImage;

typedef struct
{
	int set_pos;
	int x_pos;
	int y_pos;
	u16 flag;
	int hide_battery;
	int hide_volume_bar;
	int params[13];
	char *format;
	char *string;
	char *week_txt[7];
	char *month_txt[12];
} Config;

typedef struct
{
	char txt[20];
	char conv[5];
	int number;
} ConvertTypes;

typedef struct SceModule2
{
   struct SceModule2*   next; //0, 0x00
   u16             attribute; //4, 0x04
   u8              version[2]; //6, 0x06
   char            modname[27]; //8, 0x08
   char            terminal; //35, 0x23
   u16             status; //36, 0x24 (Used in modulemgr for stage identification)
   u16             padding; //38, 0x26
   u32             unk_28; //40, 0x28
   SceUID          modid; //44, 0x2C
   SceUID          usermod_thid; //48, 0x30
   SceUID          memid; //52, 0x34
   SceUID          mpidtext; //56, 0x38
   SceUID          mpiddata; //60, 0x3C
   void *          ent_top; //64, 0x40
   u32             ent_size; //68, 0x44
   void *          stub_top; //72, 0x48
   u32             stub_size; //76, 0x4C
   int             (* module_start)(SceSize, void *); //80, 0x50
   int             (* module_stop)(SceSize, void *); //84, 0x54
   int             (* module_bootstart)(SceSize, void *); //88, 0x58
   int             (* module_reboot_before)(void *); //92, 0x5C
   int             (* module_reboot_phase)(SceSize, void *); //96, 0x60
   u32             entry_addr; //100, 0x64(seems to be used as a default address)
   u32             gp_value; //104, 0x68
   u32             text_addr; //108, 0x6C
   u32             text_size; //112, 0x70
   u32             data_size; //116, 0x74
   u32             bss_size; //120, 0x78
   u8              nsegment; //124, 0x7C
   u8              padding2[3]; //125, 0x7D
   u32             segmentaddr[4]; //128, 0x80
   u32             segmentsize[4]; //144, 0x90
   int             module_start_thread_priority; //160, 0xA0
   SceSize         module_start_thread_stacksize; //164, 0xA4
   SceUInt         module_start_thread_attr; //168, 0xA8
   int             module_stop_thread_priority; //172, 0xAC
   SceSize         module_stop_thread_stacksize; //176, 0xB0
   SceUInt         module_stop_thread_attr; //180, 0xB4
   int             module_reboot_before_thread_priority; //184, 0xB8
   SceSize         module_reboot_before_thread_stacksize; //188, 0xBC
   SceUInt         module_reboot_before_thread_attr; //192, 0xC0
} SceModule2;

// main.c
void ClearCaches(void);

// graphic.c
void DrawChar(int x, int y, u32 fg, u32 bg, int shadow, char cht);
int DrawString(int x, int y, u32 fg, u32 bg, const char *str);
void DrawImage(const BitmapImage *image, int x, int y);

#endif

