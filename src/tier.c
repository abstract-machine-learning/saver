#include "tier.h"
#include <stdlib.h>


void tier_create(Tier *tier, const unsigned int size) {
    tier->size = size;
    tier->tiers = (unsigned int *) calloc(size*2, sizeof(unsigned int));
    tier->unique_count = 0;
}

void tier_delete(Tier *tier) {
    tier->size = 0;
    free(tier->tiers);
}

/*void tier_resize(Tier *tier, const unsigned int size) {
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
}*/

void tier_print(const Tier tier, FILE *stream) {
    unsigned int i;
    for (i = 0; i < tier.size; ++i) {
        fprintf(stream, "%u: %u\n", i, tier.tiers[i]);
    }
}

void tier_read(
    Tier *tier,
    const char *path,
    const unsigned int space_size
)
{
    tier_create(tier, space_size);
    FILE *tiers_file = fopen(path, "r");

    if (!tiers_file) {
        fprintf(stderr, "[%s: %d] Cannot open tier file \"%s\"\n", __FILE__, __LINE__, path);
        abort();
    }

    for (unsigned int i = 0; i < space_size*2; ++i) 
    {
        fscanf(tiers_file, "%u ", &tier->tiers[i]);
        //printf("%d : %u\n",i,tier->tiers[i]);
        
        if(tier->tiers[i] > tier->unique_count)
        {
            tier->unique_count = tier->tiers[i];
        }
    }

}

unsigned int get_tier_unique_count(const Tier tier)
{
    return tier.unique_count;
}

void fill_isOneHot(bool* isOneHot, const Tier tier)
{
    for(unsigned int i = tier.size; i<tier.size*2;i++)
    {
        
        if(tier.tiers[i] == 0)
            isOneHot[i-tier.size] = false;
        else if(tier.tiers[i] == 1)
            isOneHot[i-tier.size] = true;
        else
        {
            printf("Error Tier format mismatch");
            exit(0);
        }
    }
    /*for(unsigned int i = 0; i<tier.size;i++)
    {

        isOneHot[i] = false;
        for(unsigned int j = 0; j<tier.size;j++)
        {
            if((i != j) && (tier.tiers[i] == tier.tiers[j]))
            {
                isOneHot[i] = true;
                break;
            }
        }

    }*/

}
