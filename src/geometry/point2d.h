/**
 * Point in a bidimensional space.
 *
 * Defines a point in a bidimensional space.
 *
 * @file point2d.h
 */
#ifndef POINT2D
#define POINT2D

#include <stdio.h>

#include "../type.h"

/** Type of a 2D point. */
typedef struct point2d Point2D;

/**
 * Structure of a 2D point.
 */
struct point2d {
    Real x;  /**< First component. */
    Real y;  /**< Second component. */
};



/**
 * Copies a point.
 *
 * @param[out] r Destination point
 * @param[in] a Source point
 */
static inline void point2d_copy(Point2D *r, const Point2D a) {
    r->x = a.x;
    r->y = a.y;
}



/**
 * Computes \f$\vec{r} = \vec{a} + \vec{b}\f$.
 *
 * @param[out] r Result
 * @param[in] a First addendum
 * @param[in] b Second addendum
 */
static inline void point2d_add(Point2D *r, const Point2D a, const Point2D b) {
    r->x = a.x + b.x;
    r->y = a.y + b.y;
}



/**
 * Computes \f$\vec{r} = \vec{a} - \vec{b}\f$.
 *
 * @param[out] r Result
 * @param[in] a Minuend
 * @param[in] b Subtrahend
 */
static inline void point2d_sub(Point2D *r, const Point2D a, const Point2D b) {
    r->x = a.x - b.x;
    r->y = a.y - b.y;
}



/**
 * Computes \f$\vec{r} = s \vec{a}\f$.
 *
 * @param[out] r Result
 * @param[in] a Point
 * @param[in] s Scalig factor
 */
static inline void point2d_scale(Point2D *r, const Point2D a, const Real s) {
    r->x = s * a.x;
    r->y = s * a.y;
}



/**
 * Computes fused multiply-add: \f$\vec{r} = \alpha \vec{a} + \vec{b}\f$.
 *
 * @param[out] r Result
 * @param[in] alpha Scaling factor
 * @param[in] a First addendum
 * @param[in] b Second addendum
 */
static inline void point2d_fma(Point2D *r, const Real alpha, const Point2D a, const Point2D b) {
    r->x = alpha * a.x + b.x;
    r->y = alpha * a.y + b.y;
}



/**
 * Finds line between two points.
 *
 * @param[out] m Slope
 * @param[out] q Intercept
 * @param[in] a First point
 * @param[in] b Second point
 */
static inline void point2d_find_line(Real *m, Real *q, const Point2D a, const Point2D b) {
    *m = (b.y - a.y) / (b.x - a.x);
    *q = a.y - *m * a.x;
}



/**
 * Prints a points.
 *
 * @param[out] fd Output stream
 * @param[in] a Point
 */
static inline void point2d_print(FILE *fd, const Point2D a) {
    fprintf(fd, "(%f; %f)\n", a.x, a.y);
}

#endif
