/**
 * Controls type of numerical values used in the program.
 * 
 * Allows to switch among float, double and long double.
 * 
 * @file type.h
 * @author Marco Zanella <marco.zanella.1991@gmail.com>
 */
#ifndef TYPE_H
#define TYPE_H

#include <math.h>

#ifdef ENFORCE_SOUNDNESS
    /** Sets floating point rounding mode to \f$\rightarrow -\infty\f$. */
    #define round_down fesetround(FE_DOWNWARD)

    /** Sets floating point rounding mode to \f$\rightarrow +\infty\f$. */
    #define round_up   fesetround(FE_UPWARD)
#else
    /** Sets floating point rounding mode to \f$\rightarrow -\infty\f$. */
    #define round_down

    /** Sets floating point rounding mode to \f$\rightarrow +\infty\f$. */
    #define round_up
#endif


/**
 * Type of a real number.
 */
#ifdef PRECISION_DOUBLE
typedef double Real;

#elif PRECISION_LONG_DOUBLE
typedef long double Real;

#else
typedef float Real;

#endif


/**
 * Returns \f$\min(x, y)\f$.
 *
 * @param[in] x First operand
 * @param[in] y Second operand
 * @return Minimum between x and y
 * @note This function implement a call-by-name strategy
 */
#define min(x, y) \
    ((x) <= (y) ? (x) : (y))


/**
 * Returns \f$\max(x, y)\f$.
 *
 * @param[in] x First operand
 * @param[in] y Second operand
 * @return Maximum between x and y
 * @note This function implement a call-by-name strategy
 */
#define max(x, y) \
    ((x) >= (y) ? (x) : (y))

#endif
