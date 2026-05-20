#include "keytweak/catalog.h"

#include <string.h>

static const struct {
    kt_key_id id;
    const char *label;
} k_labels[] = {
    {1, "Esc"},
    {2, "1"},
    {3, "2"},
    {4, "3"},
    {5, "4"},
    {6, "5"},
    {7, "6"},
    {8, "7"},
    {9, "8"},
    {10, "9"},
    {11, "0"},
    {12, "-"},
    {13, "="},
    {14, "Backspace"},
    {15, "Tab"},
    {16, "Q"},
    {17, "W"},
    {18, "E"},
    {19, "R"},
    {20, "T"},
    {21, "Y"},
    {22, "U"},
    {23, "I"},
    {24, "O"},
    {25, "P"},
    {26, "["},
    {27, "]"},
    {28, "Enter"},
    {29, "L-Ctrl"},
    {30, "A"},
    {31, "S"},
    {32, "D"},
    {33, "F"},
    {34, "G"},
    {35, "H"},
    {36, "J"},
    {37, "K"},
    {38, "L"},
    {39, ";"},
    {40, "'"},
    {41, "`"},
    {42, "L-Shift"},
    {43, "\\"},
    {44, "Z"},
    {45, "X"},
    {46, "C"},
    {47, "V"},
    {48, "B"},
    {49, "N"},
    {50, "M"},
    {51, ","},
    {52, "."},
    {53, "/"},
    {54, "R-Shift"},
    {55, "Num *"},
    {56, "L-Alt"},
    {57, "Space"},
    {58, "Caps"},
    {59, "F1"},
    {60, "F2"},
    {61, "F3"},
    {62, "F4"},
    {63, "F5"},
    {64, "F6"},
    {65, "F7"},
    {66, "F8"},
    {67, "F9"},
    {68, "F10"},
    {69, "Num Lock"},
    {70, "Scroll Lock"},
    {71, "Num 7"},
    {72, "Num 8"},
    {73, "Num 9"},
    {74, "Num -"},
    {75, "Num 4"},
    {76, "Num 5"},
    {77, "Num 6"},
    {78, "Num +"},
    {79, "Num 1"},
    {80, "Num 2"},
    {81, "Num 3"},
    {82, "Num 0"},
    {83, "Num ."},
    {87, "F11"},
    {88, "F12"},
};

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

    for (size_t i = 0; i < sizeof(k_labels) / sizeof(k_labels[0]); i++) {
        if (strcmp(k_labels[i].label, label) == 0) {
            return k_labels[i].id;
        }
    }
    return KT_KEY_ID_INVALID;
}
