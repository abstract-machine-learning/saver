#include "raf.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "../geometry/point2d.h"

#ifndef RAF_MUL_ALGO
#define RAF_MUL_ALGO raf_mul_algo3
#endif

#define RAF_DENSE -1



/***********************************************************************
 * Alternative multiplication functions.
 **********************************************************************/
void raf_mul_algo1(Raf *r, const Raf x, const Raf y) {
    unsigned int i;
    Real x_norm_one = 0.0,
         y_norm_one = 0.0;

    r->c = x.c * y.c;
    for (i = 0; i < min(x.size, y.size); ++i) {
        x_norm_one += fabs(x.noise[i]);
        y_norm_one += fabs(y.noise[i]);
        r->noise[i] = y.c * x.noise[i] + x.c * y.noise[i];
    }
    r->delta = fabs(y.c) * x.delta
             + fabs(x.c) * y.delta
             + (x_norm_one + x.delta) * (y_norm_one + y.delta);
}



void raf_mul_algo2(Raf *r, const Raf x, const Raf y) {
    unsigned int i;
    Real x_norm_one = 0.0,
         y_norm_one = 0.0,
         xy = 0.0,
         xy_abs = 0.0;

    for (i = 0; i < min(x.size, y.size); ++i) {
        xy += x.noise[i] * y.noise[i];
        xy_abs += fabs(x.noise[i] * y.noise[i]);
        x_norm_one += fabs(x.noise[i]);
        y_norm_one += fabs(y.noise[i]);
        r->noise[i] = y.c * x.noise[i] + x.c * y.noise[i];
    }
    r->c = x.c * y.c + 0.5 * xy;
    r->delta = fabs(y.c) * x.delta
             + fabs(x.c) * y.delta
             + (x_norm_one + x.delta) * (y_norm_one + y.delta)
             - 0.5 * xy_abs;
}



static int algo3_cmp(const void *x, const void *y) {
    const Point2D a = *((const Point2D *) x),
                  b = *((const Point2D *) y);
    const Real r = a.y / a.x - b.y / b.x;

    return r > 0.0
           ? +1
           : (r < 0.0 ? -1 : 0);
}

void raf_mul_algo3(Raf *r, const Raf x, const Raf y) {
    unsigned int i, j, m = 0, has_double_min = 0, has_double_max = 0;
    const unsigned int n = x.size;
    Real R_min, R_max, x_norm_one = 0.0, sgn_x_y = 0.0, obj_min, obj_max;
    Real *X = (Real *) malloc((n + 2) * sizeof(Real)),
         *Y = (Real *) malloc((n + 2) * sizeof(Real));
    Point2D w, w_acc, w_min1, w_min2 = {0, 0}, w_max1, w_max2 = {0, 0}, *h;

    /* Computes R_min and R_max */
    for (i = 0; i < n; ++i) {
        X[i] = x.noise[i];
        Y[i] = x.noise[i] != 0 ? y.noise[i] : fabs(y.noise[i]);
        x_norm_one += fabs(X[i]);
        sgn_x_y += (X[i] >= 0.0 ? Y[i] : -Y[i]);
        m += X[i] != 0.0;
    }
    X[n] = x.delta;
    Y[n] = 0.0;
    x_norm_one += fabs(X[n]);
    m += X[n] != 0.0;
    X[n + 1] = 0.0;
    Y[n + 1] = fabs(y.delta);

    h = (Point2D *) malloc(m * sizeof(Point2D));
    for (i = 0, j = 0; i < n + 2; ++i) {
        if (X[i] == 0.0) {
            continue;
        }

        h[j].x = fabs(X[i]);
        h[j].y = X[i] > 0.0 ? Y[i] : -Y[i];
        ++j;
    }

    qsort(h, m, sizeof(Point2D), algo3_cmp);

    w.x = x_norm_one;
    w.y = sgn_x_y;
    point2d_copy(&w_acc, w);
    point2d_copy(&w_min1, w);
    obj_min = w_min1.x * w_min1.y;
    point2d_copy(&w_max1, w);
    obj_max = w_max1.x * w_max1.y;
    for (i = 0; i < m; ++i) {
        Real obj;

        point2d_fma(&w_acc, -2.0, h[i], w_acc);
        obj = w_acc.x * w_acc.y;

        if (obj > obj_max) {
            obj_max = obj;
            point2d_copy(&w_max1, w_acc);
            has_double_max = 0;
        }
        else if (obj == obj_max) {
            point2d_copy(&w_max2, w_acc);
            has_double_max = 1;
        }

        if (obj < obj_min) {
            obj_min = obj;
            point2d_copy(&w_min1, w_acc);
            has_double_min = 0;
        }
        else if (obj == obj_min) {
            point2d_copy(&w_min2, w_acc);
            has_double_min = 1;
        }
    }

    if (!has_double_max) {
        R_max = obj_max;
    }
    else {
        Real m, q, p_x;
        point2d_find_line(&m, &q, w_max1, w_max2);
        p_x = -0.5 * q / m;
        if (min(w_max1.x, w_max2.x) < p_x && p_x < max(w_max1.x, w_max2.x)) {
            R_max = max(obj_max, -0.25 * q * q / m);
        }
        else {
            R_max = obj_max;
        }
    }

    if (!has_double_min) {
        R_min = obj_min;
    }
    else {
        Real m, q, p_x;
        point2d_find_line(&m, &q, w_min1, w_min2);
        p_x = -0.5 * q / m;
        if (min(w_max1.x, w_max2.x) < p_x && p_x < max(w_max1.x, w_max2.x)) {
            R_min = min(obj_min, -0.25 * q * q / m);
        }
        else {
            R_min = obj_min;
        }
    }

    free(X);
    free(Y);
    free(h);


    /* Computes result */
    r->c = x.c * y.c + 0.5 * (R_min + R_max);
    for (i = 0; i < n; ++i) {
        r->noise[i] = y.c * x.noise[i] + x.c * y.noise[i];
    }
    r->delta = fabs(y.c) * x.delta
             + fabs(x.c) * y.delta
             + 0.5 * (R_max - R_min);
}



