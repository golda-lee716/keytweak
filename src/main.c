#include <stdio.h>
#include <stdlib.h>

#include "keytweak/app.h"
#include "keytweak/catalog.h"
#include "keytweak/layout.h"
#include "keytweak/remap.h"

static void print_layout_summary(const kt_layout_snapshot *snap)
{
    printf("Detected keys: %zu\n", snap->key_count);
    for (size_t i = 0; i < snap->key_count && i < 8; i++) {
        const kt_physical_key *k = &snap->keys[i];
        const char *label = kt_catalog_label(k->id);
        printf("  [%zu] %s  sc=0x%04X  vk=0x%04X  row=%d col=%d\n",
               i, label, (unsigned)k->scancode_make, (unsigned)k->vk, k->row, k->col);
    }
    if (snap->key_count > 8) {
        printf("  ...\n");
    }
}

int main(void)
{
    printf("%s %s — layout probe + remap scaffold\n", KEYTWEAK_NAME, KEYTWEAK_VERSION);

    kt_layout_snapshot layout = {0};
    if (kt_layout_probe(&layout) != 0) {
        fprintf(stderr, "layout probe failed\n");
        return 1;
    }

    print_layout_summary(&layout);

    kt_remap_table remaps = {0};
    kt_remap_init(&remaps);
    if (layout.key_count >= 2) {
        kt_key_id a = layout.keys[0].id;
        kt_key_id b = layout.keys[1].id;
        kt_remap_set(&remaps, a, b);
        printf("Example remap: %s -> %s\n", kt_catalog_label(a), kt_catalog_label(b));
    }

    kt_remap_free(&remaps);
    kt_layout_free(&layout);
    return 0;
}
