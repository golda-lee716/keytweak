#ifndef KEYTWEAK_REMAP_H
#define KEYTWEAK_REMAP_H

#include "keytweak/types.h"

typedef struct kt_remap_table {
    kt_key_id *sources;
    kt_key_id *targets;
    size_t count;
    size_t capacity;
} kt_remap_table;

void kt_remap_init(kt_remap_table *t);
void kt_remap_free(kt_remap_table *t);

int kt_remap_set(kt_remap_table *t, kt_key_id from, kt_key_id to);
kt_key_id kt_remap_lookup(const kt_remap_table *t, kt_key_id from);

int kt_remap_install(const kt_remap_table *t);
void kt_remap_uninstall(void);
int kt_remap_runloop(void);

#endif