/***********************************************************************
 * Reduced Affine Form (RAF) operations.
 **********************************************************************/

void raf_create(Raf *r, const unsigned int size) {
    unsigned int i;

    r->c = 0.0;
    r->noise = (Real *) malloc(size * sizeof(Real));
    r->delta = 0.0;
    r->size = size;
    r->index = RAF_DENSE;

    for (i = 0; i < size; ++i) {
        r->noise[i] = 0.0;
    }
}



void raf_delete(Raf *r) {
    free(r->noise);
}



void raf_singleton(Raf *r, const Real value) {
    unsigned int i;

    r->c = value;
    for (i = 0; i < r->size; ++i) {
        r->noise[i] = 0.0;
    }
    r->delta = 0.0;
}



void raf_copy(Raf *r, const Raf x) {
    r->c = x.c;
    memmove(r->noise, x.noise, x.size * sizeof(Real));
    r->delta = x.delta;
    r->index = x.index;
}



void raf_to_interval(Interval *r, const Raf x) {
    const Real midpoint = raf_midpoint(x),
               radius = raf_radius(x);
    round_down;
    r->l = midpoint - radius;
    round_up;
    r->u = midpoint + radius;

}  



void interval_to_raf(Raf *r, const Interval x) {
    unsigned int i;

    r->c = interval_midpoint(x);
    r->delta = interval_radius(x);
    for (i = 0; i < r->size; ++i) {
        r->noise[i] = 0.0;
    }
    r->index = RAF_DENSE;
}



Real raf_midpoint(const Raf x) {
    return x.c;
}



Real raf_radius(const Raf x) {
    unsigned int i;
    Real radius = x.delta;

    round_up;
    for (i = 0; i < x.size; ++i) {
        radius += fabs(x.noise[i]);
    }

    return radius;
}



void raf_add(Raf *r, const Raf x, const Raf y) {
    unsigned int i;
    const unsigned int size = min(r->size, min(x.size, y.size));

    r->c = x.c + y.c;
    for (i = 0; i < size; ++i) {
        r->noise[i] = x.noise[i] + y.noise[i];
    }
    r->delta = x.delta + y.delta;
}



void raf_add_in_place(Raf *r, const Raf x) {
    unsigned int i;

    r->c += x.c;
    for (i = 0; i < x.size; ++i) {
        r->noise[i] += x.noise[i];
    }
    r->delta += x.delta;
}



void raf_add_sparse_in_place(Raf *r, const Raf x_sparse) {
    r->c += x_sparse.c;
    r->noise[x_sparse.index] += x_sparse.noise[0];
    r->delta += x_sparse.delta;
}



void raf_sub(Raf *r, const Raf x, const Raf y) {
    unsigned int i;
    const unsigned int size = min(r->size, min(x.size, y.size));

    r->c = x.c - y.c;
    for (i = 0; i < size; ++i) {
        r->noise[i] = x.noise[i] - y.noise[i];
    }
    r->delta = x.delta + y.delta;
}



void raf_mul(Raf *r, const Raf x, const Raf y) {
    RAF_MUL_ALGO(r, x, y);
}



void raf_pow(Raf *r, const Raf x, const unsigned int d) {
    unsigned int i;

    raf_copy(r, x);
    for (i = 1; i < d / 2; ++i) {
        raf_mul(r, *r, x);
    }

    raf_mul(r, *r, *r);
    if (d % 2) {
        raf_mul(r, *r, x);
    }
}



