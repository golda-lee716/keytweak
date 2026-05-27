#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <commctrl.h>

#include "keytweak/app.h"
#include "keytweak/catalog.h"
#include "keytweak/layout.h"
#include "keytweak/remap.h"

#pragma comment(lib, "comctl32.lib")

enum {
    IDC_KEYBOARD_LAYOUT_LIST = 101,
    IDC_FROM_COMBO = 102,
    IDC_TO_COMBO = 103,
    IDC_ADD_REMAP_BUTTON = 104,
    IDC_ACTIVATE_BUTTON = 105,
    IDC_STATUS_INDICATOR = 106,
    IDC_STATUS_TEXT = 107,
    IDC_REMAP_LIST = 108,
    IDC_DELETE_BUTTON = 109,
    IDC_CLEAR_ALL_BUTTON = 110,
    IDC_DETECTED_KEYS_GROUP = 111,
    IDC_REMAPS_GROUP = 112,
    IDC_CONTROLS_GROUP = 113,
};

static HINSTANCE g_hInstance = NULL;
static kt_remap_table g_remaps = {0};
static kt_layout_snapshot g_layout = {0};
static bool g_remap_active = false;
static HFONT g_hFontTitle = NULL;
static HFONT g_hFontNormal = NULL;

// Color constants for visual design
#define COLOR_ACTIVE RGB(76, 175, 80)      // Green
#define COLOR_INACTIVE RGB(158, 158, 158)  // Gray
#define COLOR_BG_LIGHT RGB(245, 245, 245)  // Light gray background

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

static void set_status_text(HWND hwnd, const wchar_t *text, bool is_active)
{
    HWND status = GetDlgItem(hwnd, IDC_STATUS_TEXT);
    if (status) {
        SetWindowTextW(status, text);
        if (is_active) {
            SetTextColor(GetDC(status), COLOR_ACTIVE);
        } else {
            SetTextColor(GetDC(status), COLOR_INACTIVE);
        }
    }
}

// Initialize and populate key combo boxes with all available keys
static void populate_key_combos(HWND from_combo, HWND to_combo)
{
    // Add keys to combo boxes - get them from the layout
    char key_list[4096] = {0};
    int offset = 0;
    
    for (size_t i = 0; i < g_layout.key_count; i++) {
        const char *label = kt_catalog_label(g_layout.keys[i].id);
        if (label && offset < (int)sizeof(key_list) - strlen(label) - 1) {
            wchar_t buffer[256];
            if (utf8_to_utf16(buffer, ARRAYSIZE(buffer), label)) {
                SendMessageW(from_combo, CB_ADDSTRING, 0, (LPARAM)buffer);
                SendMessageW(to_combo, CB_ADDSTRING, 0, (LPARAM)buffer);
            }
        }
    }
}

static void refresh_remap_list(HWND hwnd)
{
    HWND list = GetDlgItem(hwnd, IDC_REMAP_LIST);
    if (!list) {
        return;
    }

    SendMessageW(list, LB_RESETCONTENT, 0, 0);
    
    if (g_remaps.count == 0) {
        SendMessageW(list, LB_ADDSTRING, 0, (LPARAM)L"(No remaps configured)");
    } else {
        for (size_t i = 0; i < g_remaps.count; i++) {
            const char *from = kt_catalog_label(g_remaps.sources[i]);
            const char *to = kt_catalog_label(g_remaps.targets[i]);
            char item[256];
            snprintf(item, sizeof(item), "%-20s  →  %-20s", from ? from : "?", to ? to : "?");
            
            wchar_t buffer[256];
            if (utf8_to_utf16(buffer, ARRAYSIZE(buffer), item)) {
                SendMessageW(list, LB_ADDSTRING, 0, (LPARAM)buffer);
            }
        }
    }
}

static void refresh_keyboard_display(HWND hwnd)
{
    HWND list = GetDlgItem(hwnd, IDC_KEYBOARD_LAYOUT_LIST);
    if (!list) {
        return;
    }

    SendMessageW(list, LB_RESETCONTENT, 0, 0);
    
    char header[128];
    snprintf(header, sizeof(header), "Detected %zu keys on %s %s", g_layout.key_count, KEYTWEAK_NAME, KEYTWEAK_VERSION);
    wchar_t header_w[128];
    if (utf8_to_utf16(header_w, ARRAYSIZE(header_w), header)) {
        SendMessageW(list, LB_ADDSTRING, 0, (LPARAM)header_w);
        SendMessageW(list, LB_ADDSTRING, 0, (LPARAM)L"");
    }

    for (size_t i = 0; i < g_layout.key_count && i < 64; i++) {
        const kt_physical_key *key = &g_layout.keys[i];
        const char *label = kt_catalog_label(key->id);
        char item[256];
        snprintf(item, sizeof(item), "%-15s (VK: 0x%02X) | Row: %d, Col: %d", 
                 label ? label : "?", (unsigned)key->vk, key->row, key->col);
        
        wchar_t buffer[256];
        if (utf8_to_utf16(buffer, ARRAYSIZE(buffer), item)) {
            SendMessageW(list, LB_ADDSTRING, 0, (LPARAM)buffer);
        }
    }
}

