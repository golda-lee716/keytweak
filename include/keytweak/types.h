#ifndef KEYTWEAK_TYPES_H
#define KEYTWEAK_TYPES_H

#include <stddef.h>
#include <stdint.h>

typedef uint32_t kt_key_id;

#define KT_KEY_ID_INVALID ((kt_key_id)0)

typedef struct kt_physical_key {
    kt_key_id id;
    uint16_t scancode_make;
    uint16_t vk;
    int row;
    int col;
} kt_physical_key;

typedef struct kt_layout_snapshot {
    kt_physical_key *keys;
    size_t key_count;
} kt_layout_snapshot;

#endif
