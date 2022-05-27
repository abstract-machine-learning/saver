/**
 * Adversarial region.
 * 
 * Defines the structure of an adversarial region.
 *
 * @file adversarial_region.h
 * @author Marco Zanella <marco.zanella.1991@gmail.com>
 */
#ifndef ADVERSARIAL_REGION_H
#define ADVERSARIAL_REGION_H

#include <stdio.h>

#include "type.h"
#include "perturbation.h"
#include "tier.h"

/**
 * Structure of an adversarial region.
 */
struct adversarial_region {
    const Real *sample;               /**< Originator of the region. */
    const Perturbation perturbation;  /**< Perturbation function. */
    const Tier tier;
};


/** Type of an adversarial region. */
typedef struct adversarial_region AdversarialRegion;


/** Type of a property of an adversarial region. */
typedef unsigned int (*AdversarialRegionProperty)(const AdversarialRegion, void *);



/**
 * Tells whether an adversarial region contains a point.
 *
 * @param[in] adversarial_region Adversarial region
 * @param[in] point Point to check
 * @return 1 if point lies within adversarial region, 0 otherwise
 */
unsigned int adversarial_region_contains(
    const AdversarialRegion adversarial_region,
    const Real *point
);


/**
 * Returns a random point in the adversarial region.
 *
 * @param[out] sample Random point
 * @param[in] adversarial_region Adversarial region
 */
void adversarial_region_get_sample(
    Real *sample,
    const AdversarialRegion adversarial_region
);


/**
 * Prints an adversarial region.
 *
 * @param[in] adversarial_region Adversarial region
 * @param[out] stream Stream
 */
void adversarial_region_print(
    const AdversarialRegion adversarial_region,
    FILE *stream
);


#endif
