#ifndef TIER_H
#define TIER_H

#include <stdio.h>

typedef struct tier Tier;

struct tier {
    unsigned int *tiers;
    unsigned int size;
};

void tier_create(Tier *tier, const unsigned int size);
void tier_delete(Tier *tier);
void tier_resize(Tier *tier, const unsigned int size);
void tier_print(const Tier tier, FILE *stream);

#endif
