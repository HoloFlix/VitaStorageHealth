#include <stdio.h>
#include <stdlib.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/ctrl.h>
#include "ui.h"

int main(int argc, char *argv[]) {
    AppState app;
    SceCtrlData pad;

    // Enable button input reading
    sceCtrlSetSamplingMode(SCE_CTRL_MODE_DIGITAL);

    // Initialize UI and graphics subsystems
    ui_init(&app);

    // Main App Loop
    while (!app.quit_requested) {
        sceCtrlPeekBufferPositive(0, &pad, 1);
        
        ui_update(&app, &pad);
        ui_render(&app);

        // Limit frame execution rate to ~60 FPS
        sceKernelDelayThread(16000);
    }

    ui_cleanup();
    sceKernelExitProcess(0);
    return 0;
}
