#include "menu.h"
#include "blit.h"

#include "fonts.h"
#include "scepaf.h"
#include "kernel.h"
#include "images.h"
#include "bt.h"

typedef struct {
    const unsigned char *imageData;
    int width;
    int height;
} BatteryImageInfo;

MenuState menuState = {
    .menuOpen = 0,
    .btModuleFound = 0,
    .btModuleLoaded = 0,
    .boardConnected = 0,
    .newConnectionsEnabled = 0,
    .activeController = 0,
    .focussedController = 0,
    .padState = {0},
    .prevPadState = {0},
    .controllers = {
        {0}
    }
};

int errorCount = 0;
int tryCount = 0;

MenuState* menu_getPointer(void) {
    return (MenuState*)&menuState;
}

void menuAddError() {
    errorCount++;
}

void menuAddTry() {
    tryCount++;
}

void menuSetError(int count) {
    errorCount = count;
}

int menu_draw(void);

void menu_show() {
    menuState.menuOpen = 1;
}

void menu_hide() {
    menuState.menuOpen = 0;
}

void menu_toggle() {
    if (menuState.menuOpen) {
        menu_hide();
    } else {
        menu_show();
    }
}

void menu_render() {
    if (menuState.menuOpen) {
        menu_draw();
    }
}

BatteryImageInfo getBatteryImage(int batteryPercent) {
    BatteryImageInfo info;
    
    if (batteryPercent < 10) {
        // Empty: 0-9%
        info.imageData = BatEmptyImage;
        info.width = BatEmptyImage_W;
        info.height = BatEmptyImage_H;
    } else if (batteryPercent < 35) {
        // Quarter: 10-34%
        info.imageData = BatQuarterImage;
        info.width = BatQuarterImage_W;
        info.height = BatQuarterImage_H;
    } else if (batteryPercent < 60) {
        // Half: 35-59%
        info.imageData = BatHalfImage;
        info.width = BatHalfImage_W;
        info.height = BatHalfImage_H;
    } else if (batteryPercent < 90) {
        // Three Quarter: 60-89%
        info.imageData = BatThreeQuarterImage;
        info.width = BatThreeQuarterImage_W;
        info.height = BatThreeQuarterImage_H;
    } else {
        // Full: 90-100%
        info.imageData = BatFullImage;
        info.width = BatFullImage_W;
        info.height = BatFullImage_H;
    }
    
    return info;
}

void drawWarningMesage(const char *line1, const char *line2) {
    blit_Gfx *gfx = blit_gfx_pointer();
    font_Data *font = font_data_pointer();

    int window_width = 300;
    int window_height = gfx->height - 40;
    int menu_start_y = (gfx->height - window_height) / 2;
    int menu_start_x = (gfx->width - window_width) / 2;

    int messageOneY = gfx->height / 2 - font->height;
    int messageTwoY = gfx->height / 2;

    // top, left, right border
    blit_set_color(0xffffff, RGBT(80, 58, 147, 0));
    blit_rect_fill(menu_start_x, menu_start_y - 3, window_width, 2); // top
    blit_rect_fill(menu_start_x - 3, menu_start_y, 2, window_height); // left
    blit_rect_fill(menu_start_x + window_width + 2, menu_start_y, 2, window_height); // right

    // top backgrund half
    blit_set_color(0xffffff, RGBT(80, 58, 147, 128));
    int topBackgroundHeight = messageOneY - menu_start_y;
    blit_rect_fill(menu_start_x, menu_start_y, window_width, topBackgroundHeight);

    // middle text
    blit_set_color(0xffffff, RGBT(80, 58, 147, 0));
    blit_string_windowed_ctr(menu_start_x, messageOneY, window_width, line1);
    blit_string_windowed_ctr(menu_start_x, messageTwoY, window_width, line2);

    // bottom background half
    blit_set_color(0xffffff, RGBT(80, 58, 147, 128));
    blit_rect_fill(menu_start_x, messageTwoY + font->height, window_width, window_height - topBackgroundHeight - 16);

    // bottom border
    blit_set_color(0xffffff, RGBT(80, 58, 147, 0));
    blit_rect_fill(menu_start_x, menu_start_y + window_height + 2, window_width, 2);
}

