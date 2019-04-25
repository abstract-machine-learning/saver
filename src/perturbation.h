/**
 * Perturbations.
 *
 * Defines which perturbations are available.
 *
 * @file perturbation.h
 */
#ifndef PERTURBATION_H
#define PERTURBATION_H

#include <stdio.h>

#include "type.h"

/** Default perturbation name. */
#define DEFAULT_PERTURBATION "l_inf"

/** Default pertubation magnitude. */
#define DEFAULT_EPSILON 0.01


/**
 * Type of perturbations.
 */
typedef enum {
    PERTURBATION_L_ONE,  /**< Norm one perturbation: \f$||x||_1\f$. */
    PERTURBATION_L_INF,  /**< Norm infinity perturbation: \f$||x||_\infty\f$. */
    PERTURBATION_FRAME   /**< Fame around the image */
} PerturbationType;



/**
 * Type of a perturbation.
 */
typedef struct perturbation *Perturbation;



/**
 * Reads a perturbation.
 *
 * @param[out] perturbation Perturbation
 * @param[in] argc ARGument Counter
 * @param[in] argv ARGument Vector
 * @param[in] space_size Size of the space
 */
void perturbation_read(
    Perturbation *perturbation,
    const int argc,
    const char *argv[],
    const unsigned int space_size
);


/**
 * Deletes a perturbation.
 *
 * @param[out] perturbation Perturbation to delete
 */
void perturbation_delete(Perturbation *perturbation);


/**
 * Returns type of perturbation.
 *
 * @param[in] perturbation Perturbation
 * @return Type of perturbation
 */
PerturbationType perturbation_get_type(const Perturbation perturbation);


/**
 * Returns magnitude of perturbation
 *
 * @param[in] perturbation Perturbation
 * @return Magnitude of perturbation
 */
Real perturbation_get_magnitude(const Perturbation perturbation);


/**
 * Returns image width of given perturbation.
 *
 * @param[in] perturbation Perturbation
 * @return Image width
 */
Real perturbation_get_image_width(const Perturbation perturbation);


/**
 * Returns image height of given perturbation.
 *
 * @param[in] perturbation Perturbation
 * @return Image height
 */
Real perturbation_get_image_height(const Perturbation perturbation);


/**
 * Returns frame width of given perturbation.
 *
 * @param[in] perturbation Perturbation
 * @return Frame width
 */
Real perturbation_get_frame_width(const Perturbation perturbation);


/**
 * Returns frame height of given perturbation.
 *
 * @param[in] perturbation Perturbation
 * @return Frame height
 */
Real perturbation_get_frame_height(const Perturbation perturbation);

#endif
