/**
 * Reduced Affine Forms (RAF).
 *
 * Defines abstract transfer functions for RAFs.
 *
 * @file raf.h
 * @author Marco Zanella <marco.zanella.1991@gmail.com>
 */
#ifndef RAF_H
#define RAF_H

#include <stdio.h>

#include "interval.h"

/** RAF abstract value. */
typedef struct raf Raf;

/**
 * Structure of a RAF.
 *
 * A Reduced Affine Form (RAF) is an alternative way to expess an interval. A
 * RAF is defined by a center, a fixed quantity of radius variables whose origin
 * is tracked and an additional radius value:
 * \f$c + \sum_{i=1}^n {x_i \epsilon_i} + \delta \epsilon_{n + 1}\f$, where every
 * \f$\epsilon_i = [-1; +1]\f$.
 */
struct raf {
    Real c;             /**< Center. */
    Real *noise;        /**< Tracked noise variables. */
    Real delta;         /**< Additional noise. */
    unsigned int size;  /**< Number of noise variables. */
    int index;          /**< \f$\geq 0\f$ iff RAF contains a single non-zero noise
                             value. Negative otherwise. Used for performance
                             reasons. */
};


/**
 * Creates a RAF.
 *
 * Allocates memory and centers the RAF in 0.
 *
 * @param[out] r RAF to be initialized
 * @param[in] size Number of noise variables.
 * @note #raf_delete must be called to ensure memory deallocation
 */
void raf_create(Raf *r, const unsigned int size);


/**
 * Deletes a RAF.
 *
 * @param[out] r RAF to delete
 */
void raf_delete(Raf *r);


/**
 * Centers a RAF in a given value and sets radius to 0.
 *
 * @param[out] r RAF
 * @param[in] value New center
 */
void raf_singleton(Raf *r, const Real value);


/**
 * Copies a RAF.
 *
 * @param[out] r Destination
 * @param[in] x Source
 * @warning If RAFs have different sizes, copy will result in an undefined behavior
 */
void raf_copy(Raf *r, const Raf x);


/**
 * Converts a RAF into an interval.
 *
 * @param[out] r Destination interval
 * @param[in] x RAF to convert
 */
void raf_to_interval(Interval *r, const Raf x);


/**
 * Converts an interval to a RAF.
 *
 * @param[out] r Destination RAF
 * @param[in] x Interval to convert
 */
void interval_to_raf(Raf *r, const Interval x);


/**
 * Computes center of a RAF.
 *
 * @param[in] x RAF
 * @return Center of given RAF
 */
Real raf_midpoint(const Raf x);


/**
 * Computes radius of a RAF.
 *
 * @param[in] x RAF
 * @return Radius of given RAF
 */
Real raf_radius(const Raf x);


/**
 * Computes \f$r = x +^\mathcal{Raf} y\f$.
 *
 * @param[out] r Result
 * @param[in] x First addendum
 * @param[in] y Second addendum
 */
void raf_add(Raf *r, const Raf x, const Raf y);


/**
 * Computes \f$x = x +^\mathcal{Raf} y\f$.
 *
 * @param[in,out] x First addendum
 * @param[in] y Second addendum
 */
void raf_add_in_place(Raf *x, const Raf y);


/**
 * Computes \f$x = x +^\mathcal{Raf} y\f$ when x is sparse.
 *
 * @param[in,out] x First addendum
 * @param[in] y Second addendum
 */
void raf_add_sparse_in_place(Raf *r, const Raf x_sparse);


/**
 * Computes \f$r = a -^\mathcal{Raf} y\f$.
 *
 * @param[out] r Result
 * @param[in] x Minuendum
 * @param[in] y Subtrahendum
 */
void raf_sub(Raf *r, const Raf x, const Raf y);


/**
 * Computes \f$r = x \cdot^\mathcal{Raf} y\f$.
 *
 * @param[out] r Result
 * @param[in] x First factor
 * @param[in] y Second factor
 */
void raf_mul(Raf *r, const Raf x, const Raf y);


/**
 * Computes \f$r = x^d\f$.
 *
 * @param[out] r Result
 * @param[in] x Base (RAF)
 * @param[in] d Natural exponent
 */
void raf_pow(Raf *r, const Raf x, const unsigned int d);


/**
 * Computes \f$r = x^2\f$.
 * 
 * @param[out] r Result
 * @param[in] x Base (RAF)
 */
void raf_sqr(Raf *r, const Raf x);


/**
 * Computes \f$r = e^x\f$.
 *
 * @param[out] r Result
 * @param[in] x Exponent (RAF)
 */
void raf_exp(Raf *r, const Raf x);


/**
 * Computes \f$r = x +^\mathcal{Raf} t\f$.
 *
 * @param[out] r Result
 * @param[in] x RAF
 * @param[in] t Translation length
 */
void raf_translate(Raf *r, const Raf x, const Real t);


/**
 * Computes \f$x = x +^\mathcal{Raf} t\f$.
 *
 * @param[in,out] x RAF
 * @param[in] t Translation length
 */
void raf_translate_in_place(Raf *x, const Real t);


/**
 * Computes \f$r = s \cdot^\mathcal{Raf} x\f$.
 *
 * @param[out] r Result
 * @param[in] x RAF
 * @param[in] s Scaling factor
 */
void raf_scale(Raf *r, const Raf x, const Real s);


/**
 * Computes fused multiply-add \f$r = (\alpha \cdot^\mathcal{Raf} x) +^\mathcal{Raf} y\f$.
 *
 * @param[out] r Result
 * @param[in] alpha Scaling factor
 * @param[in] x First addendum
 * @param[in] y Second addendum
 */
void raf_fma(Raf *r, const Real alpha, const Raf x, const Raf y);


/**
 * Computes fused multiply-add \f$y = (\alpha \cdot^\mathcal{Raf} x) +^\mathcal{Raf} y\f$.
 *
 * @param[in,out] y Result and second addendum
 * @param[in] alpha Scaling factor
 * @param[in] x Addendum
 */
void raf_fma_in_place(Raf *y, const Real alpha, const Raf x);


/**
 * Prints a RAF.
 *
 * @param[in,out] fh File pointer
 * @param[in] r RAF
 */
void raf_print(FILE *fp, const Raf r);

#endif
