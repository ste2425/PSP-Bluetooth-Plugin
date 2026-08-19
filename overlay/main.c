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

// header
#include "dayviewer.h"

// info
PSP_MODULE_INFO("DayViewerForGame", 0x1007, 0, 6);


/// Checks whether a result code indicates success.
#define R_SUCCEEDED(res) ((res) >= 0)
/// Checks whether a result code indicates failure.
#define R_FAILED(res)    ((res) < 0)


#define PRX_PATH "ms0:/SEPLUGINS/bt_ctr_driver.PRX"

// global
u32 impose_text_addr;
u32 ver_addr;
u32 stat_addr;
int time_count;
void *vram_top;
pspTime g_time;
SceSysTimerId timer;
int enabled = 0;
int test = 10;

// extern
extern u16 gray_bg;
extern DrawBuffer draw_buf;

// function_ptr
void (*ImposeDrawVolumebar)(void);
void (*ImposeClearDisplay)(int clear_mode_flag);

// struct
PatchAddress patch_addr[] =
{
	{ 0x658, 0x6350, 0x4AC8, 0x3CF4, 0x694C, 0x6980, 0xA060, 432, 392 }, //FW6.20
	{ 0x658, 0x64E8, 0x4B74, 0x3DA0, 0x6AE4, 0x6B18, 0xA230, 440, 400 }, //FW6.3X
	{ 0x5D0, 0x6470, 0x4AEC, 0x3D18, 0x6A6C, 0x6AA0, 0xA1D0, 440, 400 }, //FW6.60
};

PatchAddress patch_addr_go[] =
{
	{ 0x6DC, 0x7D70, 0x5DB0, 0x4D40, 0x8464, 0x8498, 0xC1D0, 444, 404 }, //FW6.20
	{ 0x750, 0x7F98, 0x5EEC, 0x4E68, 0x868C, 0x86C0, 0xC440, 452, 412 }, //FW6.3X
	{ 0x6C8, 0x7E60, 0x5E64, 0x4DE0, 0x8554, 0x8588, 0xC2D0, 452, 412 }, //FW6.60
};

int LoadModule(const char *path) {
    SceUID modID = -1;

    modID = kuKernelLoadModule(path, 0, NULL);
    
    return modID;
}

// functions
void ClearCaches(void)
{
	sceKernelDcacheWritebackAll();
	sceKernelIcacheClearAll();
}

int GetPatchAddr(void)
{
	int res, devkit, version;

	// �o�[�W�������擾���� - trans. Obtain version
	devkit = sceKernelDevkitVersion();
	version = (((devkit >> 24) & 0xF) << 8) | (((devkit >> 16) & 0xF) << 4) | ((devkit >> 8) & 0xF);

	switch(version)
	{
		case 0x620:
			res = 0;
			break;

		case 0x635:
		case 0x637:
		case 0x638:
		case 0x639:
			res = 1;
			break;

		case 0x660:
		case 0x661:
			res = 2;
			break;

		default:
			res = -1;
			break;
	}

	return res;
}

void InitGraphic(void)
{
	u16 *vram16;

	draw_buf.width = 480;
	draw_buf.height = 272;
	draw_buf.bufferwidth = 512;
	draw_buf.pixelformat = PSP_DISPLAY_PIXEL_FORMAT_5551;
	draw_buf.vram = vram_top;

	if(gray_bg == 0)
	{
		vram16 = (u16 *)vram_top;
		gray_bg = vram16[0];
	}

	return;
}

// HOME��ʂȂ�1��Ԃ��A����ȊO��0��Ԃ� - Return 1 if screen, otherwise return 0
int ImposeGetHomeStatus(void)
{
	return _lw(_lw(impose_text_addr + ver_addr) + stat_addr);
}

int TickHandler(void)
{
	time_count++;

	if(time_count > 59)
	{
		// �������\������
		if(ImposeGetHomeStatus() == ENABLE_HOME_SCREEN)
		{
			for(int i = 0; i < 4; i++) {
				ControllerInfo ctrInf =	btCtrLoadControllerInfo(i);

				if (ctrInf.connected != 1) {
					continue;
				}

					char info_text[64];
					int baseY = 15 + i * 36;

					/* Print raw numeric values of ControllerInfo fields */
					uint8_t percent = ctrInf.batteryLevel / 255 * 100;
					snprintf(info_text, sizeof(info_text), "P%d %u%%",
								 i + 1,
								 ctrInf.batteryLevel);
					DrawString(5, baseY, 0xC0FFFFFF, 0xFF000000, info_text);
			}
			//if (enabled)
			//	test = BTCtrTEST();

		}

		time_count = 0;
	}

	return -1;
}

