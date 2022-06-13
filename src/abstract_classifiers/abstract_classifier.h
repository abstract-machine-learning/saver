/**
 * Abstract version of an SVM classifier.
 * 
 * Defines the abstract version of an SVM classifier.
 * 
 * @file abstract_classifier.h
 * @author Marco Zanella <marco.zanella.1991@gmail.com>
 */
#ifndef ABSTRACT_CLASSIFIER_H
#define ABSTRACT_CLASSIFIER_H

#include "../classifier.h"
#include "../adversarial_region.h"
#include "../abstract_domains/abstract_domain.h"
#include "../perturbation.h"
#include "../tier.h"


/** Default abstraction name. */
#define DEFAULT_ABSTRACTION "interval"


/** Type of an abstract classifier. */
typedef struct abstract_classifier *AbstractClassifier;


/**
 * Creates an abstract classifier.
 * 
 * Uses data of a (concrete) classifier to create its abstract version,
 * given a type of abstract domain.
 * 
 * @param[in] classifier Concrete classifier to abstract
 * @param[in] abstract_domain Abstraction to use
 * @return Abstract classifier
 * @note #abstract_classifier_delete should be called to ensure proper memory deallocation.
 */
AbstractClassifier abstract_classifier_create(
    const Classifier classifier,
    const AbstractDomainType abstract_domain
);


/**
 * Deletes an abstract classifier.
 * 
 * @param[out] abstract_classifier Pointer to abstract classifier to delete
 */
void abstract_classifier_delete(AbstractClassifier *abstract_classifier);


/**
 * Creates an abstract classifier reading form argument vector.
 *
 * @param[in] classifier Concrete classifier to abstract
 * @param[in] argc ARGument Counter
 * @param[in] argv ARGument Vector
 * @return Abstract classifier
 * @note This function is intended as a shorthand to read parameters
 * from command line.
 */
AbstractClassifier abstract_classifier_read(
    const Classifier classifier,
    const int argc,
    const char *argv[]
);


/**
 * Returns concrete classifier.
 *
 * @param[in] abstract_classifier Abstract classifier
 * @return Concrete classifier
 */
Classifier abstract_classifier_get_classifier(
    const AbstractClassifier abstract_classifier
);


/**
 * Computes decision function of an abstract sample.
 *
 * Type and nature of the result depend on the type of classifier
 * (binary, OVO, OVR...) and on the abstraction.
 *
 * @param[in] abstract_classifier Abstract classifier to use
 * @param[in] adversarial_region  Sample to compute decision function on
 * @return Pointer to a memory area containing the results (depends on
 *         classifier and abstract domain)
 */
void *abstract_classifier_score(
    const AbstractClassifier abstract_classifier,
    const AdversarialRegion adversarial_region
);


/**
 * Classifies an abstract sample.
 * 
 * Abstract classification returns a superset containing at least the
 * classes the concrete classifier would return if run on every
 * point in \f$\gamma(abstract\_sample)\f$ (soundness).
 * 
 * @param[in]  abstract_classifier Abstract classifier to user
 * @param[in]  adversarial_region  Sample to classify
 * @param[out] classes             Previously allocated array of strings,
 *                                 will be filled with classes chosen by classifier
 * @return Number of chosen classes
 */
unsigned int abstract_classifier_classify(
    const AbstractClassifier abstract_classifier,
    const AdversarialRegion adversarial_region,
    char **classes,
    bool isTop
);


#endif
