#ifndef TIER_H
#define TIER_H

#include <stdio.h>
#include <stdbool.h>

typedef struct tier Tier;

struct tier {
    unsigned int *tiers;
    unsigned bool *isOH;
    unsigned int size;
    unsigned int unique_count;
};

void tier_create(Tier *tier, const unsigned int size);
void tier_delete(Tier *tier);
void tier_resize(Tier *tier, const unsigned int size);
void tier_print(const Tier tier, FILE *stream);
void tier_read(Tier *tier,const char *path,const unsigned int space_size);
void fill_isOneHot(bool* isOneHot, const Tier tier);

#endif
