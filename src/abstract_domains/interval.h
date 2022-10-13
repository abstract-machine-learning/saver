/**
 * Intervals.
 *
 * Defines abstract transfer functions for intervals.
 *
 * @file interval.h
 *
 */
#ifndef INTERVAL_H
#define INTERVAL_H

#include "../type.h"

/**
 * Type of an interval.
 */
typedef struct interval Interval;


/**
 * Structure of an interval.
 */
struct interval {
    Real l;  /**< Lowerbound. */
    Real u;  /**< Upperbound. */
};



/**
 * Returns center of an interval.
 *
 * @param[in] x Interval
 * @return Center of given interval
 */
static inline Real interval_midpoint(const Interval x) {
    return (x.l + x.u) * 0.5;
}



/**
 * Returns radius of an interval.
 *
 * @param[in] x Interval
 * @return Radius of given interval
 */
static inline Real interval_radius(const Interval x) {
    return (x.u - x.l) * 0.5;
}



/**
 * Computes \f$r = x +^{\mathcal{Int}} y\f$.
 *
 * @param[out] r Result
 * @param[in] x First addendum
 * @param[in] y Second addendum
 */
static inline void interval_add(Interval *r, const Interval x, const Interval y) {
    round_down;
    r->l = x.l + y.l;
    round_up;
    r->u = x.u + y.u;
}



/**
 * Computes \f$r = x -^\mathcal{Int} y\f$.
 *
 * @param[out] r Result
 * @param[in] x Minuendum
 * @param[in] y Subtrahendum
 */
static inline void interval_sub(Interval *r, const Interval x, const Interval y) {
    round_down;
    r->l = x.l - y.l;
    round_up;
    r->u = x.u - y.u;
}



/**
 * Computes \f$r = x \cdot^\mathcal{Int} y\f$.
 *
 * @param[out] r Result
 * @param[in] x First factor
 * @param[in] y Second factor
 */
static inline void interval_mul(Interval *r, const Interval x, const Interval y) {
    if ((x.l == 0.0 && x.u == 0.0) || (y.l == 0.0 && y.u == 0.0)) {
        r->l = 0.0;
        r->u = 0.0;
        return;
    }

    if (x.l >= 0.0) {
        if (y.l >= 0.0) {
            round_down;
            r->l = x.l * y.l;
            round_up;
            r->u = x.u * y.u;
        }

        else if (y.u <= 0.0) {
            round_down;
            r->l = x.u * y.l;
            round_up;
            r->u = x.l * y.u;
        }

        else {
            round_down;
            r->l = x.u * y.l;
            round_up;
            r->u = x.u * y.u;
        }
    }

    else if (x.u <= 0.0) {
        if (y.l >= 0.0) {
            round_down;
            r->l = x.l * y.u;
            round_up;
            r->u = x.u * y.l;
        }
        else if (y.u <= 0.0) {
            round_down;
            r->l = x.u * y.u;
            round_up;
            r->u = x.l * y.l;
        }
        else {
            round_down;
            r->l = x.l * y.u;
            round_up;
            r->u = x.l * y.l;
        }
    }

    else {
        if (y.l >= 0.0) {
            round_down;
            r->l = x.l * y.u;
            round_up;
            r->u = x.u * y.u;
        }
        else if (y.u <= 0.0) {
            round_down;
            r->l = x.u * y.l;
            round_up;
            r->u = x.l * y.l;
        }
        else {
            round_down;
            r->l = min(x.l * y.u, x.u * y.l);
            round_up;
            r->u = max(x.l * y.l, x.u * y.u);
        }
    }
}



/**
 * Computes \f$r = x^{degree}\f$.
 *
 * @param[out] r Result
 * @param[in] x Base (interval)
 * @param[in] degree Natural exponent
 */
static inline void interval_pow(Interval *r, const Interval x, const unsigned int degree) {
    unsigned int i;

    *r = x;
    for (i = 1; i < degree; ++i) {
        interval_mul(r, *r, x);
    }
//printf("POW [%f,%f] --> [%f,%f] \n",x.l,x.u,r->l,r->u);
}



/**
 * Computes \f$r = e^x\f$.
 *
 * @param[out] r Result
 * @param[in] x Exponent (interval)
 */
static inline void interval_exp(Interval *r, const Interval x) {
    round_down;
    r->l = exp(x.l);
    round_up;
    r->u = exp(x.u);
}



/**
 * Computes \f$r = x +^\mathcal{Int} t\f$.
 *
 * @param[out] r Result
 * @param[in] x Interval
 * @param[in] t Translation length
 */
static inline void interval_translate(Interval *r, const Interval x, const Real t) {
    round_down;
    r->l = x.l + t;
    round_up;
    r->u = x.u + t;
}



/**
 * Computes \f$r = s \cdot^\mathcal{Int} x\f$.
 *
 * @param[out] r Result
 * @param[in] x Interval
 * @param[in] s Scaling factor
 */
static inline void interval_scale(Interval *r, const Interval x, const Real s) {
    if (s >= 0.0) {
        round_down;
        r->l = s * x.l;
        round_up;
        r->u = s * x.u;
    }
    else {
        round_down;
        r->l = s * x.u;
        round_up;
        r->u = s * x.l;
    }
}



/**
 * Coputes fused multiply-add \f$r = (\alpha \cdot^\mathcal{Int} x) +^\mathcal{Int} y\f$.
 *
 * @param[out] r Result
 * @param[in] alpha Scaling factor
 * @param[in] x First addendum
 * @param[in] y Second addendum
 */
static inline void interval_fma(Interval *r, const Real alpha, const Interval x, const Interval y) {
    if (alpha >= 0) {
        round_down;
        r->l = alpha * x.l + y.l;
        round_up;
        r->u = alpha * x.u + y.u;
    }
    else {
        round_down;
        r->l = alpha * x.u + y.l;
        round_up;
        r->u = alpha * x.l + y.u;
    }
}

#endif
