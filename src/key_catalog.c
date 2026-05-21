#include "keytweak/catalog.h"

#include <ctype.h>
#include <string.h>

#ifdef KEYTWEAK_PLATFORM_WIN32
#include <windows.h>
#endif

static const struct {
    kt_key_id id;
    uint16_t vk;
    const char *label;
} k_labels[] = {
    {1,  VK_ESCAPE,      "Esc"},
    {2,  '1',            "1"},
    {3,  '2',            "2"},
    {4,  '3',            "3"},
    {5,  '4',            "4"},
    {6,  '5',            "5"},
    {7,  '6',            "6"},
    {8,  '7',            "7"},
    {9,  '8',            "8"},
    {10, '9',            "9"},
    {11, '0',            "0"},
    {12, VK_OEM_MINUS,   "-"},
    {13, VK_OEM_PLUS,    "="},
    {14, VK_BACK,        "Backspace"},
    {15, VK_TAB,         "Tab"},
    {16, 'Q',            "Q"},
    {17, 'W',            "W"},
    {18, 'E',            "E"},
    {19, 'R',            "R"},
    {20, 'T',            "T"},
    {21, 'Y',            "Y"},
    {22, 'U',            "U"},
    {23, 'I',            "I"},
    {24, 'O',            "O"},
    {25, 'P',            "P"},
    {26, VK_OEM_4,       "["},
    {27, VK_OEM_6,       "]"},
    {28, VK_RETURN,      "Enter"},
    {29, VK_LCONTROL,    "L-Ctrl"},
    {30, 'A',            "A"},
    {31, 'S',            "S"},
    {32, 'D',            "D"},
    {33, 'F',            "F"},
    {34, 'G',            "G"},
    {35, 'H',            "H"},
    {36, 'J',            "J"},
    {37, 'K',            "K"},
    {38, 'L',            "L"},
    {39, VK_OEM_1,       ";"},
    {40, VK_OEM_7,       "'"},
    {41, VK_OEM_3,       "`"},
    {42, VK_LSHIFT,      "L-Shift"},
    {43, VK_OEM_5,       "\\"},
    {44, 'Z',            "Z"},
    {45, 'X',            "X"},
    {46, 'C',            "C"},
    {47, 'V',            "V"},
    {48, 'B',            "B"},
    {49, 'N',            "N"},
    {50, 'M',            "M"},
    {51, VK_OEM_COMMA,   ","},
    {52, VK_OEM_PERIOD,  "."},
    {53, VK_OEM_2,       "/"},
    {54, VK_RSHIFT,      "R-Shift"},
    {55, VK_MULTIPLY,    "Num *"},
    {56, VK_LMENU,       "L-Alt"},
    {57, VK_SPACE,       "Space"},
    {58, VK_CAPITAL,     "Caps"},
    {59, VK_F1,          "F1"},
    {60, VK_F2,          "F2"},
    {61, VK_F3,          "F3"},
    {62, VK_F4,          "F4"},
    {63, VK_F5,          "F5"},
    {64, VK_F6,          "F6"},
    {65, VK_F7,          "F7"},
    {66, VK_F8,          "F8"},
    {67, VK_F9,          "F9"},
    {68, VK_F10,         "F10"},
    {69, VK_NUMLOCK,     "Num Lock"},
    {70, VK_SCROLL,      "Scroll Lock"},
    {71, VK_NUMPAD7,     "Num 7"},
    {72, VK_NUMPAD8,     "Num 8"},
    {73, VK_NUMPAD9,     "Num 9"},
    {74, VK_SUBTRACT,    "Num -"},
    {75, VK_NUMPAD4,     "Num 4"},
    {76, VK_NUMPAD5,     "Num 5"},
    {77, VK_NUMPAD6,     "Num 6"},
    {78, VK_ADD,         "Num +"},
    {79, VK_NUMPAD1,     "Num 1"},
    {80, VK_NUMPAD2,     "Num 2"},
    {81, VK_NUMPAD3,     "Num 3"},
    {82, VK_NUMPAD0,     "Num 0"},
    {83, VK_DECIMAL,     "Num ."},
    {87, VK_F11,         "F11"},
    {88, VK_F12,         "F12"},
};

static int label_equal(const char *a, const char *b)
{
    if (!a || !b) {
        return 0;
    }
    while (*a && *b) {
        unsigned char ca = (unsigned char) tolower((unsigned char) *a);
        unsigned char cb = (unsigned char) tolower((unsigned char) *b);
        if (ca != cb) {
            return 0;
        }
        a++;
        b++;
    }
    return *a == '\0' && *b == '\0';
}

static const char *normalize_label(const char *label)
{
    if (label_equal(label, "CapsLock")) {
        return "Caps";
    }
    if (label_equal(label, "Ctrl")) {
        return "L-Ctrl";
    }
    if (label_equal(label, "Control")) {
        return "L-Ctrl";
    }
    if (label_equal(label, "Shift")) {
        return "L-Shift";
    }
    if (label_equal(label, "Alt")) {
        return "L-Alt";
    }
    if (label_equal(label, "Return")) {
        return "Enter";
    }
    if (label_equal(label, "Spacebar")) {
        return "Space";
    }
    return label;
}

const char *kt_catalog_label(kt_key_id id)
{
    for (size_t i = 0; i < sizeof(k_labels) / sizeof(k_labels[0]); i++) {
        if (k_labels[i].id == id) {
            return k_labels[i].label;
        }
    }
    return "?";
}

kt_key_id kt_catalog_id(const char *label)
{
    if (!label) {
        return KT_KEY_ID_INVALID;
    }

    label = normalize_label(label);
    for (size_t i = 0; i < sizeof(k_labels) / sizeof(k_labels[0]); i++) {
        if (label_equal(k_labels[i].label, label)) {
            return k_labels[i].id;
        }
    }
    return KT_KEY_ID_INVALID;
}

kt_key_id kt_catalog_id_for_vk(uint16_t vk)
{
    for (size_t i = 0; i < sizeof(k_labels) / sizeof(k_labels[0]); i++) {
        if (k_labels[i].vk == vk) {
            return k_labels[i].id;
        }
    }
    return KT_KEY_ID_INVALID;
}

uint16_t kt_catalog_vk(kt_key_id id)
{
    for (size_t i = 0; i < sizeof(k_labels) / sizeof(k_labels[0]); i++) {
        if (k_labels[i].id == id) {
            return k_labels[i].vk;
        }
    }
    return 0;
}
