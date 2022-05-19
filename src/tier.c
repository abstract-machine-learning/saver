#include "tier.h"
#include <stdlib.h>

void tier_create(Tier *tier, const unsigned int size) {
    tier->size = size;
    tier->tiers = (unsigned int *) calloc(size, sizeof(unsigned int));
}

void tier_delete(Tier *tier) {
    tier->size = 0;
    free(tier->tiers);
}

void tier_resize(Tier *tier, const unsigned int size) {
    if (size == tier->size) {
        return;
    }

    if (tier->size == 0) {
        tier_create(tier, size);
        return;
    }

    tier->tiers = realloc(tier->tiers, size * sizeof(unsigned int));
    for (; tier->size < size; ++tier->size) {
        tier->tiers[tier->size] = 0;
    }
    tier->size = size;
}

void tier_print(const Tier tier, FILE *stream) {
    unsigned int i;
    for (i = 0; i < tier.size; ++i) {
        fprintf(stream, "%u: %u\n", i, tier.tiers[i]);
    }
}

