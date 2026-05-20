#include "keytweak/remap.h"

#include <stdlib.h>
#include <string.h>

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