const unsigned char* getControllerImage(int model) {
    switch (model) {
        case CONTROLLER_TYPE_PS3Controller:
        case CONTROLLER_TYPE_PS4Controller:
        case CONTROLLER_TYPE_PS5Controller:
            return PSControllerImage;
        case CONTROLLER_TYPE_SwitchProController:
        case CONTROLLER_TYPE_WiiController:
        case CONTROLLER_TYPE_SwitchJoyConLeft:
        case CONTROLLER_TYPE_SwitchJoyConRight:
        case CONTROLLER_TYPE_SwitchJoyConPair:
        case CONTROLLER_TYPE_SwitchInputOnlyController:
            return NintendoControllerImage;
        case CONTROLLER_TYPE_XBoxOneController:
        case CONTROLLER_TYPE_XBox360Controller:
            return XboxControllerImage;
        default:
            return GenericControllerImage;
    }
}

const char* getControllerModelName(int model) {
    switch (model) {
        case CONTROLLER_TYPE_PS3Controller:
            return "DS 3";
        case CONTROLLER_TYPE_PS4Controller:
            return "DS 4";
        case CONTROLLER_TYPE_PS5Controller:
            return "DualSense";
        case CONTROLLER_TYPE_SwitchProController:
            return "Switch Pro";
        case CONTROLLER_TYPE_XBoxOneController:
            return "Xbox One";
        default:
            return "Other";
    }
}

void drawConnectedControllers() {
    blit_Gfx *gfx = blit_gfx_pointer();
    font_Data *font = font_data_pointer();

    int window_width = 300;
    int window_height = gfx->height - 40;
    int menu_start_y = (gfx->height - window_height) / 2;
    int menu_start_x = (gfx->width - window_width) / 2;
    int controllerSectionY = (window_height - font->height - 10) / 4;
    int controllerSectionWidth = window_width / 3;

   // blit_image(triangle_image, 5, 5, 10, 10);

    // new connections text with padding
    blit_set_color(0xffffff, RGBT(80, 58, 147, 0));
    blit_rect_fill(menu_start_x, menu_start_y, window_width, 5);

    char buffer[64];
    scePaf_snprintf(buffer, sizeof(buffer), "E %d, T: %d", errorCount, tryCount);

    //if (menuState.newConnectionsEnabled) {
        blit_string_windowed_ctr(menu_start_x, menu_start_y + 5, window_width, buffer);
    //} else {
    //    blit_string_windowed_ctr(menu_start_x, menu_start_y + 5, window_width, "Connections Disabled");
    //}
    blit_rect_fill(menu_start_x, menu_start_y + font->height + 5, window_width, 5);

    // top, left, right border
    blit_set_color(0xffffff, RGBT(80, 58, 147, 0));
   // blit_rect_fill(menu_start_x, menu_start_y - 3, window_width, 2); // top
    blit_rect_fill(menu_start_x - 3, menu_start_y, 2, window_height); // left
    blit_rect_fill(menu_start_x + window_width + 2, menu_start_y, 2, window_height); // right

    // Render controller rows
    for (int i = 0; i < 4; i++) {
        ControllerInfo controller = BtCtrDriverLoadControllerInfo(i);
        int offset = menu_start_y + font->height + 10 + (controllerSectionY * i);
        int textY = controllerSectionY / 2 - font->height;

        // Top background half - to make text center of pannel
        blit_set_color(0xffffff, RGBT(80, 58, 147, 128));
       // blit_rect_fill(menu_start_x, offset, window_width, textY);

        // Render controller status
        blit_set_color(0xffffff, RGBT(80, 58, 147, 128));
        if (controller.connected) {
            char buffer[64];
            // TODO extend controler types into a lookup, maybe imges not text?
            const char *modelName = getControllerModelName(controller.controllerModel);
            const unsigned char *imageData = getControllerImage(controller.controllerModel);

            //column one
            // controler image with controller name below
            //blit_image(imageData, menu_start_x + 40, offset, 30, 30);
            blit_image_windowed(imageData, 30, 30, menu_start_x, offset, controllerSectionWidth);

            blit_string_windowed_ctr(menu_start_x + controllerSectionWidth, offset, controllerSectionWidth, modelName);


            int batteryPercent = (controller.batteryLevel * 100 + 127) / 255;
            char batBuff[64];
            scePaf_snprintf(batBuff, sizeof(batBuff), "Battery: %d%", batteryPercent);
            blit_string_windowed_ctr(menu_start_x + controllerSectionWidth, offset + font->height + 5, controllerSectionWidth, batBuff);

            // column two
            
            /* batteryLevel is 0-255; convert to 0-100% for display.*/
           // BatteryImageInfo batteryLevelImage = getBatteryImage(batteryPercent);

            //blit_image(batteryLevelImage.imageData, menu_start_x + controllerSectionWidth, offset, batteryLevelImage.width, batteryLevelImage.height);
           /* int barFull = controllerSectionWidth - 20;
            int barWidth = (batteryPercent / 100.0) * barFull;
            //percentage fill
            blit_rect_fill(menu_start_x + controllerSectionWidth + 10, offset + 30, barWidth, 20);
            // percentage borders

            blit_set_color(0xffffff, RGBT(80, 58, 147, 0));
            //left
            blit_rect_fill(menu_start_x + controllerSectionWidth + 9, offset + 30, 1, 20);
            //right
            blit_rect_fill(menu_start_x + controllerSectionWidth + barFull + 1, offset + 30, 1, 20);
            //top
            blit_rect_fill(menu_start_x + controllerSectionWidth + 9, offset + 29, barFull, 1);
            //bottom
            blit_rect_fill(menu_start_x + controllerSectionWidth + 9, offset + 50, barFull, 1);*/

            if (menuState.focussedController == i) {
                // right column buttons
                blit_image(TriangleImage, menu_start_x + (controllerSectionWidth * 2) + 5,
                    offset + 10,
                    TriangleImage_H,
                    TriangleImage_W
                );
                blit_string(menu_start_x + (controllerSectionWidth * 2) + 5, offset + 15 + TriangleImage_H, "disconnect");

                blit_image(SquareImage, menu_start_x + (controllerSectionWidth * 2) + 5,
                    offset + 10 + TriangleImage_H + font->height + 15,
                    SquareImage_H,
                    SquareImage_W
                );
                blit_string(menu_start_x + (controllerSectionWidth * 2) + 5, offset + 30 + SquareImage_H + TriangleImage_H + font->height, "activate");
            }

            //blit_string_windowed_ctr(offset + 30, menu_start_x + controllerSectionWidth, controllerSectionWidth, buffer);
            
            // TODO add a `disconnect button`
           // blit_string_windowed_ctr(textY + offset, menu_start_x + 30, window_width, buffer);
        } else {
            char buffer[64];
            scePaf_snprintf(buffer, sizeof(buffer), "Ctr %d, Not Connected.", i + 1);
            
            blit_string_windowed_ctr(menu_start_x, textY + offset, window_width, buffer);
        }

        // Bottom background half - TODO add bottom border
        blit_set_color(0xffffff, RGBT(80, 58, 147, 128));
       // blit_rect_fill(menu_start_x, offset + textY + font->height, window_width, textY + font->height);
    }

    // bottom border
    blit_set_color(0xffffff, RGBT(80, 58, 147, 0));
    blit_rect_fill(menu_start_x, menu_start_y + window_height + 2, window_width, 2);
}

