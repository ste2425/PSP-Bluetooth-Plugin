#ifndef _FONT_H
#define _FONT_H

#define FONT_WIDTH 8
#define FONT_HEIGHT 8

typedef struct _font_Data{
    unsigned char *bitmap;
    int width, height;
}font_Data;

font_Data* font_data_pointer(void);

int font_load();


#endif