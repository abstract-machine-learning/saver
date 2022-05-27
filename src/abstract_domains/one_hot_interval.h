/**
 * One Hot Interval Abstraction (OHInt).
 *
 * Defines abstract transfer functions for OHInts.
 *
 * @file one_hot_interval.h
 * @author Abhinandan Pal <abhinandan.mike123@gmail.com>
 */
#ifndef ONE_HOT_INTERVAL_H
#define ONE_HOT_INTERVAL_H

#include <stdio.h>
#include <stdbool.h>

#include "interval.h"

/** OHInt abstract value. */
typedef struct ohint OHInt;

/**
 * Structure of a OHInt.
 *
 * One Hot Interval Abstraction (OHInt) allows to maintain the constrains of 
 * one-hot encoding while using Interval abstraction in general. In one-hot encoding
 * in a tier only one of the values can be 1 and rest 0.
 * 
 * OHInt stores the value origniating from zero and that originating from one.
 * During the innerproduct (of poly kernal) or magnitude (of KBF kernal) step it obtains
 * an Interval for the tier by setting one of the features of the tier as 1 and rest as 0
 * so as to obtain the min and the max
 */
struct ohint {
    Real zero;       /* Store value originating from zero */
    Real one;        /* Store value originating from one */
};

/*
 * As Interval and ohint are structurally same. The current version doesnt Convert Interval to 
 * ohint but stores zero in l and one in u. [To Do: use an union type for it]
 */

void interval_to_ohint(const bool* isOneHot,short* origin,const Interval *y, const unsigned int space_size);




/**
 * @param[out] r Result
 * @param[in]  x One hot Interval
 * @param[in]  t Translation length
 */
void ohint_translate(Interval *r, const Interval x, const Real t);

/**
 * @param[out] r Result
 * @param[in]  x One hot Interval
 * @param[in]  s Scale magnitude
 */
void ohint_scale(Interval *r, const Interval x, const Real s);

/**
 * @param[out] r Result
 * @param[in]  x One hot Interval
 */
void ohint_exponent(Interval *r, const Interval x);

/**
 * @param[out] r Result
 * @param[in]  x One hot Interval
 * @param[in]  degree power degree
 */
void ohint_pow(Interval *r, const Interval x, const unsigned int degree);

/**
 * @param[out] r Result
 * @param[in]  x One hot Interval
 * @param[in]  tier_size No of features in current tier
 * Obtain An Interval by merging all the one hot interval in current tier.
 */
void ohint_intervalize(Interval *r,const Interval* x,const short* origin,const unsigned int tier_size);

#endif