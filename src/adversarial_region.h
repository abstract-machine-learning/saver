/**
 * Adversarial region.
 * 
 * Defines the structure of an adversarial region.
 *
 * @file adversarial_region.h
 */
#ifndef ADVERSARIAL_REGION_H
#define ADVERSARIAL_REGION_H

#include "type.h"
#include "perturbation.h"


/**
 * Structure of an adversarial region.
 */
struct adversarial_region {
    const Real *sample;               /**< Originator of the region. */
    const Perturbation perturbation;  /**< Perturbation function. */
};


/** Type of an adversarial region. */
typedef struct adversarial_region AdversarialRegion;


#endif
