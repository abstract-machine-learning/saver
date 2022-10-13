/**
 * Tool for automatic counterexample generation.
 *
 * Automatically searches for counterexamples.
 *
 * @file counterexample_seeker.h
 * 
 */
#ifndef COUNTEREXAMPLE_SEEKER_H
#define COUNTEREXAMPLE_SEEKER_H

#include <stdio.h>

#include "../abstract_classifiers/abstract_classifier.h"
#include "../adversarial_region.h"
#include "counterexample.h"


/**
 * Available types of counterexamples.
 */
typedef enum {
    COUNTEREXAMPLE_ROBUSTNESS  /**< Searches for counterexamples about robustness. */
} CounterExampleType;


/**
 * Possible outcomes of a counterexample seeker.
 */
typedef enum {
    COUNTEREXAMPLE_SEEKER_ROBUST,          /**< Robustness was proved. */
    COUNTEREXAMPLE_SEEKER_COUNTEREXAMPLE,  /**< A counterexample was found. */
    COUNTEREXAMPLE_SEEKER_DONT_KNOW        /**< Seeker failed. */
} CounterExampleSeekerOutput;


/** Type of a counterexample seeker. */
typedef struct counterexample_seeker *CounterexampleSeeker;


/**
 * Creates a counterexample seeker.
 *
 * @param[out] counterexample_seeker Pointer to counterexample seeker to create
 * @param[in] abstract_classifier Abstract classifier to consider
 * @param[in] type Type of counterexamples
 * @note #counterexample_seeker_delete should be called to ensure proper memory deallocation.
 */
void counterexample_seeker_create(
    CounterexampleSeeker *counterexample_seeker,
    const AbstractClassifier abstract_classifier,
    const CounterExampleType type
);


/**
 * Deletes a counterexample seeker.
 *
 * @param[out] counterexample_seeker Pointer to counterexample seeker to delete
 */
void counterexample_seeker_delete(CounterexampleSeeker *counterexample_seeker);


/**
 * Searches for a counterexample.
 *
 * Counterexample is written to the seeker's stream as a pair of samples.
 *
 * @param[out] counterexample Counterexample to generate
 * @param[in] counterexample_seeker Counterexample seeker
 * @param[in] adversarial_region Adversarial region to search counterexamples in
 * @return 1 if a counterexample is found, 0 otherwise
 */
unsigned int counterexample_seeker_search(
    Counterexample counterexample,
    const CounterexampleSeeker counterexample_seeker,
    const AdversarialRegion adversarial_region
);

#endif
