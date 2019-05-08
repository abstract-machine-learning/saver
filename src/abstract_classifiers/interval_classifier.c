#include "interval_classifier.h"

#include <malloc.h>
#include "../report_error.h"


/**
 * Structure of an interval classifier.
 */
struct interval_classifier {
    Classifier classifier;  /**< Concrete classifier. */
    Interval *buffer;       /**< Internal buffer. */
};



static void interval_kernel(
    Interval *r,
    const Kernel kernel,
    const Real *x,
    const Interval *y,
    const unsigned int space_size
) {
    if (kernel_get_type(kernel) == KERNEL_RBF) {
        Interval exponent = {0.0, 0.0}, product;
        unsigned int i;

        for (i = 0; i < space_size; ++i) {
            interval_translate(&product, y[i], -x[i]);
            interval_pow(&product, product, 2);
            interval_add(&exponent, exponent, product);
        }
        interval_scale(&exponent, exponent, -kernel_get_gamma(kernel));
        interval_exp(r, exponent);
    }

    else if (kernel_get_type(kernel) == KERNEL_POLYNOMIAL) {
        unsigned int i;
        Interval product;
        product.l = kernel_get_c(kernel);
        product.u = kernel_get_c(kernel);

        for (i = 0; i < space_size; ++i) {
            interval_fma(&product, x[i], y[i], product);
        }
        interval_pow(r, product, kernel_get_degree(kernel));
    }

    else {
        report_error("Unsupported kernel type.");
    }
}



static void overapproximate(
    Interval *abstract_sample,
    const AdversarialRegion adversarial_region,
    const unsigned int space_size
) {
    const Real *sample = adversarial_region.sample;
    const Perturbation perturbation = adversarial_region.perturbation;
    const Real magnitude = perturbation_get_magnitude(perturbation);
    const Real *epsilon_l = perturbation_get_epsilon_lowerbounds(perturbation),
               *epsilon_u = perturbation_get_epsilon_upperbounds(perturbation);
    const unsigned int image_width = perturbation_get_image_width(perturbation),
                       image_height = perturbation_get_image_height(perturbation),
                       frame_width = perturbation_get_frame_width(perturbation),
                       frame_height = perturbation_get_frame_height(perturbation);
    unsigned int i, j;

    switch (perturbation_get_type(perturbation)) {
        case PERTURBATION_L_ONE:
        case PERTURBATION_L_INF:
            for (i = 0; i < space_size; ++i) {
                abstract_sample[i].l = (sample[i] - magnitude >= 0.0) ? sample[i] - magnitude : 0.0;
                abstract_sample[i].u = (sample[i] + magnitude <= 1.0) ? sample[i] + magnitude : 1.0;
            }
            break;

        case PERTURBATION_HYPER_RECTANGLE:
            for (i = 0; i < space_size; ++i) {
                abstract_sample[i].l = sample[i] - epsilon_l[i];
                abstract_sample[i].u = sample[i] + epsilon_u[i];
            }
            break;

        case PERTURBATION_FRAME:
            for (i = 0; i < image_height; ++i) {
                for (j = 0; j < image_width; ++j) {
                    const unsigned int index = i * image_width + j;
                    if (i < frame_height || i > image_height - frame_height
                     || j < frame_width || j > image_width - frame_width) {
                        abstract_sample[index].l = (sample[index] - magnitude >= 0.0)
                                                 ? sample[index] - magnitude : 0.0;
                        abstract_sample[index].u = (sample[index] + magnitude <= 1.0)
                                                 ? sample[index] + magnitude : 1.0;
                    }
                    else {
                        abstract_sample[index].l = sample[index];
                        abstract_sample[index].u = sample[index];
                    }
                }
            }
            break;

        default:
            report_error("Unrecognized type of adversarial region.");
    }
}



static Interval *interval_classifier_ovo_score(
    const IntervalClassifier interval_classifier,
    const AdversarialRegion adversarial_region
) {
    const Real *alpha = classifier_get_alpha(interval_classifier->classifier);
    const Real *bias = classifier_get_bias(interval_classifier->classifier);
    const Real *support_vectors = classifier_get_support_vectors(interval_classifier->classifier);
    const unsigned int space_size = classifier_get_space_size(interval_classifier->classifier);
    const unsigned int N = classifier_get_n_classes(interval_classifier->classifier);
    const unsigned int *n_support_vectors = classifier_get_n_support_vectors(interval_classifier->classifier);
    const Kernel kernel = classifier_get_kernel(interval_classifier->classifier);
    unsigned int i, j, k, support_vectors_i_offset = 0, total_support_vectors = 0;
    Interval *abstract_sample, *K;

    /* Overapproximates abstract sample with an interval */
    abstract_sample = (Interval *) malloc(space_size * sizeof(Interval));
    overapproximate(abstract_sample, adversarial_region, space_size);

    /* Computes total number of support vectors */
    for (i = 0; i < N; ++i) {
        total_support_vectors += n_support_vectors[i];
    }


    /* Computes a complete version if kernel is linear */
    if (kernel_get_type(kernel) == KERNEL_LINEAR) {
        const Real *coefficients = classifier_get_coefficients(interval_classifier->classifier);
        for (i = 0; i < N; ++i) {
            for (j = i + 1; j < N; ++j) {
                const unsigned int index = i * (N - 1) - (i * (i + 1)) / 2 + j - 1;

                Interval sum;
                sum.l = bias[index];
                sum.u = bias[index];
                for (k = 0; k < space_size; ++k) {
                    interval_fma(&sum, coefficients[index * space_size + k], abstract_sample[k], sum);
                }

                interval_classifier->buffer[index] = sum;
            }
        }

        free(abstract_sample);
        return interval_classifier->buffer;
    }



    /* Precomputes kernel matrix */
    K = (Interval *) malloc(total_support_vectors * sizeof(Interval));
    for (i = 0; i < total_support_vectors; ++i) {
        interval_kernel(
            K + i,
            kernel,
            support_vectors + i * space_size,
            abstract_sample,
            space_size
        );
    }

    /* Computes scores */
    for (i = 0; i < N; ++i) {
        unsigned int support_vectors_j_offset;
        if (i > 0) {
            support_vectors_i_offset += n_support_vectors[i - 1];
        }
        support_vectors_j_offset = support_vectors_i_offset;
        for (j = i + 1; j < N; ++j) {
            const unsigned int index = i * (N - 1) - (i * (i + 1)) / 2 + j - 1;

            Interval sum;
            sum.l = bias[index];
            sum.u = bias[index];
            support_vectors_j_offset += n_support_vectors[j - 1];

            for (k = 0; k < n_support_vectors[i]; ++k) {
                interval_fma(
                    &sum,
                    alpha[(j - 1) * total_support_vectors + support_vectors_i_offset + k],
                    K[support_vectors_i_offset + k],
                    sum
                );
            }

            for (k = 0; k < n_support_vectors[j]; ++k) {
                interval_fma(
                    &sum,
                    alpha[i * total_support_vectors + support_vectors_j_offset + k],
                    K[support_vectors_j_offset + k],
                    sum
                );
            }

            interval_classifier->buffer[index] = sum;
        }
    }
    free(abstract_sample);
    free(K);

    return interval_classifier->buffer;
}