static void update_controls(HWND hwnd)
{
    HWND from_combo = GetDlgItem(hwnd, IDC_FROM_COMBO);
    HWND to_combo = GetDlgItem(hwnd, IDC_TO_COMBO);
    HWND add_button = GetDlgItem(hwnd, IDC_ADD_REMAP_BUTTON);
    HWND activate_button = GetDlgItem(hwnd, IDC_ACTIVATE_BUTTON);
    HWND delete_button = GetDlgItem(hwnd, IDC_DELETE_BUTTON);
    HWND clear_button = GetDlgItem(hwnd, IDC_CLEAR_ALL_BUTTON);
    HWND status_text = GetDlgItem(hwnd, IDC_STATUS_TEXT);
    HWND status_indicator = GetDlgItem(hwnd, IDC_STATUS_INDICATOR);

    if (g_remap_active) {
        EnableWindow(from_combo, FALSE);
        EnableWindow(to_combo, FALSE);
        EnableWindow(add_button, FALSE);
        EnableWindow(delete_button, FALSE);
        EnableWindow(clear_button, FALSE);
        SetWindowTextW(activate_button, L"⊗ Deactivate Remapping");
        
        set_status_text(hwnd, L"● ACTIVE: Remapping is running. Close app to stop.", TRUE);
        
        // Visual indicator
        InvalidateRect(status_indicator, NULL, FALSE);
    } else {
        EnableWindow(from_combo, TRUE);
        EnableWindow(to_combo, TRUE);
        EnableWindow(add_button, TRUE);
        EnableWindow(delete_button, g_remaps.count > 0 ? TRUE : FALSE);
        EnableWindow(clear_button, g_remaps.count > 0 ? TRUE : FALSE);
        SetWindowTextW(activate_button, L"▶ Activate Remapping");
        
        set_status_text(hwnd, L"● INACTIVE: Configure remaps and press Activate", FALSE);
        
        InvalidateRect(status_indicator, NULL, FALSE);
    }
}

static void add_remap(HWND hwnd)
{
    HWND from_combo = GetDlgItem(hwnd, IDC_FROM_COMBO);
    HWND to_combo = GetDlgItem(hwnd, IDC_TO_COMBO);
    
    int from_sel = (int)SendMessageW(from_combo, CB_GETCURSEL, 0, 0);
    int to_sel = (int)SendMessageW(to_combo, CB_GETCURSEL, 0, 0);
    
    if (from_sel == CB_ERR || to_sel == CB_ERR) {
        MessageBoxW(hwnd, L"Please select both a 'From' and 'To' key.", L"Selection Required", MB_OK | MB_ICONINFORMATION);
        return;
    }
    
    if (from_sel == to_sel) {
        MessageBoxW(hwnd, L"Cannot remap a key to itself.", L"Invalid Mapping", MB_OK | MB_ICONWARNING);
        return;
    }

    // Get the key IDs from layout
    if ((size_t)from_sel >= g_layout.key_count || (size_t)to_sel >= g_layout.key_count) {
        MessageBoxW(hwnd, L"Invalid key selection.", L"Error", MB_OK | MB_ICONERROR);
        return;
    }

    kt_key_id from_id = g_layout.keys[from_sel].id;
    kt_key_id to_id = g_layout.keys[to_sel].id;

    if (kt_remap_set(&g_remaps, from_id, to_id) != 0) {
        MessageBoxW(hwnd, L"Unable to add remap. Maximum remaps may be reached.", L"Error", MB_OK | MB_ICONERROR);
        return;
    }

    SendMessageW(from_combo, CB_SETCURSEL, (WPARAM)-1, 0);
    SendMessageW(to_combo, CB_SETCURSEL, (WPARAM)-1, 0);
    
    refresh_remap_list(hwnd);
    update_controls(hwnd);
    set_status_text(hwnd, L"✓ Remap added successfully!", FALSE);
}

static void delete_selected_remap(HWND hwnd)
{
    HWND list = GetDlgItem(hwnd, IDC_REMAP_LIST);
    int sel = (int)SendMessageW(list, LB_GETCURSEL, 0, 0);
    
    if (sel == LB_ERR || sel < 0 || (size_t)sel >= g_remaps.count) {
        MessageBoxW(hwnd, L"Please select a remap to delete.", L"No Selection", MB_OK | MB_ICONINFORMATION);
        return;
    }
    
    // Remove the remap
    for (size_t i = (size_t)sel; i < g_remaps.count - 1; i++) {
        g_remaps.sources[i] = g_remaps.sources[i + 1];
        g_remaps.targets[i] = g_remaps.targets[i + 1];
    }
    g_remaps.count--;
    
    refresh_remap_list(hwnd);
    update_controls(hwnd);
    set_status_text(hwnd, L"✓ Remap deleted.", FALSE);
}

