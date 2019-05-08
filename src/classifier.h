/**
 * SVM classifier.
 * 
 * Defines a (concrete) SVM classifier.
 * 
 * @file classifier.h
 * @author Marco Zanella <marco.zanella.1991@gmail.com>
 * 
 * @todo Binary and OVR classifiers are currently not supported.
 */
#ifndef CLASSIFIER_H
#define CLASSIFIER_H

#include <stdio.h>
#include "type.h"
#include "kernel.h"


/**
 * Available types of classifier.
 */
typedef enum {
    CLASSIFIER_BINARY,  /**< Binary classifier. */
    CLASSIFIER_OVR,     /**< One-Versus-Rest classifier. */
    CLASSIFIER_OVO      /**< One-Versus-One classifier. */
} ClassifierType;


/**
 * Type of a classifier.
 */
typedef struct classifier *Classifier;


/**
 * Creates a new classifier.
 *
 * @param[in] type              Type of classifier
 * @param[in] kernel            Kernel object
 * @param[in] n_classes         Number of classes
 * @param[in] classes           Array of names of classes
 * @param[in] alpha             Array of dual coefficients
 * @param[in] support_vectors   Array of support vectors (flat)
 * @param[in] n_support_vectors Number of support vectors per class
 * @param[in] bias              Array of intercept, one per classifier
 * @param[in] space_size        Number of features
 * @return A new classifier
 */
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
);


/**
 * Creates a classifier reading from a source.
 *
 * @param[in,out] stream Source to read from
 * @return A new classifier
 */
Classifier classifier_read(FILE *stream);


/**
 * Deletes a classifier.
 * 
 * @param[out] classifier Pointer to classifier to delete
 */
void classifier_delete(Classifier *classifier);


/**
 * Returns type of given classifier.
 * 
 * @param[in] classifier Classifier
 * @return Type of classifier
 */
ClassifierType classifier_get_type(const Classifier classifier);


/**
 * Returns kernel function of given classifier.
 * 
 * @param[in] classifier Classifier
 * @return Kernel function used by classifier
 */
Kernel classifier_get_kernel(const Classifier classifier);


/**
 * Returns number of classes of given classifier.
 * 
 * @param[in] classifier Classifier
 * @return Number of classes
 */
unsigned int classifier_get_n_classes(const Classifier classifier);


/**
 * Returns array of class names of given classifier.
 * 
 * @param[in] classifier Classifier
 * @return Array of class names
 */
char **classifier_get_classes(const Classifier classifier);


/**
 * Returns array of dual coefficients of given classifier.
 * 
 * @param[in] classifier Classifier
 * @return Array of dual coefficients (flattened)
 */
Real *classifier_get_alpha(const Classifier classifier);


/**
 * Returns array of support vectors of given classifier.
 * 
 * @param[in] classifier Classifier
 * @return Array of support vectors (flattened)
 */
Real *classifier_get_support_vectors(const Classifier classifier);


/**
 * Returns number of support vectors per class in given classifier.
 * 
 * @param[in] classifier Classifier
 * @return Number of support vectors per class
 */
unsigned int *classifier_get_n_support_vectors(const Classifier classifier);


/**
 * Returns array of intercept in given classifier.
 * 
 * @param[in] classifier Classifier
 * @return Array of intercept, one per internal classifier
 */
Real *classifier_get_bias(const Classifier classifier);


/**
 * Returns number of features used by given classifier.
 * 
 * @param[in] classifier Classifier
 * @return Number of features
 */
unsigned int classifier_get_space_size(const Classifier classifier);


/**
 * Returns array of primal coefficients for given classifier.
 * 
 * @param[in] classifier Classifier
 * @return Array of prima coefficients
 * @note Primal coefficients are defined only when a linear kernel is used.
 *       Calling this function passing a classifier which uses a non linear
 *       kernel will return a null pointer.
 */
Real *classifier_get_coefficients(const Classifier classifier);


/**
 * Returns output of the decision function on a sample.
 * 
 * @param[in] classifier Classifier
 * @param[in] sample     Sample
 * @return Result of the decision function
 */
Real *classifier_score(const Classifier classifier, const Real *sample);


/**
 * Returns class of a sample according to given classifier.
 * 
 * @param[in]  classifier Classifier
 * @param[in]  sample     Sample
 * @param[out] classes    Previously allocated array of strings, will be
 *                             filled with chosen classes
 * @return Number of chosen classes
 */
unsigned int classifier_classify(
    const Classifier classifier,
    const Real *sample,
    char **classes
);

#endif
