#include <windows.h>
#include "core.h"
#include "ui.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    (void)hInstance; (void)hPrevInstance; (void)lpCmdLine; (void)nCmdShow;

    GameState state;
    core_init(&state);

    return ui_run(&state);
}