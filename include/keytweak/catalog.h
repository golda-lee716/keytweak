#ifndef KEYTWEAK_CATALOG_H
#define KEYTWEAK_CATALOG_H

#include "keytweak/types.h"

const char *kt_catalog_label(kt_key_id id);
kt_key_id kt_catalog_id(const char *label);

#endif
