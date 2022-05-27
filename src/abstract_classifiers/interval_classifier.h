/**
 * Interval abstract classifier.
 *
 * Performs an interval analysis on a classifier.
 *
 * @file interval_classifier.h
 * @author Marco Zanella <marco.zanella.1991@gmail.com>
 */
#ifndef INTERVAL_CLASSIFIER_H
#define INTERVAL_CLASSIFIER_H

#include "abstract_classifier.h"
#include "../abstract_domains/interval.h"
#include "../classifier.h"
#include "../tier.h"
 
/** Type of an interval classifier. */
typedef struct interval_classifier *IntervalClassifier;


/**
 * Creates an interval classifier.
 *
 * @param[in] classifier Concrete classifier
 * @return Interval classifier
 * @note #interval_classifier_delete must be called to ensure proper memory
 *       deallocation
 */
IntervalClassifier interval_classifier_create(const Classifier classifier);


/**
 * Deletes an interval classifier.
 *
 * @param[out] interval_classifier Interval classifier
 */
void interval_classifier_delete(IntervalClassifier *interval_classifier);


/**
 * Computes scores on an adversarial_region.
 *
 * @param[in] interval_classifier Interval classifier
 * @param[in] adversarial_region Adversarial region
 * @return Array of scores, each one being an #Interval
 */
Interval *interval_classifier_score(
    const IntervalClassifier interval_classifier,
    const AdversarialRegion adversarial_region
);


/**
 * Computes possible classes of an adversarial region.
 *
 * @param[in] interval_classifier Interval classifier
 * @param[in] adversarial_region Adversarial region
 * @param[out] classes Array of class names
 * @return Number of possible classes
 */
unsigned int interval_classifier_classify(
    const IntervalClassifier interval_classifier,
    const AdversarialRegion adversarial_region,
    char **classes
);

#endif
