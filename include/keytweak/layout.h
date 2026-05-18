#ifndef KEYTWEAK_LAYOUT_H
#define KEYTWEAK_LAYOUT_H

#include "keytweak/types.h"

int kt_layout_probe(kt_layout_snapshot *out);
void kt_layout_free(kt_layout_snapshot *snap);

#endif