int PatchImposeDriver(void)
{
	SceUID thid;
	SceModule2 *mod;
	u32 text_addr, model, patch_fw, clear_func, draw_func, vol_addr, clock_addr, icon_addr, blit_addr, vram_addr;

	// ���W���[��������
	mod = sceKernelFindModuleByName("sceImpose_Driver");

	if(!mod)
		return -1;

	// text_addr���擾����
	text_addr = mod->text_addr;
	impose_text_addr = mod->text_addr;

	// index�̎擾
	patch_fw = GetPatchAddr();

	if(patch_fw < 0)
		return -1;

	// PSP�̃��f�����擾����
	model = sceKernelGetModel();

	if(model != 4)
	{
		clear_func = patch_addr[patch_fw].clear_func;
		draw_func = patch_addr[patch_fw].draw_func;
		vol_addr = patch_addr[patch_fw].vol_addr;
		clock_addr = patch_addr[patch_fw].clock_addr;
		icon_addr = patch_addr[patch_fw].icon_addr;
		blit_addr = patch_addr[patch_fw].blit_addr;
		ver_addr = patch_addr[patch_fw].ver_addr;
		vram_addr = patch_addr[patch_fw].vram_addr;
		stat_addr = patch_addr[patch_fw].stat_addr;
	}
	else
	{
		clear_func = patch_addr_go[patch_fw].clear_func;
		draw_func = patch_addr_go[patch_fw].draw_func;
		vol_addr = patch_addr_go[patch_fw].vol_addr;
		clock_addr = patch_addr_go[patch_fw].clock_addr;
		icon_addr = patch_addr_go[patch_fw].icon_addr;
		blit_addr = patch_addr_go[patch_fw].blit_addr;
		ver_addr = patch_addr_go[patch_fw].ver_addr;
		vram_addr = patch_addr_go[patch_fw].vram_addr;
		stat_addr = patch_addr_go[patch_fw].stat_addr;
	}

	// patch ImposeClearDisplay(int clear_mode_flag)
	// andi $v1, $a0, 0x1 -> lui $v1, 0x0
	//_sw(0x3C030000, text_addr + clear_func + 4);

	// patch ImposeBlitPrintf(pspTime *time, int x, int y, int arg3)
	//_sw(0, text_addr + blit_addr);

	// HOME��ʂ�VRAM�̃A�h���X���擾����
	vram_top = (void *)_lw(_lw(text_addr + ver_addr) + vram_addr);

	// ������
	InitGraphic();

	// �L���b�V���N���A
	ClearCaches();
	return 0;
}

int module_start(SceSize args, void *argp)
{
	int i;

	if(sceKernelInitKeyConfig() == PSP_INIT_KEYCONFIG_GAME)
	{

    	SceModule btModule;
    	// Check if the bt_ctr_driver module is running and the prx exists wher we expect
    	if (R_SUCCEEDED(kuKernelFindModuleByName("BTControllerModule", &btModule)))
        	enabled = 1;

		if (enabled == 1) {
			LoadModule(PRX_PATH);
			
			btCtrSetControllerInfoPolling(true);
		}
		
			// ������
		gray_bg = 0;
		time_count = 59;
		memset(&g_time, 0, sizeof(pspTime));
		memset(&draw_buf, 0, sizeof(DrawBuffer));

		// �p�b�`
		if(PatchImposeDriver() < 0)
		{
			return -1;
		}

		// �^�C�}�[���Z�b�g����
		timer = sceSTimerAlloc();
		sceSTimerStartCount(timer);
		sceSTimerSetHandler(timer, 799999, TickHandler, 0);
	}

	return 0;
}

int module_stop(SceSize args, void *argp)
{
	sceSTimerStopCount(timer);
	sceSTimerFree(timer);
	return 0;
}

