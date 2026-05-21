#ifndef KEYTWEAK_CATALOG_H
#define KEYTWEAK_CATALOG_H

#include "keytweak/types.h"
#include <stdint.h>

const char *kt_catalog_label(kt_key_id id);
kt_key_id kt_catalog_id(const char *label);
kt_key_id kt_catalog_id_for_vk(uint16_t vk);
uint16_t kt_catalog_vk(kt_key_id id);

#endif
