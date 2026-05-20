#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "keytweak/app.h"
#include "keytweak/catalog.h"
#include "keytweak/layout.h"
#include "keytweak/remap.h"

static void print_usage(const char *prog)
{
    printf("Usage: %s [--list] [--help] [--remap <from>=<to>...]\n", prog);
    printf("Example: %s --remap Caps=Esc --remap F1=F12\n", prog);
}

static void print_layout_summary(const kt_layout_snapshot *snap)
{
    printf("Detected keys: %zu\n", snap->key_count);
    for (size_t i = 0; i < snap->key_count && i < 16; i++) {
        const kt_physical_key *k = &snap->keys[i];
        const char *label = kt_catalog_label(k->id);
        printf("  [%zu] %s  sc=0x%04X  vk=0x%04X  row=%d col=%d\n",
               i, label, (unsigned)k->scancode_make, (unsigned)k->vk, k->row, k->col);
    }
    if (snap->key_count > 16) {
        printf("  ...\n");
    }
}

static int parse_remap_arg(const char *arg, kt_key_id *out_from, kt_key_id *out_to)
{
    const char *eq = strchr(arg, '=');
    if (!eq || eq == arg || eq[1] == '\0') {
        return -1;
    }

    char from_label[64];
    char to_label[64];
    size_t from_len = (size_t)(eq - arg);
    if (from_len >= sizeof(from_label)) {
        return -1;
    }

    memcpy(from_label, arg, from_len);
    from_label[from_len] = '\0';
    if (strlen(eq + 1) >= sizeof(to_label)) {
        return -1;
    }
    strcpy(to_label, eq + 1);

    kt_key_id from = kt_catalog_id(from_label);
    kt_key_id to = kt_catalog_id(to_label);
    if (from == KT_KEY_ID_INVALID || to == KT_KEY_ID_INVALID) {
        return -1;
    }

    *out_from = from;
    *out_to = to;
    return 0;
}

int main(int argc, char **argv)
{
    bool list_only = false;
    kt_remap_table remaps = {0};
    kt_remap_init(&remaps);

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            print_usage(argv[0]);
            kt_remap_free(&remaps);
            return 0;
        }

        if (strcmp(argv[i], "--list") == 0) {
            list_only = true;
            continue;
        }

        if (strcmp(argv[i], "--remap") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "Missing remap target after --remap\n");
                kt_remap_free(&remaps);
                return 1;
            }
            kt_key_id from, to;
            if (parse_remap_arg(argv[++i], &from, &to) != 0 || kt_remap_set(&remaps, from, to) != 0) {
                fprintf(stderr, "Invalid remap argument: %s\n", argv[i]);
                kt_remap_free(&remaps);
                return 1;
            }
            continue;
        }

        if (strncmp(argv[i], "--remap=", 8) == 0) {
            kt_key_id from, to;
            if (parse_remap_arg(argv[i] + 8, &from, &to) != 0 || kt_remap_set(&remaps, from, to) != 0) {
                fprintf(stderr, "Invalid remap argument: %s\n", argv[i]);
                kt_remap_free(&remaps);
                return 1;
            }
            continue;
        }

        fprintf(stderr, "Unknown option: %s\n", argv[i]);
        print_usage(argv[0]);
        kt_remap_free(&remaps);
        return 1;
    }

    printf("%s %s — layout probe + remap scaffold\n", KEYTWEAK_NAME, KEYTWEAK_VERSION);

    kt_layout_snapshot layout = {0};
    if (kt_layout_probe(&layout) != 0) {
        fprintf(stderr, "layout probe failed\n");
        kt_remap_free(&remaps);
        return 1;
    }

    print_layout_summary(&layout);

    if (list_only) {
        kt_remap_free(&remaps);
        kt_layout_free(&layout);
        return 0;
    }

    if (remaps.count == 0 && layout.key_count >= 2) {
        kt_key_id a = layout.keys[0].id;
        kt_key_id b = layout.keys[1].id;
        kt_remap_set(&remaps, a, b);
        printf("Example remap: %s -> %s\n", kt_catalog_label(a), kt_catalog_label(b));
    }

    for (size_t i = 0; i < remaps.count; i++) {
        printf("Remap %s -> %s\n", kt_catalog_label(remaps.sources[i]), kt_catalog_label(remaps.targets[i]));
    }

    kt_remap_free(&remaps);
    kt_layout_free(&layout);
    return 0;
}
