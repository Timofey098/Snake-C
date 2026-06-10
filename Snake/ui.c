#include "ui.h"
#include <stdio.h>
#include <string.h>

static int client_w = 800;
static int client_h = 600;
static HBITMAP hBackBuffer = NULL;
static HDC     hBackDC = NULL;

static void recreate_backbuffer(HWND hwnd) {
    if (hBackBuffer) { DeleteObject(hBackBuffer); hBackBuffer = NULL; }
    if (hBackDC) { DeleteDC(hBackDC); hBackDC = NULL; }

    HDC hdc = GetDC(hwnd);
    if (!hdc) return;
    hBackDC = CreateCompatibleDC(hdc);
    hBackBuffer = CreateCompatibleBitmap(hdc, client_w, client_h);
    if (hBackDC && hBackBuffer) {
        SelectObject(hBackDC, hBackBuffer);
    }
    ReleaseDC(hwnd, hdc);
}

static void ui_draw(const GameState* state) {
    if (!hBackDC || !hBackBuffer) return;

    // 1. Очистка фона
    HBRUSH hClear = CreateSolidBrush(RGB(20, 20, 20));
    HBRUSH hOldBrush = (HBRUSH)SelectObject(hBackDC, hClear);
    Rectangle(hBackDC, 0, 0, client_w, client_h);
    SelectObject(hBackDC, hOldBrush);
    DeleteObject(hClear);

    // 2. Резервируем верхнюю панель под UI и считаем размер ячейки
    const int UI_MARGIN_TOP = 40;
    int available_h = client_h - UI_MARGIN_TOP;
    if (available_h < 60) available_h = 60;

    int cell_w = client_w / GRID_W;
    int cell_h = available_h / GRID_H;
    int cell_size = (cell_w < cell_h) ? cell_w : cell_h;
    if (cell_size < 4) cell_size = 4;

    int offset_x = (client_w - cell_size * GRID_W) / 2;
    int offset_y = UI_MARGIN_TOP + (available_h - cell_size * GRID_H) / 2;

    // 3. Сетка
    HPEN hGridPen = CreatePen(PS_SOLID, 1, RGB(45, 45, 45));
    HPEN hOldPen = (HPEN)SelectObject(hBackDC, hGridPen);
    SelectObject(hBackDC, GetStockObject(NULL_BRUSH));
    SetBkMode(hBackDC, TRANSPARENT);

    for (int x = 0; x <= GRID_W; ++x) {
        int px = offset_x + x * cell_size;
        MoveToEx(hBackDC, px, offset_y, NULL);
        LineTo(hBackDC, px, offset_y + GRID_H * cell_size);
    }
    for (int y = 0; y <= GRID_H; ++y) {
        int py = offset_y + y * cell_size;
        MoveToEx(hBackDC, offset_x, py, NULL);
        LineTo(hBackDC, offset_x + GRID_W * cell_size, py);
    }
    SelectObject(hBackDC, hOldPen);
    DeleteObject(hGridPen);

    // 4. Змейка
    HBRUSH hSnake = CreateSolidBrush(RGB(0, 200, 80));
    hOldBrush = (HBRUSH)SelectObject(hBackDC, hSnake);
    for (int i = 0; i < state->length; ++i) {
        int x = offset_x + state->snake[i].x * cell_size;
        int y = offset_y + state->snake[i].y * cell_size;
        Rectangle(hBackDC, x + 1, y + 1, x + cell_size - 1, y + cell_size - 1);
    }
    SelectObject(hBackDC, hOldBrush);
    DeleteObject(hSnake);

    // 5. Еда
    HBRUSH hFood = CreateSolidBrush(RGB(220, 40, 40));
    hOldBrush = (HBRUSH)SelectObject(hBackDC, hFood);
    int fx = offset_x + state->food.x * cell_size;
    int fy = offset_y + state->food.y * cell_size;
    Ellipse(hBackDC, fx + 2, fy + 2, fx + cell_size - 2, fy + cell_size - 2);
    SelectObject(hBackDC, hOldBrush);
    DeleteObject(hFood);

    // 6. Текст и UI
    HFONT hFont = CreateFontA(24, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
        DEFAULT_PITCH | FF_SWISS, "Consolas");
    HFONT hOldFont = (HFONT)SelectObject(hBackDC, hFont);
    SetTextColor(hBackDC, RGB(255, 255, 255));
    SetBkMode(hBackDC, TRANSPARENT);

    char buf[32];
    snprintf(buf, sizeof(buf), "Score: %d", state->score);
    RECT scoreRect = { 10, 10, client_w, 40 };
    DrawTextA(hBackDC, buf, -1, &scoreRect, DT_LEFT | DT_TOP | DT_SINGLELINE);

    if (state->game_over) {
        RECT centerRect = { 0, 0, client_w, client_h };
        DrawTextA(hBackDC, "GAME OVER", -1, &centerRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        centerRect.top = client_h / 2 + 35;
        centerRect.bottom = client_h / 2 + 65;
        DrawTextA(hBackDC, "Press SPACE to restart", -1, &centerRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }
    else if (state->paused) {
        RECT centerRect = { 0, 0, client_w, client_h };
        DrawTextA(hBackDC, "PAUSED", -1, &centerRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }

    SelectObject(hBackDC, hOldFont);
    DeleteObject(hFont);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    GameState* state = (GameState*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

    switch (msg) {
    case WM_CREATE:
        recreate_backbuffer(hwnd);
        return 0;
    case WM_SIZE:
        if (wParam == SIZE_MINIMIZED) return 0;
        client_w = LOWORD(lParam);
        client_h = HIWORD(lParam);
        recreate_backbuffer(hwnd);
        return 0;
    case WM_KEYDOWN:
        if (state) core_handle_input(state, wParam);
        return 0;
    case WM_DESTROY:
        if (hBackBuffer) DeleteObject(hBackBuffer);
        if (hBackDC)     DeleteDC(hBackDC);
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

int ui_run(GameState* state) {
    WNDCLASSEX wc = { 0 };
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.lpszClassName = TEXT("SnakeWin32");
    if (!RegisterClassEx(&wc)) return 1;

    DWORD style = WS_OVERLAPPEDWINDOW;
    RECT rc = { 0, 0, 800, 600 };
    AdjustWindowRect(&rc, style, FALSE);

    HWND hwnd = CreateWindowEx(0, TEXT("SnakeWin32"), TEXT("Snake - C/Win32"),
        style, CW_USEDEFAULT, CW_USEDEFAULT,
        rc.right - rc.left, rc.bottom - rc.top,
        NULL, NULL, wc.hInstance, NULL);
    if (!hwnd) return 1;

    SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)state);
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    LARGE_INTEGER freq, lastTime;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&lastTime);

    MSG msg;
    BOOL running = TRUE;

    while (running) {
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                running = FALSE;
                break;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        if (!running) break;

        LARGE_INTEGER now;
        QueryPerformanceCounter(&now);
        float dt = (float)(now.QuadPart - lastTime.QuadPart) / (float)freq.QuadPart;
        lastTime = now;

        core_update(state, dt);
        ui_draw(state);

        HDC hdc = GetDC(hwnd);
        if (hdc && hBackDC) {
            BitBlt(hdc, 0, 0, client_w, client_h, hBackDC, 0, 0, SRCCOPY);
        }
        ReleaseDC(hwnd, hdc);
    }

    return 0;
}