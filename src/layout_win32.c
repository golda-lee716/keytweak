#include "keytweak/layout.h"

#include <stdlib.h>

#ifdef KEYTWEAK_PLATFORM_WIN32
#include <windows.h>
#endif

#ifndef KEYTWEAK_PLATFORM_WIN32

int kt_layout_probe(kt_layout_snapshot *out)
{
    (void)out;
    return -1;
}

#else

static int push_key(kt_layout_snapshot *snap, kt_physical_key k)
{
    kt_physical_key *next = realloc(snap->keys, (snap->key_count + 1) * sizeof(*next));
    if (!next) {
        return -1;
    }
    snap->keys = next;
    snap->keys[snap->key_count++] = k;
    return 0;
}

int kt_layout_probe(kt_layout_snapshot *out)
{
    static const struct {
        kt_key_id id;
        uint16_t sc;
        int row;
        int col;
    } scan_table[] = {
        {1, 0x01, 0, 0},     {2, 0x02, 0, 1},     {3, 0x03, 0, 2},     {4, 0x04, 0, 3},
        {5, 0x05, 0, 4},     {6, 0x06, 0, 5},     {7, 0x07, 0, 6},     {8, 0x08, 0, 7},
        {9, 0x09, 0, 8},     {10, 0x0A, 0, 9},    {11, 0x0B, 0, 10},   {12, 0x0C, 0, 11},
        {13, 0x0D, 0, 12},   {14, 0x0E, 0, 13},   {15, 0x0F, 1, 0},     {16, 0x10, 1, 1},
        {17, 0x11, 1, 2},    {18, 0x12, 1, 3},    {19, 0x13, 1, 4},    {20, 0x14, 1, 5},
        {21, 0x15, 1, 6},    {22, 0x16, 1, 7},    {23, 0x17, 1, 8},    {24, 0x18, 1, 9},
        {25, 0x19, 1, 10},   {26, 0x1A, 1, 11},   {27, 0x1B, 1, 12},   {28, 0x1C, 1, 13},
        {29, 0x1D, 2, 0},    {30, 0x1E, 2, 1},    {31, 0x1F, 2, 2},    {32, 0x20, 2, 3},
        {33, 0x21, 2, 4},    {34, 0x22, 2, 5},    {35, 0x23, 2, 6},    {36, 0x24, 2, 7},
        {37, 0x25, 2, 8},    {38, 0x26, 2, 9},    {39, 0x27, 2, 10},   {40, 0x28, 2, 11},
        {41, 0x29, 2, 12},   {42, 0x2A, 3, 0},    {43, 0x2B, 3, 1},    {44, 0x2C, 3, 2},
        {45, 0x2D, 3, 3},    {46, 0x2E, 3, 4},    {47, 0x2F, 3, 5},    {48, 0x30, 3, 6},
        {49, 0x31, 3, 7},    {50, 0x32, 3, 8},    {51, 0x33, 3, 9},    {52, 0x34, 3, 10},
        {53, 0x35, 3, 11},   {54, 0x36, 3, 12},   {55, 0x37, 4, 13},   {56, 0x38, 4, 1},
        {57, 0x39, 4, 5},    {58, 0x3A, 3, 0},     {59, 0x3B, 0, 14},   {60, 0x3C, 0, 15},
        {61, 0x3D, 0, 16},   {62, 0x3E, 0, 17},   {63, 0x3F, 0, 18},   {64, 0x40, 0, 19},
        {65, 0x41, 0, 20},   {66, 0x42, 0, 21},   {67, 0x43, 0, 22},   {68, 0x44, 0, 23},
        {69, 0x45, 4, 14},   {70, 0x46, 4, 15},   {71, 0x47, 4, 16},   {72, 0x48, 4, 17},
        {73, 0x49, 4, 18},   {74, 0x4A, 4, 19},   {75, 0x4B, 4, 20},   {76, 0x4C, 4, 21},
        {77, 0x4D, 4, 22},   {78, 0x4E, 4, 23},   {79, 0x4F, 4, 24},   {80, 0x50, 4, 25},
        {81, 0x51, 4, 26},   {82, 0x52, 4, 27},   {83, 0x53, 4, 28},   {87, 0x57, 0, 24},
        {88, 0x58, 0, 25},
    };

    kt_layout_snapshot snap = {0};

    for (size_t i = 0; i < sizeof(scan_table) / sizeof(scan_table[0]); i++) {
        uint16_t sc = scan_table[i].sc;
        UINT vk = MapVirtualKeyW((UINT)sc, MAPVK_VSC_TO_VK_EX);
        if (vk == 0) {
            vk = MapVirtualKeyW((UINT)sc, MAPVK_VSC_TO_VK);
        }
        kt_physical_key k = {0};
        k.id = scan_table[i].id;
        k.scancode_make = sc;
        k.vk = (uint16_t)vk;
        k.row = scan_table[i].row;
        k.col = scan_table[i].col;
        if (push_key(&snap, k) != 0) {
            kt_layout_free(&snap);
            return -1;
        }
    }

    *out = snap;
    return 0;
}

#endif

void kt_layout_free(kt_layout_snapshot *snap)
{
    if (!snap) {
        return;
    }
    free(snap->keys);
    snap->keys = NULL;
    snap->key_count = 0;
}
