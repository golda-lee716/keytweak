#include "keytweak/remap.h"
#include "keytweak/catalog.h"

#include <stdlib.h>
#include <string.h>

#ifdef KEYTWEAK_PLATFORM_WIN32
#include <windows.h>
#endif

static const kt_remap_table *g_remap_table = NULL;
static HHOOK g_keyboard_hook = NULL;

void kt_remap_init(kt_remap_table *t)
{
    if (!t) {
        return;
    }
    memset(t, 0, sizeof(*t));
}

void kt_remap_free(kt_remap_table *t)
{
    if (!t) {
        return;
    }
    free(t->sources);
    free(t->targets);
    t->sources = NULL;
    t->targets = NULL;
    t->count = 0;
    t->capacity = 0;
}

static int ensure_capacity(kt_remap_table *t, size_t need)
{
    if (need <= t->capacity) {
        return 0;
    }
    size_t cap = t->capacity ? t->capacity : 8;
    while (cap < need) {
        cap *= 2;
    }

    kt_key_id *sources = malloc(cap * sizeof(*sources));
    kt_key_id *targets = malloc(cap * sizeof(*targets));
    if (!sources || !targets) {
        free(sources);
        free(targets);
        return -1;
    }

    if (t->count > 0) {
        memcpy(sources, t->sources, t->count * sizeof(*sources));
        memcpy(targets, t->targets, t->count * sizeof(*targets));
    }

    free(t->sources);
    free(t->targets);
    t->sources = sources;
    t->targets = targets;
    t->capacity = cap;
    return 0;
}

int kt_remap_set(kt_remap_table *t, kt_key_id from, kt_key_id to)
{
    if (!t || from == KT_KEY_ID_INVALID || to == KT_KEY_ID_INVALID) {
        return -1;
    }
    for (size_t i = 0; i < t->count; i++) {
        if (t->sources[i] == from) {
            t->targets[i] = to;
            return 0;
        }
    }
    if (ensure_capacity(t, t->count + 1) != 0) {
        return -1;
    }
    t->sources[t->count] = from;
    t->targets[t->count] = to;
    t->count++;
    return 0;
}

kt_key_id kt_remap_lookup(const kt_remap_table *t, kt_key_id from)
{
    if (!t) {
        return KT_KEY_ID_INVALID;
    }
    for (size_t i = 0; i < t->count; i++) {
        if (t->sources[i] == from) {
            return t->targets[i];
        }
    }
    return KT_KEY_ID_INVALID;
}

#ifdef KEYTWEAK_PLATFORM_WIN32

static void send_synthetic_key(uint16_t vk, DWORD extra, BOOL key_up)
{
    INPUT input = {0};
    input.type = INPUT_KEYBOARD;
    input.ki.wVk = vk;
    input.ki.dwFlags = key_up ? KEYEVENTF_KEYUP : 0;
    input.ki.dwExtraInfo = extra;
    SendInput(1, &input, sizeof(input));
}

static LRESULT CALLBACK keyboard_proc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode != HC_ACTION || !g_remap_table) {
        return CallNextHookEx(g_keyboard_hook, nCode, wParam, lParam);
    }

    KBDLLHOOKSTRUCT *kbd = (KBDLLHOOKSTRUCT *)lParam;
    if (!kbd) {
        return CallNextHookEx(g_keyboard_hook, nCode, wParam, lParam);
    }

    kt_key_id src_id = kt_catalog_id_for_vk((uint16_t)kbd->vkCode);
    if (src_id == KT_KEY_ID_INVALID) {
        return CallNextHookEx(g_keyboard_hook, nCode, wParam, lParam);
    }

    kt_key_id dst_id = kt_remap_lookup(g_remap_table, src_id);
    if (dst_id == KT_KEY_ID_INVALID) {
        return CallNextHookEx(g_keyboard_hook, nCode, wParam, lParam);
    }

    uint16_t dst_vk = kt_catalog_vk(dst_id);
    if (dst_vk == 0) {
        return CallNextHookEx(g_keyboard_hook, nCode, wParam, lParam);
    }

    BOOL key_up = (wParam == WM_KEYUP || wParam == WM_SYSKEYUP);
    send_synthetic_key(dst_vk, kbd->dwExtraInfo, key_up);
    return 1;
}

static BOOL WINAPI console_ctrl_handler(DWORD ctrl_type)
{
    if (ctrl_type == CTRL_C_EVENT || ctrl_type == CTRL_BREAK_EVENT || ctrl_type == CTRL_CLOSE_EVENT) {
        PostQuitMessage(0);
        return TRUE;
    }
    return FALSE;
}

int kt_remap_install(const kt_remap_table *t)
{
    if (!t) {
        return -1;
    }
    g_remap_table = t;
    g_keyboard_hook = SetWindowsHookExW(WH_KEYBOARD_LL, keyboard_proc, NULL, 0);
    return g_keyboard_hook ? 0 : -1;
}

void kt_remap_uninstall(void)
{
    if (g_keyboard_hook) {
        UnhookWindowsHookEx(g_keyboard_hook);
        g_keyboard_hook = NULL;
    }
    g_remap_table = NULL;
}

int kt_remap_runloop(void)
{
    if (!SetConsoleCtrlHandler(console_ctrl_handler, TRUE)) {
        return -1;
    }

    MSG msg;
    while (GetMessageW(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return 0;
}

#else

int kt_remap_install(const kt_remap_table *t)
{
    (void)t;
    return -1;
}

void kt_remap_uninstall(void)
{
}

int kt_remap_runloop(void)
{
    return -1;
}

#endif