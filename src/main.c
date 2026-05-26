#include <windows.h>
#include <stdlib.h>
#include <string.h>

#include "keytweak/app.h"
#include "keytweak/catalog.h"
#include "keytweak/layout.h"
#include "keytweak/remap.h"

enum {
    IDC_KEY_LIST = 101,
    IDC_FROM_LABEL,
    IDC_FROM_EDIT,
    IDC_TO_LABEL,
    IDC_TO_EDIT,
    IDC_ADD_REMAP,
    IDC_ACTIVATE_REMAP,
    IDC_STATUS_TEXT,
};

static HINSTANCE g_hInstance = NULL;
static kt_remap_table g_remaps = {0};
static kt_layout_snapshot g_layout = {0};
static bool g_remap_active = false;

static int utf8_to_utf16(wchar_t *dst, size_t size, const char *src)
{
    if (!dst || size == 0 || !src) {
        return 0;
    }
    int count = MultiByteToWideChar(CP_UTF8, 0, src, -1, dst, (int)size);
    return count > 0;
}

static int utf16_to_utf8(char *dst, size_t size, const wchar_t *src)
{
    if (!dst || size == 0 || !src) {
        return 0;
    }
    int count = WideCharToMultiByte(CP_UTF8, 0, src, -1, dst, (int)size, NULL, NULL);
    return count > 0;
}

static void set_status_text(HWND hwnd, const wchar_t *text)
{
    HWND status = GetDlgItem(hwnd, IDC_STATUS_TEXT);
    if (status) {
        SetWindowTextW(status, text);
    }
}

static void add_list_item(HWND list, const char *utf8_text)
{
    wchar_t buffer[256];
    if (utf8_to_utf16(buffer, ARRAYSIZE(buffer), utf8_text)) {
        SendMessageW(list, LB_ADDSTRING, 0, (LPARAM)buffer);
    }
}

static void refresh_display(HWND hwnd)
{
    HWND list = GetDlgItem(hwnd, IDC_KEY_LIST);
    if (!list) {
        return;
    }

    SendMessageW(list, LB_RESETCONTENT, 0, 0);
    wchar_t header[64];
    swprintf(header, ARRAYSIZE(header), L"%hs %hs - detected %zu keys", KEYTWEAK_NAME, KEYTWEAK_VERSION, g_layout.key_count);
    SendMessageW(list, LB_ADDSTRING, 0, (LPARAM)header);
    SendMessageW(list, LB_ADDSTRING, 0, (LPARAM)L"--- layout ---");

    for (size_t i = 0; i < g_layout.key_count && i < 64; i++) {
        const kt_physical_key *key = &g_layout.keys[i];
        const char *label = kt_catalog_label(key->id);
        char item[128];
        snprintf(item, sizeof(item), "%2zu: %s  vk=0x%04X row=%d col=%d", i + 1, label, (unsigned)key->vk, key->row, key->col);
        add_list_item(list, item);
    }

    SendMessageW(list, LB_ADDSTRING, 0, (LPARAM)L"");
    SendMessageW(list, LB_ADDSTRING, 0, (LPARAM)L"--- remaps ---");
    if (g_remaps.count == 0) {
        SendMessageW(list, LB_ADDSTRING, 0, (LPARAM)L"(none)");
    } else {
        for (size_t i = 0; i < g_remaps.count; i++) {
            const char *from = kt_catalog_label(g_remaps.sources[i]);
            const char *to = kt_catalog_label(g_remaps.targets[i]);
            char item[128];
            snprintf(item, sizeof(item), "%s -> %s", from, to);
            add_list_item(list, item);
        }
    }
}

static void update_controls(HWND hwnd)
{
    HWND from_edit = GetDlgItem(hwnd, IDC_FROM_EDIT);
    HWND to_edit = GetDlgItem(hwnd, IDC_TO_EDIT);
    HWND add_button = GetDlgItem(hwnd, IDC_ADD_REMAP);
    HWND activate_button = GetDlgItem(hwnd, IDC_ACTIVATE_REMAP);

    if (g_remap_active) {
        EnableWindow(from_edit, FALSE);
        EnableWindow(to_edit, FALSE);
        EnableWindow(add_button, FALSE);
        SetWindowTextW(activate_button, L"Deactivate Remap");
        set_status_text(hwnd, L"Remapping active. Close the window to stop.");
    } else {
        EnableWindow(from_edit, TRUE);
        EnableWindow(to_edit, TRUE);
        EnableWindow(add_button, TRUE);
        SetWindowTextW(activate_button, L"Activate Remap");
        set_status_text(hwnd, L"Enter a remap and press Add Remap.");
    }
}