static void clear_all_remaps(HWND hwnd)
{
    int result = MessageBoxW(hwnd, L"Delete all remaps? This cannot be undone.", L"Confirm", MB_YESNO | MB_ICONQUESTION);
    if (result != IDYES) {
        return;
    }
    
    g_remaps.count = 0;
    refresh_remap_list(hwnd);
    update_controls(hwnd);
    set_status_text(hwnd, L"✓ All remaps cleared.", FALSE);
}

static void toggle_remap(HWND hwnd)
{
    if (g_remap_active) {
        kt_remap_uninstall();
        g_remap_active = false;
        update_controls(hwnd);
        return;
    }

    if (g_remaps.count == 0) {
        MessageBoxW(hwnd, L"Add at least one remap before activating.", L"No Remaps", MB_OK | MB_ICONINFORMATION);
        return;
    }

    if (kt_remap_install(&g_remaps) != 0) {
        MessageBoxW(hwnd, L"Could not install keyboard remap hook.\n\nThis may require administrator privileges.", L"Error", MB_OK | MB_ICONERROR);
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
            // Initialize fonts
            g_hFontTitle = CreateFontW(16, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, 
                                       ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, 
                                       DEFAULT_QUALITY, DEFAULT_PITCH, L"Segoe UI");
            g_hFontNormal = CreateFontW(11, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, 
                                        ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, 
                                        DEFAULT_QUALITY, DEFAULT_PITCH, L"Segoe UI");

            int y_pos = 15;

            // Detected Keyboard Section
            CreateWindowExW(0, L"BUTTON", L"Detected Keyboard Layout", 
                WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
                10, y_pos, 760, 140, hwnd, (HMENU)(intptr_t)IDC_DETECTED_KEYS_GROUP, g_hInstance, NULL);
            y_pos += 20;

            HWND keyboard_list = CreateWindowExW(WS_EX_CLIENTEDGE, L"LISTBOX", NULL,
                WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOTIFY,
                20, y_pos, 740, 110,
                hwnd, (HMENU)(intptr_t)IDC_KEYBOARD_LAYOUT_LIST, g_hInstance, NULL);
            SendMessageW(keyboard_list, WM_SETFONT, (WPARAM)g_hFontNormal, FALSE);

            y_pos += 130;

            // Key Remapping Configuration Section
            CreateWindowExW(0, L"BUTTON", L"Configure Remapping", 
                WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
                10, y_pos, 760, 120, hwnd, (HMENU)(intptr_t)IDC_CONTROLS_GROUP, g_hInstance, NULL);
            
            int config_y = y_pos + 20;
            
            // From key label and combo
            CreateWindowExW(0, L"STATIC", L"Map From:", 
                WS_CHILD | WS_VISIBLE, 20, config_y, 65, 20, 
                hwnd, NULL, g_hInstance, NULL);
            HWND from_combo = CreateWindowExW(WS_EX_CLIENTEDGE, L"COMBOBOX", L"",
                WS_CHILD | WS_VISIBLE | CBS_DROPDOWN | CBS_HASSTRINGS,
                90, config_y - 2, 130, 100,
                hwnd, (HMENU)(intptr_t)IDC_FROM_COMBO, g_hInstance, NULL);
            SendMessageW(from_combo, WM_SETFONT, (WPARAM)g_hFontNormal, FALSE);

            // To key label and combo
            CreateWindowExW(0, L"STATIC", L"Map To:", 
                WS_CHILD | WS_VISIBLE, 230, config_y, 55, 20, 
                hwnd, NULL, g_hInstance, NULL);
            HWND to_combo = CreateWindowExW(WS_EX_CLIENTEDGE, L"COMBOBOX", L"",
                WS_CHILD | WS_VISIBLE | CBS_DROPDOWN | CBS_HASSTRINGS,
                290, config_y - 2, 130, 100,
                hwnd, (HMENU)(intptr_t)IDC_TO_COMBO, g_hInstance, NULL);
            SendMessageW(to_combo, WM_SETFONT, (WPARAM)g_hFontNormal, FALSE);

            // Add button
            HWND add_btn = CreateWindowExW(0, L"BUTTON", L"+ Add Mapping", 
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                430, config_y - 2, 110, 26, 
                hwnd, (HMENU)(intptr_t)IDC_ADD_REMAP_BUTTON, g_hInstance, NULL);
            SendMessageW(add_btn, WM_SETFONT, (WPARAM)g_hFontNormal, FALSE);

            config_y += 32;

            // Delete and Clear buttons on second row
            HWND del_btn = CreateWindowExW(0, L"BUTTON", L"- Delete Selected", 
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                90, config_y, 130, 24, 
                hwnd, (HMENU)(intptr_t)IDC_DELETE_BUTTON, g_hInstance, NULL);
            SendMessageW(del_btn, WM_SETFONT, (WPARAM)g_hFontNormal, FALSE);

            HWND clear_btn = CreateWindowExW(0, L"BUTTON", L"✕ Clear All", 
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                230, config_y, 100, 24, 
                hwnd, (HMENU)(intptr_t)IDC_CLEAR_ALL_BUTTON, g_hInstance, NULL);
            SendMessageW(clear_btn, WM_SETFONT, (WPARAM)g_hFontNormal, FALSE);

            y_pos += 130;

            // Active Remaps Section
            CreateWindowExW(0, L"BUTTON", L"Active Remaps", 
                WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
                10, y_pos, 760, 110, hwnd, (HMENU)(intptr_t)IDC_REMAPS_GROUP, g_hInstance, NULL);
            y_pos += 20;

            HWND remap_list = CreateWindowExW(WS_EX_CLIENTEDGE, L"LISTBOX", NULL,
                WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOTIFY,
                20, y_pos, 740, 80,
                hwnd, (HMENU)(intptr_t)IDC_REMAP_LIST, g_hInstance, NULL);
            SendMessageW(remap_list, WM_SETFONT, (WPARAM)g_hFontNormal, FALSE);

            y_pos += 100;

            // Control Buttons Section
            HWND activate_btn = CreateWindowExW(0, L"BUTTON", L"▶ Activate Remapping", 
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                10, y_pos, 200, 32, 
                hwnd, (HMENU)(intptr_t)IDC_ACTIVATE_BUTTON, g_hInstance, NULL);
            SendMessageW(activate_btn, WM_SETFONT, (WPARAM)g_hFontNormal, FALSE);

            // Status section
            CreateWindowExW(0, L"STATIC", L"Status:", 
                WS_CHILD | WS_VISIBLE, 230, y_pos + 5, 50, 20, 
                hwnd, NULL, g_hInstance, NULL);
            
            HWND status_ind = CreateWindowExW(WS_EX_CLIENTEDGE, L"STATIC", L"", 
                WS_CHILD | WS_VISIBLE | SS_CENTER | SS_SUNKEN,
                290, y_pos, 20, 20, 
                hwnd, (HMENU)(intptr_t)IDC_STATUS_INDICATOR, g_hInstance, NULL);
            
            HWND status_text = CreateWindowExW(0, L"STATIC", L"Loading...", 
                WS_CHILD | WS_VISIBLE,
                320, y_pos + 5, 450, 20, 
                hwnd, (HMENU)(intptr_t)IDC_STATUS_TEXT, g_hInstance, NULL);
            SendMessageW(status_text, WM_SETFONT, (WPARAM)g_hFontNormal, FALSE);

            // Initialize
            if (kt_layout_probe(&g_layout) != 0) {
                MessageBoxW(hwnd, L"Failed to detect keyboard layout.", L"Error", MB_OK | MB_ICONERROR);
                PostQuitMessage(1);
                return -1;
            }

            kt_remap_init(&g_remaps);
            populate_key_combos(from_combo, to_combo);
            refresh_keyboard_display(hwnd);
            refresh_remap_list(hwnd);
            update_controls(hwnd);
        }
        return 0;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_ADD_REMAP_BUTTON:
            add_remap(hwnd);
            return 0;
        case IDC_DELETE_BUTTON:
            delete_selected_remap(hwnd);
            return 0;
        case IDC_CLEAR_ALL_BUTTON:
            clear_all_remaps(hwnd);
            return 0;
        case IDC_ACTIVATE_BUTTON:
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
        if (g_hFontTitle) DeleteObject(g_hFontTitle);
        if (g_hFontNormal) DeleteObject(g_hFontNormal);
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
    
    // Initialize common controls
    InitCommonControls();

    WNDCLASSEXW wc = {0};
    wc.cbSize = sizeof(wc);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = L"KeyTweakWindowClass";
    wc.hIcon = LoadIconW(NULL, IDI_APPLICATION);
    wc.hIconSm = LoadIconW(NULL, IDI_APPLICATION);

    if (!RegisterClassExW(&wc)) {
        return 1;
    }

    // Create main window - larger for better layout
    HWND hwnd = CreateWindowExW(0, wc.lpszClassName, L"KeyTweak - Keyboard Remapper", 
        WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 650, 
        NULL, NULL, hInstance, NULL);
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
