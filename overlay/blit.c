/*
* This originated from PRO CFW project and has been extended by the evil that is AI
* Like most library code I dont really have a clue what is going on in here.
* 
* It works, i think
*
*/

/*
 * This file is part of PRO CFW.

 * PRO CFW is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * PRO CFW is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with PRO CFW. If not, see <http://www.gnu.org/licenses/ .
 */

/*
    PSP VSH 24bpp text bliter
*/
#include "blit.h"

#include <pspdisplay.h>

#include "scepaf.h"
#include "fonts.h"


blit_Gfx gfx = {
    .vram32 = NULL,
    .fg_color = 0x00ffffff,
    .bg_color = 0xff000000,
    .width = 0,
    .height = 0,
    .bufferwidth = 0,
    .pixelformat = 0
};


static u32 adjust_alpha(u32 col) {
    u32 c1, c2;
    u32 alpha = col >> 24;
    u8 mul;

    if (alpha == 0)    
        return col;
    if (alpha == 0xff) 
        return col;

    c1 = col & 0x00ff00ff;
    c2 = col & 0x0000ff00;
    mul = (u8)(255 - alpha);
    c1 = ((c1*mul) >> 8) & 0x00ff00ff;
    c2 = ((c2*mul) >> 8) & 0x0000ff00;
    return (alpha << 24) | c1 | c2;
}


blit_Gfx* blit_gfx_pointer(void) {
    return (blit_Gfx*)&gfx;
}


int blit_setupOld(void) {
    int unk;
    sceDisplayGetMode(&unk, &gfx.width, &gfx.height);
    sceDisplayGetFrameBuf((void*)&gfx.vram32, &gfx.bufferwidth, &gfx.pixelformat, PSP_DISPLAY_SETBUF_NEXTFRAME);
    if ((gfx.bufferwidth == 0) || (gfx.pixelformat != 3)) 
        return -1;

    gfx.fg_color = 0x00ffffff;
    gfx.bg_color = 0xff000000;
    return 0;
}

int blit_setup()
{
    int ret = sceDisplayGetFrameBuf((void*)&gfx.vram32, &gfx.bufferwidth, &gfx.pixelformat, PSP_DISPLAY_SETBUF_NEXTHSYNC);

    /* Ensure we got a valid pointer/stride */
    if (ret != 0 || gfx.vram32 == NULL || gfx.bufferwidth == 0)
        return -1;

    /* Set width/height if not already set.
       Most PSP screens are 480x272. If you support different
       resolutions change these constants accordingly or define
       SCREEN_W/SCREEN_H in your project. */
#ifndef SCREEN_W
#define SCREEN_W 480
#endif
#ifndef SCREEN_H
#define SCREEN_H 272
#endif

    /* If gfx.width/height are zero (not initialized), set them.
       If you already set them elsewhere, this will leave them alone. */
    if (gfx.width == 0)  gfx.width  = SCREEN_W;
    if (gfx.height == 0) gfx.height = SCREEN_H;

    return 0;
}

void blit_set_color(int fg_col,int bg_col) {
    gfx.fg_color = fg_col;
    gfx.bg_color = bg_col;
}


int blit_string(int sx, int sy, const char *msg) {
    int char_x = sx;
    u8 code;
    u32 fg_col, bg_col;
    u32 col, c1, c2;
    u32 alpha;
    
    font_Data *font = (font_Data*)font_data_pointer();
    
    fg_col = adjust_alpha(gfx.fg_color);
    bg_col = adjust_alpha(gfx.bg_color);
    
    if (!gfx.vram32 || gfx.bufferwidth == 0 || gfx.pixelformat != 3)
        return -1;
    
    for (int i = 0; msg[i]; i++) {
        if (msg[i] == '\n') {
            sy += font->height;
            char_x = sx;
            continue;
        }
        
        code = (u8)msg[i];
        if (code < 32 || code > 127) {
            char_x += font->width;
            continue;
        }
        
        // Get font row data from bitmap
        // Font data starts at ASCII 32, so subtract 32 from code
        // Each character is font->height bytes (8 for 8x8)
        int char_offset = (code - 32) * font->height;
        
        // Draw character with foreground and background colors
        for (int row = 0; row < font->height; ++row) {
            unsigned char bits = font->bitmap[char_offset + row];
            for (int col_bit = 0; col_bit < font->width; ++col_bit) {
                int px = char_x + col_bit;
                int py = sy + row;
                
                // Bounds check
                if (px < 0 || py < 0 || px >= gfx.width || py >= gfx.height)
                    continue;
                
                // Determine color: foreground if bit is set, background otherwise
                col = (bits & (1 << col_bit)) ? fg_col : bg_col;
                alpha = col >> 24;
                
                u32 *pixel = &gfx.vram32[py * gfx.bufferwidth + px];
                
                if (alpha == 0) {
                    (*pixel) = col;
                } else if (alpha != 0xff) {
                    c2 = (*pixel);
                    c1 = c2 & 0x00ff00ff;
                    c2 = c2 & 0x0000ff00;
                    c1 = ((c1 * alpha) >> 8) & 0x00ff00ff;
                    c2 = ((c2 * alpha) >> 8) & 0x0000ff00;
                    (*pixel) = (col & 0xffffff) + c1 + c2;
                } else {
                    (*pixel) = col;
                }
            }
        }
        
        char_x += font->width;
    }
    return char_x;
}

int blit_string_ctr(int sy,const char *msg) {
    font_Data *font = (font_Data*)font_data_pointer();
    return blit_string((gfx.width - scePaf_strlen(msg) * font->width) / 2, sy, msg);
}

int blit_string_windowed_ctr(int sy, int sx, int w, const char *msg) {
    font_Data *font = (font_Data*)font_data_pointer();

    int msgWidth = scePaf_strlen(msg) * font->width;
    int offset = (w - msgWidth) / 2;

    blit_rect_fill(sx, sy, offset, font->height); // Draw background

    blit_string(sx + offset, sy, msg); // Draw string

    blit_rect_fill(sx + offset + msgWidth, sy, offset, font->height); // Draw background

    return 0;
}

void blit_rect_fill(int sx, int sy, int w, int h) {
    int x, y;
    u32 col, c1, c2, alpha;
    u32 *pixel;
    
    col = adjust_alpha(gfx.bg_color);
    alpha = col >> 24;
    
    // set start position
    pixel = &gfx.vram32[sy * gfx.bufferwidth + sx];
    
    for (y = 0; y < h; y++) {
        for (x = 0; x < w; x++) {
        	if(alpha == 0)
        		(*pixel) = col;
        	else if (alpha != 0xff) {
        		c2 = (*pixel);
        		c1 = c2 & 0x00ff00ff;
        		c2 = c2 & 0x0000ff00;
        		c1 = ((c1 * alpha) >> 8) & 0x00ff00ff;
        		c2 = ((c2 * alpha) >> 8) & 0x0000ff00;
        		(*pixel) = (col & 0xffffff) + c1 + c2;
        	}
        	pixel++;
        }
        // go back to start position on the x-axis
        pixel -= w;
        // increase y position
        pixel += gfx.bufferwidth;
    }
}

// Returns size of string in pixels
int blit_get_string_width(char *msg) {
    font_Data *font = (font_Data*)font_data_pointer();
    return scePaf_strlen(msg) * font->width;
}