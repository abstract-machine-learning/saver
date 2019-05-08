/**
 * Perturbations.
 *
 * Defines which perturbations are available.
 *
 * @file perturbation.h
 * @author Marco Zanella <marco.zanella.1991@gmail.com>
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
 * Type of perturbation.
 *
 * Every perturbation is a function \f$P: \mathbb{R}^n \rightarrow \wp(\mathbb{R}^n)\f$
 * computing the set of points which are obtained by altering an initial point \f$p \in \mathbb{R}^n\f$
 * by a magnitude \f$\epsilon\f$. Depending on the type of perturbation, \f$\epsilon\f$ may be a number in
 * \f$\mathbb{R}\f$, a vector of \f$\mathbb{R}^n\f$, a tuple, etc.
 */
typedef enum {
    PERTURBATION_L_ONE,           /**< Norm one perturbation: 
                                       \f$P_\epsilon(p) = \{x \in \mathbb{R}^n ~|~ ||p - x||_1 \leq \epsilon\}\f$. */
    PERTURBATION_L_INF,           /**< Norm infinity perturbation:
                                       \f$P_\epsilon(p) = \{x \in \mathbb{R}^n ~|~ ||p - x||_\infty \leq \epsilon\}\f$. */
    PERTURBATION_HYPER_RECTANGLE, /**< Hyper rectangle: 
                                       \f$P_\epsilon(p) = \{x \in \mathbb{R}^n ~|~ p - \epsilon_l \leq x \leq p + e_u\}\f$. */
    PERTURBATION_FRAME            /**< Occlusive fame around an image */
} PerturbationType;



/**
 * Type of a perturbation.
 */
typedef struct perturbation *Perturbation;



/**
 * Creates a hyper-rectangle perturbation.
 *
 * Lower and upper bounds are initially set to 0.
 *
 * @param[out] perturbation Perturbation
 * @param[in] space_size Size of the space
 * @note #perturbation_delete should be called to ensure proper memory deallocation.
 */
void perturbation_create_hyper_rectangle(
    Perturbation *perturbation,
    const unsigned int space_size
);


/**
 * Reads a perturbation.
 *
 * @param[out] perturbation Perturbation
 * @param[in] argc ARGument Counter
 * @param[in] argv ARGument Vector
 * @param[in] space_size Size of the space
 * @note #perturbation_delete should be called to ensure proper memory deallocation.
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
 * Copies a perturbation.
 *
 * @param[out] dst Destination perturbation
 * @param[in] src Source perturbation
 */
void perturbation_copy(Perturbation dst, const Perturbation src);


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
 * Returns space size of given perturbation.
 *
 * @param[in] perturbation Perturbation
 * @return space size
 */
unsigned int perturbation_get_space_size(const Perturbation perturbation);


/**
 * Returns array of epsilon lowerbounds.
 *
 * @param[in] perturbation Perturbation
 * @return Array of epsilon lowerbounds
 */
Real *perturbation_get_epsilon_lowerbounds(const Perturbation perturbation);


/**
 * Returns array of epsilon upperbounds.
 *
 * @param[in] perturbation Perturbation
 * @return Array of epsilon upperbounds
 */
Real *perturbation_get_epsilon_upperbounds(const Perturbation perturbation);


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


/**
 * Sets magnitude of a perturbation.
 *
 * @param[out] perturbation Perturbation
 * @param[in] magnitude New magnitude
 */
void perturbation_set_magnitude(Perturbation perturbation, const Real magnitude);


/**
 * Prints a perturbation.
 *
 * @param[in] perturbation Perturbation
 * @param[out] stream Stream
 */
void perturbation_print(const Perturbation perturbation, FILE *stream);

#endif
