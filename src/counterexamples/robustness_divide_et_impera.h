/**
 * Recursively divides an adversarial region to find counterexamples
 *
 * @file robustness_divide_et_impera.h
 * @author Marco Zanella <marco.zanella.1991@gmail.com>
 */
#ifndef ROBSTNESS_DIVIDE_ET_IMPERA_H
#define ROBSTNESS_DIVIDE_ET_IMPERA_H

#include "counterexample.h"
#include "../abstract_classifiers/abstract_classifier.h"
#include "../adversarial_region.h"

/**
 * Searches for a robustness counterexample by recursive division of adversarial region.
 *
 * @param[out] counterexample Counterexample, if found
 * @param[in] abstract_classifier Abstract classifier
 * @param[in] adversarial_region Adversial region
 * @return 1 if counterexample is found, 0 otherwise
 */
unsigned int robustness_divide_et_impera(
    Counterexample counterexample,
    const AbstractClassifier abstract_classifier,
    const AdversarialRegion adversarial_region
);

#endif
