#include "hybrid_classifier.h"

#include <malloc.h>
#include "../report_error.h"
#include "interval_classifier.h"
#include "raf_classifier.h"

/**
 * Structure of a hybrid classifier.
 */
struct hybrid_classifier {
    Classifier classifier;                   /**< Concrete classifier. */
    IntervalClassifier interval_classifier;  /**< Interval classifier. */
    RafClassifier raf_classifier;            /**< RAF classifier. */
    Interval *buffer;                        /**< Internal buffer. */
};



static Interval *hybrid_classifier_ovo_score(
    const HybridClassifier hybrid_classifier,
    const AdversarialRegion adversarial_region
) {
    long offset = 0;
    if (perturbation_get_type(adversarial_region.perturbation) == PERTURBATION_FROM_FILE) {
        offset = ftell(perturbation_get_file_stream(adversarial_region.perturbation));
    }
    const Interval *interval_scores = interval_classifier_score(hybrid_classifier->interval_classifier, adversarial_region);
    if (perturbation_get_type(adversarial_region.perturbation) == PERTURBATION_FROM_FILE) {
        fseek(perturbation_get_file_stream(adversarial_region.perturbation), offset, SEEK_SET);
    }
    const Interval *raf_scores = raf_classifier_score(hybrid_classifier->raf_classifier, adversarial_region);
    const unsigned int N = classifier_get_n_classes(hybrid_classifier->classifier);
    unsigned int i, j;

    for (i = 0; i < N; ++i) {
        for (j = i + 1; j < N; ++j) {
            const unsigned int index = i * (N - 1) - (i * (i + 1)) / 2 + j - 1;
            hybrid_classifier->buffer[index].l = max(interval_scores[index].l, raf_scores[index].l);
            hybrid_classifier->buffer[index].u = min(interval_scores[index].u, raf_scores[index].u);
        }
    }

    return hybrid_classifier->buffer;
}



static unsigned int hybrid_classifier_ovo_classify(
    const HybridClassifier hybrid_classifier,
    const AdversarialRegion adversarial_region,
    char **classes
) {
    const Classifier classifier = hybrid_classifier->classifier;
    const Interval *scores = hybrid_classifier_ovo_score(hybrid_classifier, adversarial_region);
    unsigned int *votes, i, j, max_class = 0, winning_classes = 0;
    const unsigned int N = classifier_get_n_classes(classifier);

    votes = (unsigned int *) calloc(N * (N + 1) / 2, sizeof(unsigned int));
    if (votes == NULL) {
        report_error("Cannot allocate memory.");
    }

    for (i = 0; i < N; ++i) {
        for (j = i + 1; j < N; ++j) {
            const unsigned int index = i * (N - 1) - (i * (i + 1)) / 2 + j - 1;
            if (scores[index].l >= 0.0) {
                ++votes[i];
            }
            else if (scores[index].u <= 0) {
                ++votes[j];
            }
            else {
                ++votes[i];
                ++votes[j];
            }
        }
    }

    for (i = 0; i < N; ++i) {
        if (votes[i] > votes[max_class]) {
            winning_classes = 1;
            classes[0] = classifier_get_classes(classifier)[i];
            max_class = i;
        }
        else if (votes[i] == votes[max_class]) {
            classes[winning_classes] = classifier_get_classes(classifier)[i];
            ++winning_classes;
        }
    }

    free(votes);

    return winning_classes;
}



HybridClassifier hybrid_classifier_create(const Classifier classifier) {
    const unsigned int N = classifier_get_n_classes(classifier);
    HybridClassifier hybrid_classifier = (HybridClassifier) malloc(sizeof(struct hybrid_classifier));
    if (!hybrid_classifier) {
        report_error("Cannot allocate memory.");
    }

    hybrid_classifier->classifier = classifier;
    hybrid_classifier->interval_classifier = interval_classifier_create(classifier);
    hybrid_classifier->raf_classifier = raf_classifier_create(classifier);
    hybrid_classifier->buffer = (Interval *) malloc((N * (N - 1)) / 2 * sizeof(Interval));
    if (!hybrid_classifier->buffer) {
        report_error("Cannot allocate memory.");
    }

    return hybrid_classifier;
}



void hybrid_classifier_delete(HybridClassifier *hybrid_classifier) {
    if (hybrid_classifier == NULL || *hybrid_classifier == NULL) {
        return;
    }

    interval_classifier_delete(&((*hybrid_classifier)->interval_classifier));
    raf_classifier_delete(&((*hybrid_classifier)->raf_classifier));
    free((*hybrid_classifier)->buffer);
    free(*hybrid_classifier);
    *hybrid_classifier = NULL;
}



Interval *hybrid_classifier_score(
    const HybridClassifier hybrid_classifier,
    const AdversarialRegion adversarial_region
) {
    switch (classifier_get_type(hybrid_classifier->classifier)) {
        case CLASSIFIER_OVO:
            return hybrid_classifier_ovo_score(hybrid_classifier, adversarial_region);
        default:
            report_error("Unsupported classifier type.");
    }
}



unsigned int hybrid_classifier_classify(
    const HybridClassifier hybrid_classifier,
    const AdversarialRegion adversarial_region,
    char **classes
) {
    switch (classifier_get_type(hybrid_classifier->classifier)) {
        case CLASSIFIER_OVO:
            return hybrid_classifier_ovo_classify(hybrid_classifier, adversarial_region, classes);
        default:
            report_error("Unsupported classifier.");
    }
}
