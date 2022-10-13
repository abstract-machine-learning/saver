/**
 * A genetic algorithm to generate robustness counterexamples.
 *
 * @file robustness_genetic_algorithm.h
 * 
 */
#ifndef ROBUSTNESS_GENETIC_ALGORITHM_H
#define ROBUSTNESS_GENERIC_ALGORITHM_H

#include "counterexample.h"
#include "../classifier.h"
#include "../adversarial_region.h"


/**
 * Searches for a robustness counterexample by applying a genetic algorithm.
 *
 * @param[out] counterexample Counterexample, if found
 * @param[in] classifier Classifier
 * @param[in] adversarial_region Adversarial region
 * @return 1 if counterexample is found, 0 otherwise
 */
unsigned int robustness_genetic_algorithm(
    Counterexample counterexample,
    const Classifier classifier,
    const AdversarialRegion adversarial_region
);

#endif
