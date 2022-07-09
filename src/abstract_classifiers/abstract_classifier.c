#include "abstract_classifier.h"

#include <malloc.h>
#include <string.h>

#include "../report_error.h"
#include "../type.h"
#include "interval_classifier.h"
#include "raf_classifier.h"
#include "hybrid_classifier.h"


/**
 * Structure of an abstract classifier.
 */
struct abstract_classifier {
    Classifier classifier;              /**< Concrete classifier. */
    AbstractDomainType abstract_domain; /**< Type of abstraction to use. */
    void *abstract_classifier;          /**< Pointer to actual abstract classifier. */
};



AbstractClassifier abstract_classifier_create(
    const Classifier classifier,
    const AbstractDomainType abstract_domain
) {
    AbstractClassifier abstract_classifier = (AbstractClassifier) malloc(sizeof(struct abstract_classifier));
    if (!abstract_classifier) {
        report_error("Cannot allocate memory.");
    }

    abstract_classifier->classifier = classifier;
    abstract_classifier->abstract_domain = abstract_domain;
    switch (abstract_domain) {
        case ABSTRACT_DOMAIN_INTERVAL:
            abstract_classifier->abstract_classifier = (void *) interval_classifier_create(classifier);
            break;
        case ABSTRACT_DOMAIN_RAF:
            abstract_classifier->abstract_classifier = (void *) raf_classifier_create(classifier);
            break;
        case ABSTRACT_DOMAIN_HYBRID:
            abstract_classifier->abstract_classifier = (void *) hybrid_classifier_create(classifier);
            break;
        default:
            report_error("Unsupported abstract domain.");
    }

    return abstract_classifier;
}



void abstract_classifier_delete(AbstractClassifier *abstract_classifier) {
    if (abstract_classifier == NULL || *abstract_classifier == NULL) {
        return;
    }

    switch ((*abstract_classifier)->abstract_domain) {
        case ABSTRACT_DOMAIN_INTERVAL:
            interval_classifier_delete((IntervalClassifier *) &(*abstract_classifier)->abstract_classifier);
            break;
        case ABSTRACT_DOMAIN_RAF:
            raf_classifier_delete((RafClassifier *) &(*abstract_classifier)->abstract_classifier);
            break;
        case ABSTRACT_DOMAIN_HYBRID:
            hybrid_classifier_delete((HybridClassifier *) &(*abstract_classifier)->abstract_classifier);
            break;
        default:
            report_error("Unsupported abstract domain.");
    }

    free(*abstract_classifier);
    *abstract_classifier = NULL;
}



AbstractClassifier abstract_classifier_read(
    const Classifier classifier,
    const int argc,
    const char *argv[]
) {
    const char *abstraction = (argc > 0) ? argv[0] : DEFAULT_ABSTRACTION;

    if (strcmp(abstraction, "interval") == 0) {
        return abstract_classifier_create(classifier, ABSTRACT_DOMAIN_INTERVAL);
    }
    else if (strcmp(abstraction, "raf") == 0) {
        return abstract_classifier_create(classifier, ABSTRACT_DOMAIN_RAF);
    }
    else if (strcmp(abstraction, "hybrid") == 0) {
        return abstract_classifier_create(classifier, ABSTRACT_DOMAIN_HYBRID);
    }
    else {
        report_error("Unrecognized type of abstraction.");
    }
}



Classifier abstract_classifier_get_classifier(
    const AbstractClassifier abstract_classifier
) {
    return abstract_classifier->classifier;
}



void *abstract_classifier_score(
    const AbstractClassifier abstract_classifier,
    const AdversarialRegion adversarial_region
) {
    switch (abstract_classifier->abstract_domain) {
        case ABSTRACT_DOMAIN_INTERVAL:
            return interval_classifier_score(
                (IntervalClassifier) abstract_classifier->abstract_classifier,
                adversarial_region
            );
        case ABSTRACT_DOMAIN_RAF:
            return raf_classifier_score(
                (RafClassifier) abstract_classifier->abstract_classifier,
                adversarial_region
            );
        case ABSTRACT_DOMAIN_HYBRID:
            return hybrid_classifier_score(
                (HybridClassifier) abstract_classifier->abstract_classifier,
                adversarial_region
            );
        default:
            report_error("Unsupported abstract domain.");
    }
}


unsigned int abstract_classifier_classify(
    const AbstractClassifier abstract_classifier,
    const AdversarialRegion adversarial_region,
    char **classes,
    bool* fair_opt,
    unsigned int* has_counterexample
) {
    switch (abstract_classifier->abstract_domain) {
        case ABSTRACT_DOMAIN_INTERVAL:
            return interval_classifier_classify(
                (IntervalClassifier) abstract_classifier->abstract_classifier,
                adversarial_region,
                classes,
                fair_opt
            );
        case ABSTRACT_DOMAIN_RAF:
            return raf_classifier_classify(
                (RafClassifier) abstract_classifier->abstract_classifier,
                adversarial_region,
                classes,
                fair_opt,
                has_counterexample
            );
        case ABSTRACT_DOMAIN_HYBRID:
            return hybrid_classifier_classify(
                (HybridClassifier) abstract_classifier->abstract_classifier,
                adversarial_region,
                classes
            );
        default:
            report_error("Unsupported abstract domain.");
    }
}