static unsigned int interval_classifier_ovo_classify(
    const IntervalClassifier interval_classifier,
    const AdversarialRegion adversarial_region,
    char **classes
) {
    const Interval *scores = interval_classifier_ovo_score(interval_classifier, adversarial_region);
    Interval *votes;
    unsigned int i, j, winning_classes = 0;
    const unsigned int N = classifier_get_n_classes(interval_classifier->classifier);

    votes = (Interval *) malloc(N * sizeof(Interval));
    for (i = 0; i < N; ++i) {
        votes[i].l = 0.0;
        votes[i].u = 0.0;
    }

    for (i = 0; i < N; ++i) {
        for (j = i + 1; j < N; ++j) {
            const unsigned int index = i * (N - 1) - (i * (i + 1)) / 2 + j - 1;
            if (scores[index].l >= 0.0) {
                votes[i].l += 1.0;
                votes[i].u += 1.0;
            }
            else if (scores[index].u <= 0.0) {
                votes[j].l += 1.0;
                votes[j].u += 1.0;
            }
            else {
                votes[i].u += 1.0;
                votes[j].u += 1.0;
            }
        }
    }


    for (i = 0; i < N; ++i) {
        unsigned int is_maximal = 1;
        for (j = 0; j < N; ++j) {
            if (i == j) {
                continue;
            }
            if (votes[j].l > votes[i].u) {
                is_maximal = 0;
                break;
            }
        }

        if (is_maximal) {
            classes[winning_classes] = classifier_get_classes(interval_classifier->classifier)[i];
            ++winning_classes;
        }
    }

    free(votes);

    return winning_classes;
/*
    const Interval *scores = interval_classifier_ovo_score(interval_classifier, adversarial_region);
    unsigned int *votes, i, j, max_class = 0, winning_classes = 0;
    const unsigned int N = classifier_get_n_classes(interval_classifier->classifier);

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
            classes[0] = classifier_get_classes(interval_classifier->classifier)[i];
            max_class = i;
        }
        else if (votes[i] == votes[max_class]) {
            classes[winning_classes] = classifier_get_classes(interval_classifier->classifier)[i];
            ++winning_classes;
        }
    }

    free(votes);

    return winning_classes;
*/
}



IntervalClassifier interval_classifier_create(const Classifier classifier) {
    const unsigned int N = classifier_get_n_classes(classifier);
    IntervalClassifier interval_classifier = (IntervalClassifier) malloc(sizeof(struct interval_classifier));
    if (!interval_classifier) {
        report_error("Cannot allocate memory.");
    }

    interval_classifier->classifier = classifier;
    interval_classifier->buffer = (Interval *) malloc((N * (N - 1)) / 2 * sizeof(Interval));

    return interval_classifier;
}



void interval_classifier_delete(IntervalClassifier *interval_classifier) {
    if (interval_classifier == NULL || *interval_classifier == NULL) {
        return;
    }

    free((*interval_classifier)->buffer);
    free(*interval_classifier);
    *interval_classifier = NULL;
}


Interval *interval_classifier_score(
    const IntervalClassifier interval_classifier,
    const AdversarialRegion adversarial_region
) {
    switch (classifier_get_type(interval_classifier->classifier)) {
        case CLASSIFIER_OVO:
            return interval_classifier_ovo_score(interval_classifier, adversarial_region);
        default:
            report_error("Unsupported classifier type.");
    }
}



unsigned int interval_classifier_classify(
    const IntervalClassifier interval_classifier,
    const AdversarialRegion adversarial_region,
    char **classes
) {
    switch (classifier_get_type(interval_classifier->classifier)) {
        case CLASSIFIER_OVO:
            return interval_classifier_ovo_classify(interval_classifier, adversarial_region, classes);
        default:
            report_error("Unsupported classifier.");
    }
}
