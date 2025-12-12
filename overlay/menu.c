#include "menu.h"
#include "blit.h"

#include "fonts.h"
#include "scepaf.h"
#include "kernel.h"

MenuState menuState = {
    .menuOpen = 0,
    .btModuleFound = 0,
    .btModuleLoaded = 0,
    .boardConnected = 0,
    .newConnectionsEnabled = 0,
    .padState = {0},
    .prevPadState = {0},
    .controllers = {
        {0}
    }
};

MenuState* menu_getPointer(void) {
    return (MenuState*)&menuState;
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
    blit_string_windowed_ctr(messageOneY, menu_start_x, window_width, line1);
    blit_string_windowed_ctr(messageTwoY, menu_start_x, window_width, line2);

    // bottom background half
    blit_set_color(0xffffff, RGBT(80, 58, 147, 128));
    blit_rect_fill(menu_start_x, messageTwoY + font->height, window_width, window_height - topBackgroundHeight - 16);

    // bottom border
    blit_set_color(0xffffff, RGBT(80, 58, 147, 0));
    blit_rect_fill(menu_start_x, menu_start_y + window_height + 2, window_width, 2);
}

void drawConnectedControllers() {
    blit_Gfx *gfx = blit_gfx_pointer();
    font_Data *font = font_data_pointer();

    int window_width = 300;
    int window_height = gfx->height - 40;
    int menu_start_y = (gfx->height - window_height) / 2;
    int menu_start_x = (gfx->width - window_width) / 2;
    int controllerSectionY = (window_height - font->height - 10) / 4;

    // new connections text with padding
    blit_set_color(0xffffff, RGBT(80, 58, 147, 0));
    blit_rect_fill(menu_start_x, menu_start_y, window_width, 5);
    if (menuState.newConnectionsEnabled) {
        blit_string_windowed_ctr(menu_start_y + 5, menu_start_x, window_width, "Connections Enabled");
    } else {
        blit_string_windowed_ctr(menu_start_y + 5, menu_start_x, window_width, "Connections Disabled");
    }    
    blit_rect_fill(menu_start_x, menu_start_y + font->height + 5, window_width, 5);

    // top, left, right border
    blit_set_color(0xffffff, RGBT(80, 58, 147, 0));
    blit_rect_fill(menu_start_x, menu_start_y - 3, window_width, 2); // top
    blit_rect_fill(menu_start_x - 3, menu_start_y, 2, window_height); // left
    blit_rect_fill(menu_start_x + window_width + 2, menu_start_y, 2, window_height); // right

    // Render controller rows
    for (int i = 0; i < 4; i++) {
        ControllerInfo controller = menuState.controllers[i];
        int offset = menu_start_y + font->height + 10 + (controllerSectionY * i);
        int textY = controllerSectionY / 2 - font->height;

        // Top background half - to make text center of pannel
        blit_set_color(0xffffff, RGBT(80, 58, 147, 128));
        blit_rect_fill(menu_start_x, offset, window_width, textY);

        // Render controller status
        blit_set_color(0xffffff, RGBT(80, 58, 147, 0));
        if (controller.connected) {
            char buffer[64];
            // TODO extend controler types into a lookup, maybe imges not text?
            const char *modelName = (controller.controllerModel == 1) ? "DualShock 3" : "DualShock 4";
            /* batteryLevel is 0-255; convert to 0-100% for display.*/
            int batteryPercent = (controller.batteryLevel * 100 + 127) / 255;
            scePaf_snprintf(buffer, sizeof(buffer), "Ctr %d: %s - Battery: %d%%", i + 1, modelName, batteryPercent);
            
            // TODO add a `disconnect button`
            blit_string_windowed_ctr(textY + offset, menu_start_x, window_width, buffer);
        } else {
            char buffer[64];
            scePaf_snprintf(buffer, sizeof(buffer), "Ctr %d, Not Connected.", i + 1);
            
            blit_string_windowed_ctr(textY + offset, menu_start_x, window_width, buffer);
        }

        // Bottom background half - TODO add bottom border
        blit_set_color(0xffffff, RGBT(80, 58, 147, 128));
        blit_rect_fill(menu_start_x, offset + textY + font->height, window_width, textY + font->height);
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

    drawConnectedControllers();

    // Toggle new controller connections. //TODO make this disale itself when menu close, decide which button to use
    if (menuState.padState.Buttons & PSP_CTRL_SQUARE && !(menuState.prevPadState.Buttons & PSP_CTRL_SQUARE)) {
        auto response = BtCtrDriverEnableNewConnections(!menuState.newConnectionsEnabled);

        // TODO dont hard code resoponse codes
        if (response == 0x12 || response == 0x11) {
            menuState.newConnectionsEnabled = !menuState.newConnectionsEnabled;
       }
    }

    return 0;
}