void raf_sqr(Raf *r, const Raf x) {
    if (x.index >= 0) {
        r->c = x.c * x.c;
        r->delta = x.noise[0] * x.noise[0];
        r->noise[0] = 2 * x.c * x.noise[0];
        r->index = x.index;
        return;
    }

    raf_mul(r, x, x);
}



void raf_exp(Raf *r, const Raf x) {
    Interval X;
    Real e_a, e_b, w, alpha, delta, zeta, d_min, d_max, d_a, d_b, a, b;
    unsigned int i;

    raf_to_interval(&X, x);
    /* Returns a singleton if interval is a singleton */
    if (X.l == X.u) {
        r->c = exp(X.l);
        for (i = 0; i < x.size; ++i) {
            r->noise[i] = 0.0;
        }
        r->delta = 0.0;
        return;
    }


    /* Computes Chebishev's approximation */
    e_b = exp(X.u);
    w = fabs(X.u - X.l);
    e_a = exp(X.l);
    alpha = (e_b - e_a) / w;

    if (alpha == 0) {
        d_min = e_a;
        d_max = e_b;
    }
    else if (alpha >= e_b) {
        d_min = exp(X.u) - alpha * X.u;
        d_max = exp(X.l) - alpha * X.l;
    }
    else {
        d_a = exp(X.l) - alpha * X.l;
        d_b = e_b - alpha * X.u;
        d_min = alpha * (1 - log(alpha));
        d_max = max(d_a, d_b);
    }
    zeta = (d_min + d_max) * 0.5;
    delta = (d_max - d_min) * 0.5;


    /* Computes affine form */
    r->c = alpha * x.c + zeta;
    a = alpha * x.c + zeta;
    b = alpha * x.c + zeta;
    delta += max(b - r->c, r->c - a);
    for (i = 0; i < x.size; ++i) {
        r->noise[i] = alpha * x.noise[i];
        a = alpha * x.noise[i];
        b = alpha * x.noise[i];
        delta += max(b - r->noise[i], a - r->noise[i]);
    }
    r->delta = alpha * x.delta + delta;
}



void raf_translate(Raf *r, const Raf x, const Real t) {
    r->c = x.c + t;
    memmove(r->noise, x.noise, x.size * sizeof(Real));
    r->delta = x.delta;
    r->index = x.index;
}



void raf_translate_in_place(Raf *r, const Real t) {
    r->c += t;
}



void raf_scale(Raf *r, const Raf x, const Real s) {
    unsigned int i;

    r->c = s * x.c;
    for (i = 0; i < x.size; ++i) {
        r->noise[i] = s * x.noise[i];
    }
    r->delta = fabs(s * x.delta);
    r->index = x.index;
}



void raf_fma(Raf *r, const Real alpha, const Raf x, const Raf y) {
    unsigned int i;
    const unsigned int size = r->size;

    /* Avoids multiplication if alpha is 0 */
    if (alpha == 0.0) {
        raf_copy(r, y);
        return;
    }


    r->c = alpha * x.c + y.c;
    r->delta = fabs(alpha * x.delta) + y.delta;

    /* Computes a single noise if x has a single noise */
    if (x.index >= 0) {
        memmove(r->noise, y.noise, size * sizeof(Real));
        r->noise[x.index] += alpha * x.noise[0];
        return;
    }

    /* Computes every noise otherwise */
    for (i = 0; i < size; ++i) {
        r->noise[i] = alpha * x.noise[i] + y.noise[i];
    }
}



void raf_fma_in_place(Raf *r, const Real alpha, const Raf x) {
    unsigned int i;
    /* Avoids computation if alpha is 0 */
    if (alpha == 0.0) {
        return;
    }

    r->c += alpha * x.c;
    r->delta += fabs(alpha * x.delta);
    /* Computes a single noise if x has a single noise */
    if (x.index >= 0) {
        r->noise[x.index] += alpha * x.noise[0];
        return;
    }
    /* Computes every noise otherwise */
    for (i = 0; i < x.size; ++i) {
        r->noise[i] += alpha * x.noise[i];
    }
}



void raf_print(FILE *fp, const Raf r) {
    unsigned int i;

    fprintf(fp, "%.2g", r.c);

    if (r.index >= 0) {
        fprintf(fp, " + %.2g*e_%u", r.noise[0], r.index);
    }
    else {
        for (i = 0; i < r.size; ++i) {
            fprintf(fp, " + %.2g*e_%u", r.noise[i], i);
        }
    }

    fprintf(fp, " + %.2g*e_r", r.delta);
}