// Verify menu state is valid and render error messages if not
int ensureStateValid() {
    if (!menuState.btModuleFound) {
        drawWarningMesage("'bt_ctr_driver.prx' not found", "Ensure it exists in 'SEPLUGINS'");
        return 0; 
    } else if (!menuState.btModuleLoaded) {
        drawWarningMesage("'bt_ctr_driver.prx' not loaded", "Ensure it is running.");
        return 0; 
    } else {
        return 1; 
    }
}

int menu_draw() {
    int valid = ensureStateValid();

    if (!valid) {
        return 0;
    }    


    if (menuState.padState.Buttons & PSP_CTRL_UP && !(menuState.prevPadState.Buttons & PSP_CTRL_UP)) {
        if (menuState.focussedController > 0)
            menuState.focussedController--;
        else
            menuState.focussedController = 4;
    }

    if (menuState.padState.Buttons & PSP_CTRL_DOWN && !(menuState.prevPadState.Buttons & PSP_CTRL_DOWN)) {
        if (menuState.focussedController < 4)
            menuState.focussedController++;
        else
            menuState.focussedController = 0;
    }
    
    drawConnectedControllers();

    // Toggle new controller connections. //TODO make this disale itself when menu close, decide which button to use
    if (menuState.padState.Buttons & PSP_CTRL_CIRCLE && !(menuState.prevPadState.Buttons & PSP_CTRL_CIRCLE)) {
        auto response = BtCtrDriverEnableNewConnections(!menuState.newConnectionsEnabled);

        // TODO dont hard code resoponse codes
        if (response == 0x12 || response == 0x11) {
            menuState.newConnectionsEnabled = !menuState.newConnectionsEnabled;
       } else {
            menuAddError();
       }
    }

    return 0;
}