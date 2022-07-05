#include "classifier.h"

#include <stdio.h>
#include <malloc.h>
#include <string.h>

#include "report_error.h"

#define READ_BUFFER_SIZE 32


/**
 * Structure of a classifier.
 */
struct classifier {
    ClassifierType type;              /**< Type of classifier. */
    Kernel kernel;                    /**< Kernel object. */
    unsigned int n_classes;           /**< Number of classes. */
    char **classes;                   /**< Array of class names. */
    Real *alpha;                      /**< Array of dual coefficients. */
    Real *support_vectors;            /**< Linearized array of support vectors. */
    unsigned int *n_support_vectors;  /**< Number of support vectors per class. */
    Real *bias;                       /**< Intercept, one per classifier. */
    unsigned int space_size;          /**< Size of the feature space. */
    Real *buffer;                     /**< Internal buffer. */
    Real *coefficients;               /**< Internal array of coefficient, only relevant for linear kernel. */
    unsigned int automatic_free;      /**< Tells the classifier to deallocate memory on deletion if set to 1. */
};



/**********************************************************************
 * Internal support functions.
 *********************************************************************/

static unsigned int classifier_buffer_size(const Classifier classifier) {
    switch (classifier->type) {
        case CLASSIFIER_OVR: return classifier->n_classes;
        case CLASSIFIER_OVO: return (classifier->n_classes * (classifier->n_classes + 1)) / 2;
        default: report_error("Unsupported classifier.");
    }
}



static Real* classifier_ovo_score(const Classifier classifier, const Real *sample) {
    unsigned int i, j, k;
    const unsigned int N = classifier->n_classes;
    unsigned int support_vectors_i_offset = 0;
    unsigned int total_support_vectors = 0;
    Real *K;

    /* Computes total number of support vectors */
    for (i = 0; i < classifier->n_classes; ++i) {
        total_support_vectors += classifier->n_support_vectors[i];
    }

    /* Computes a faster version if kernel is linear */
    if (kernel_get_type(classifier->kernel) == KERNEL_LINEAR) {
        const unsigned int space_size = classifier->space_size;
        const Real *coefficients = classifier->coefficients;
        for (i = 0; i < N; ++i) {
            for (j = i + 1; j < N; ++j) {
                const unsigned int index = i * (N - 1) - (i * (i + 1)) / 2 + j - 1;
                Real sum = classifier->bias[index];

                for (k = 0; k < space_size; ++k) {
                    sum += coefficients[index * space_size + k] * sample[k];
                }

                classifier->buffer[index] = sum;
            }
        }

        return classifier->buffer;
    }


    /* Precomputes kernel matrix */
    K = (Real *) malloc(total_support_vectors * sizeof(Real));
    for (i = 0; i < total_support_vectors; ++i) {
        K[i] = kernel_compute(
            classifier->kernel,
            sample,
            classifier->support_vectors + i * classifier->space_size,
            classifier->space_size
        );
    }

    /* Computes scores */
    for (i = 0; i < classifier->n_classes; ++i) {
        unsigned int support_vectors_j_offset;
        if (i > 0) {
            support_vectors_i_offset += classifier->n_support_vectors[i - 1];
        }
        support_vectors_j_offset = support_vectors_i_offset;
        for (j = i + 1; j < classifier->n_classes; ++j) {
            const unsigned int index = i * (N - 1) - (i * (i + 1)) / 2 + j - 1;

            Real sum = classifier->bias[index];
            support_vectors_j_offset += classifier->n_support_vectors[j - 1];

            for (k = 0; k < classifier->n_support_vectors[i]; ++k) {
                sum += classifier->alpha[(j - 1) * total_support_vectors + support_vectors_i_offset + k] * K[support_vectors_i_offset + k];
            }

            for (k = 0; k < classifier->n_support_vectors[j]; ++k) {
                sum += classifier->alpha[i * total_support_vectors + support_vectors_j_offset + k] * K[support_vectors_j_offset + k];
            }
            classifier->buffer[index] = sum;
        }
    }

    free(K);

    return classifier->buffer;
}




static unsigned int classifier_ovo_classify(
    const Classifier classifier,
    const Real* sample,
    char **classes
) {
    unsigned int *votes, i, j, max_class = 0, winning_classes = 0;
    const unsigned int N = classifier->n_classes;
    classifier_ovo_score(classifier, sample);
    //printf("Score Conc: %f",classifier->buffer[0]);

    votes = (unsigned int *) calloc(N * (N + 1) / 2, sizeof(unsigned int));
    if (votes == NULL) {
        report_error("Cannot allocate memory.");
    }

    for (i = 0; i < N; ++i) {
        for (j = i + 1; j < N; ++j) {
            const unsigned int index = i * (N - 1) - (i * (i + 1)) / 2 + j - 1;
            ++votes[classifier->buffer[index] >= 0.0 ? i : j];
        }
    }

    for (i = 0; i < N; ++i) {
        if (votes[i] > votes[max_class]) {
            winning_classes = 1;
            classes[0] = classifier->classes[i];
            max_class = i;
        }
        else if (votes[i] == votes[max_class]) {
            classes[winning_classes] = classifier->classes[i];
            ++winning_classes;
        }
    }


    free(votes);

    return winning_classes;
}