static void add_remap(HWND hwnd)
{
    wchar_t from_w[64];
    wchar_t to_w[64];
    char from_utf8[64];
    char to_utf8[64];

    GetWindowTextW(GetDlgItem(hwnd, IDC_FROM_EDIT), from_w, ARRAYSIZE(from_w));
    GetWindowTextW(GetDlgItem(hwnd, IDC_TO_EDIT), to_w, ARRAYSIZE(to_w));
    if (!utf16_to_utf8(from_utf8, sizeof(from_utf8), from_w) || !utf16_to_utf8(to_utf8, sizeof(to_utf8), to_w)) {
        MessageBoxW(hwnd, L"Unable to read remap labels.", L"Error", MB_OK | MB_ICONERROR);
        return;
    }

    kt_key_id from_id = kt_catalog_id(from_utf8);
    kt_key_id to_id = kt_catalog_id(to_utf8);
    if (from_id == KT_KEY_ID_INVALID || to_id == KT_KEY_ID_INVALID) {
        MessageBoxW(hwnd, L"Invalid key label. Use names like Caps, Esc, F1, Space, L-Ctrl.", L"Invalid Key", MB_OK | MB_ICONWARNING);
        return;
    }

    if (kt_remap_set(&g_remaps, from_id, to_id) != 0) {
        MessageBoxW(hwnd, L"Unable to add remap. Try again.", L"Error", MB_OK | MB_ICONERROR);
        return;
    }

    SetWindowTextW(GetDlgItem(hwnd, IDC_FROM_EDIT), L"");
    SetWindowTextW(GetDlgItem(hwnd, IDC_TO_EDIT), L"");
    refresh_display(hwnd);
    set_status_text(hwnd, L"Remap added. Activate to apply.");
}

static void toggle_remap(HWND hwnd)
{
    if (g_remap_active) {
        kt_remap_uninstall();
        g_remap_active = false;
        update_controls(hwnd);
        refresh_display(hwnd);
        return;
    }

    if (g_remaps.count == 0) {
        MessageBoxW(hwnd, L"Add at least one remap before activating.", L"No Remaps", MB_OK | MB_ICONINFORMATION);
        return;
    }

    if (kt_remap_install(&g_remaps) != 0) {
        MessageBoxW(hwnd, L"Could not install keyboard remap hook.", L"Error", MB_OK | MB_ICONERROR);
        return;
    }

    g_remap_active = true;
    update_controls(hwnd);
}

static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {
    case WM_CREATE:
        {
            HWND list = CreateWindowExW(0, L"LISTBOX", NULL,
                WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL | LBS_NOTIFY,
                10, 10, 560, 260,
                hwnd, (HMENU)(intptr_t)IDC_KEY_LIST, g_hInstance, NULL);

            CreateWindowExW(0, L"STATIC", L"From:", WS_CHILD | WS_VISIBLE,
                10, 280, 40, 20, hwnd, (HMENU)(intptr_t)IDC_FROM_LABEL, g_hInstance, NULL);
            CreateWindowExW(0, L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
                55, 280, 180, 24, hwnd, (HMENU)(intptr_t)IDC_FROM_EDIT, g_hInstance, NULL);
            CreateWindowExW(0, L"STATIC", L"To:", WS_CHILD | WS_VISIBLE,
                250, 280, 25, 20, hwnd, (HMENU)(intptr_t)IDC_TO_LABEL, g_hInstance, NULL);
            CreateWindowExW(0, L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
                280, 280, 180, 24, hwnd, (HMENU)(intptr_t)IDC_TO_EDIT, g_hInstance, NULL);

            CreateWindowExW(0, L"BUTTON", L"Add Remap", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                470, 278, 100, 26, hwnd, (HMENU)(intptr_t)IDC_ADD_REMAP, g_hInstance, NULL);
            CreateWindowExW(0, L"BUTTON", L"Activate Remap", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                10, 320, 140, 28, hwnd, (HMENU)(intptr_t)IDC_ACTIVATE_REMAP, g_hInstance, NULL);
            CreateWindowExW(0, L"STATIC", L"Status:", WS_CHILD | WS_VISIBLE,
                10, 360, 60, 20, hwnd, NULL, g_hInstance, NULL);
            CreateWindowExW(0, L"STATIC", L"", WS_CHILD | WS_VISIBLE,
                70, 360, 500, 20, hwnd, (HMENU)(intptr_t)IDC_STATUS_TEXT, g_hInstance, NULL);

            if (kt_layout_probe(&g_layout) != 0) {
                MessageBoxW(hwnd, L"Failed to detect keyboard layout.", L"Error", MB_OK | MB_ICONERROR);
                PostQuitMessage(1);
                return -1;
            }

            kt_remap_init(&g_remaps);
            refresh_display(hwnd);
            update_controls(hwnd);
        }
        return 0;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_ADD_REMAP:
            add_remap(hwnd);
            return 0;
        case IDC_ACTIVATE_REMAP:
            toggle_remap(hwnd);
            return 0;
        }
        return 0;

    case WM_CLOSE:
        if (g_remap_active) {
            kt_remap_uninstall();
            g_remap_active = false;
        }
        DestroyWindow(hwnd);
        return 0;

    case WM_DESTROY:
        kt_remap_free(&g_remaps);
        kt_layout_free(&g_layout);
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    (void)hPrevInstance;
    (void)lpCmdLine;

    g_hInstance = hInstance;
    WNDCLASSEXW wc = {0};
    wc.cbSize = sizeof(wc);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = L"KeyTweakWindowClass";

    if (!RegisterClassExW(&wc)) {
        return 1;
    }

    HWND hwnd = CreateWindowExW(0, wc.lpszClassName, L"KeyTweak", WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, 600, 430, NULL, NULL, hInstance, NULL);
    if (!hwnd) {
        return 1;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessageW(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    return (int)msg.wParam;
}