/**********************************************************************
 * Functions implementing the header file.
 *********************************************************************/

Classifier classifier_create(
    const ClassifierType type,
    const Kernel kernel,
    const unsigned int n_classes,
    const char **classes,
    const Real *alpha,
    const Real *support_vectors,
    const unsigned int *n_support_vectors,
    const Real *bias,
    const unsigned int space_size
) {
    Classifier classifier = (Classifier) malloc(sizeof(struct classifier));
    if (classifier == NULL) {
        report_error("Memory exhaustion");
    }

    classifier->type = type;
    classifier->kernel = kernel;
    classifier->n_classes = n_classes;
    classifier->classes = (char **) classes;
    classifier->alpha = (Real *) alpha;
    classifier->support_vectors = (Real *) support_vectors;
    classifier->n_support_vectors = (unsigned int *) n_support_vectors;
    classifier->bias = (Real *) bias;
    classifier->space_size = space_size;
    classifier->automatic_free = 0;

    classifier->buffer = (Real *) malloc(classifier_buffer_size(classifier) * sizeof(Real));
    if (classifier == NULL) {
        report_error("Memory exhaustion.");
    }


    classifier->coefficients = NULL;
    /* Computes primal coefficients if kernel is linear */
    if (kernel_get_type(classifier_get_kernel(classifier)) == KERNEL_LINEAR) {
        unsigned int i, j, k, support_vectors_i_offset = 0, total_support_vectors = 0;
        const unsigned int N = n_classes;

        /* Computes total number of support vectors */
        for (i = 0; i < N; ++i) {
            total_support_vectors += n_support_vectors[i];
        }

        /* Computes coefficients */
        classifier->coefficients = (Real *) malloc((N * (N - 1)) / 2 * space_size * sizeof(Real));
        if (!classifier->coefficients) {
            report_error("Cannot allocate memory.");
        }

        for (i = 0; i < N; ++i) {
            unsigned int support_vectors_j_offset;
            if (i > 0) {
                support_vectors_i_offset += n_support_vectors[i - 1];
            }
            support_vectors_j_offset = support_vectors_i_offset;
            for (j = i + 1; j < N; ++j) {
                const unsigned int index = i * (N - 1) - (i * (i + 1)) / 2 + j - 1;

                support_vectors_j_offset += n_support_vectors[j - 1];

                for (k = 0; k < space_size; ++k) {
                    unsigned int t;
                    classifier->coefficients[index * space_size + k] = 0.0;
                    for (t = 0; t < n_support_vectors[i]; ++t) {
                        classifier->coefficients[index * space_size + k] += alpha[(j - 1) * total_support_vectors + support_vectors_i_offset + t] * support_vectors[(support_vectors_i_offset + t) * space_size + k];
                    }
                    for (t = 0; t < n_support_vectors[j]; ++t) {
                        classifier->coefficients[index * space_size + k] += alpha[i * total_support_vectors + support_vectors_j_offset + t] * support_vectors[(support_vectors_j_offset + t) * space_size + k];
                    }
                }
            }
        }
    }

    return classifier;
}



Classifier classifier_read(FILE *stream) {
    char type[READ_BUFFER_SIZE], kernel_name[READ_BUFFER_SIZE];
    ClassifierType classifier_type;
    Kernel kernel;
    unsigned int space_size, n_classes, i, *n_support_vectors, total_support_vectors = 0;
    int result;
    char **classes;
    Real *alpha, *support_vectors, *bias;
    Classifier classifier;

    /* Reads type of classifier */
    result = fscanf(stream, "%s %u %u", type, &space_size, &n_classes);
    if (strcmp(type, "binary") == 0) {
        classifier_type = CLASSIFIER_BINARY;
    }
    else if (strcmp(type, "ovo") == 0) {
        classifier_type = CLASSIFIER_OVO;
    }
    else if (strcmp(type, "ovr") == 0) {
        classifier_type = CLASSIFIER_OVR;
    }
    else {
        report_error("Unsupported type of classifier.");
    }


    /* Reads kernel data */
    result = fscanf(stream, "%s", kernel_name);
    if (strcmp(kernel_name, "rbf") == 0) {
        double gamma;
        result = fscanf(stream, "%lf", &gamma);
        kernel = kernel_create(KERNEL_RBF, gamma, 0.0, 0.0);
    }
    else if (strcmp(kernel_name, "polynomial") == 0) {
        double degree, c;
        result = fscanf(stream, "%lf %lf", &degree, &c);
        kernel = kernel_create(KERNEL_POLYNOMIAL, 0.0, (unsigned int) degree, c);
    }
    else {
        kernel = kernel_create(KERNEL_LINEAR, 0.0, 0.0, 0.0);
    }


    /* Reads classes and number of support vectors */
    classes = (char **) malloc(n_classes *  sizeof(char *));
    n_support_vectors = (unsigned int *) malloc(n_classes * sizeof(unsigned int));
    for (i = 0; i < n_classes; ++i) {
        classes[i] = (char *) malloc(READ_BUFFER_SIZE * sizeof(char));
        result = fscanf(stream, "%s %u", classes[i], n_support_vectors + i);
        total_support_vectors += n_support_vectors[i];
    }


    /* Reads dual coefficients */
    alpha = (Real *) malloc(total_support_vectors * (n_classes - 1) * sizeof(Real));
    for (i = 0; i < total_support_vectors * (n_classes - 1); ++i) {
        double buffer;
        result = fscanf(stream, "%lf", &buffer);
        alpha[i] = (Real) buffer;
    }


    /* Reads support vectors */
    support_vectors = (Real *) malloc(total_support_vectors * space_size * sizeof(Real));
    for (i = 0; i < total_support_vectors * space_size; ++i) {
        double buffer;
        result = fscanf(stream, "%lf", &buffer);
        support_vectors[i] = (Real) buffer;
    }


    /* Reads intercepts */
    bias = (Real *) malloc((n_classes * (n_classes - 1)) / 2 * sizeof(Real));
    for (i = 0; i < (n_classes * (n_classes - 1)) / 2; ++i) {
        double buffer;
        result = fscanf(stream, "%lf", &buffer);
        bias[i] = buffer;
    }


    /* Creates classifier */
    classifier = classifier_create(
        classifier_type,
        kernel,
        n_classes,
        (const char **) classes,
        alpha,
        support_vectors,
        n_support_vectors,
        bias,
        space_size
    );
    classifier->automatic_free = 1;

    (void) result;

    return classifier;
}



void classifier_delete(Classifier *classifier) {
    if (classifier == NULL || *classifier == NULL) {
        return;
    }

    free((*classifier)->buffer);
    if ((*classifier)->coefficients != NULL) {
        free((*classifier)->coefficients);
    }

    if ((*classifier)->automatic_free) {
        unsigned int i;
        for (i = 0; i < (*classifier)->n_classes; ++i) {
            free((*classifier)->classes[i]);
        }

        free((*classifier)->classes);
        free((*classifier)->n_support_vectors);
        free((*classifier)->alpha);
        free((*classifier)->support_vectors);
        free((*classifier)->bias);
        kernel_delete(&((*classifier)->kernel));
    }

    free(*classifier);
    *classifier = NULL;
}



ClassifierType classifier_get_type(const Classifier classifier) {
    return classifier->type;
}



Kernel classifier_get_kernel(const Classifier classifier) {
    return classifier->kernel;
}


unsigned int classifier_get_n_classes(const Classifier classifier) {
    return classifier->n_classes;
}


char **classifier_get_classes(const Classifier classifier) {
    return classifier->classes;
}


Real *classifier_get_alpha(const Classifier classifier) {
    return classifier->alpha;
}


Real *classifier_get_support_vectors(const Classifier classifier) {
    return classifier->support_vectors;
}


unsigned int *classifier_get_n_support_vectors(const Classifier classifier) {
    return classifier->n_support_vectors;
}


Real *classifier_get_bias(const Classifier classifier) {
    return classifier->bias;
}


unsigned int classifier_get_space_size(const Classifier classifier) {
    return classifier->space_size;
}


Real *classifier_get_coefficients(const Classifier classifier) {
    return classifier->coefficients;
}



Real *classifier_score(const Classifier classifier, const Real* sample) {
    switch (classifier->type) {
        case CLASSIFIER_OVO: return classifier_ovo_score(classifier, sample);
        default: report_error("Unsupported classifier.");
    }
}



unsigned int classifier_classify(
    const Classifier classifier,
    const Real *sample,
    char **classes
) {
    switch(classifier->type) {
        case CLASSIFIER_OVO: return classifier_ovo_classify(classifier, sample, classes);
        default: report_error("Unsupported classifier.");
    }
}
