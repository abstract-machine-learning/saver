#include "kernel.h"

#include <malloc.h>
#include "type.h"
#include "report_error.h"

/**
 * Structure of a kernel.
 */
struct kernel {
    KernelType type;      /**< Type of kernel. */
    Real gamma;           /**< Gamma (for RBF kernel). */
    unsigned int degree;  /**< Degree (for polynomial kernel). */
    Real c;               /**< Unhomogenous term (for polynomial kernel). */
};



static Real kernel_compute_linear(
    const Real *x,
    const Real *y,
    const unsigned int size
) {
    unsigned int i;
    Real sum = 0.0;

    for (i = 0; i < size; ++i) {
        sum += x[i] * y[i];
    }

    return sum;
}



static Real kernel_compute_rbf(
    const Real *x,
    const Real *y,
    const unsigned int size,
    const Real gamma
) {
    unsigned int i;
    Real sum = 0.0;

    for (i = 0; i < size; ++i) {
        sum += (x[i] - y[i]) * (x[i] - y[i]);
    }

    return exp(-gamma * sum);
}



static Real kernel_compute_polynomial(
    const Real *x,
    const Real *y,
    const unsigned int size,
    unsigned int degree,
    const Real c
) {
    return pow(kernel_compute_linear(x, y, size) + c, degree);
}



Kernel kernel_create(const KernelType type, const Real gamma, const unsigned int degree, const Real c) {
    Kernel kernel = (Kernel) malloc(sizeof(struct kernel));
    if (kernel == NULL) {
        report_error("Memory exhaustion.");
    }

    kernel->type = type;
    kernel->gamma = gamma;
    kernel->degree = degree;
    kernel->c = c;

    return kernel;
}



void kernel_delete(Kernel *kernel) {
    if (kernel == NULL || *kernel == NULL) {
        return;
    }

    free(*kernel);
    *kernel = NULL;
}



KernelType kernel_get_type(const Kernel kernel) {
    if (kernel == NULL) {
        report_error("Null pointer.");
    }

    return kernel->type;
}



Real kernel_get_gamma(const Kernel kernel) {
    if (kernel == NULL) {
        report_error("Null pointer.");
    }

    return kernel->gamma;
}



unsigned int kernel_get_degree(const Kernel kernel) {
    if (kernel == NULL) {
        report_error("Null pointer.");
    }

    return kernel->degree;
}



Real kernel_get_c(const Kernel kernel) {
    if (kernel == NULL) {
        report_error("Null pointer.");
    }

    return kernel->c;
}



Real kernel_compute(const Kernel kernel, const Real *x, const Real *y, const unsigned int size) {
    switch (kernel->type) {
        case KERNEL_LINEAR: return kernel_compute_linear(x, y, size);
        case KERNEL_RBF: return kernel_compute_rbf(x, y, size, kernel->gamma);
        case KERNEL_POLYNOMIAL: return kernel_compute_polynomial(x, y, size, kernel->degree, kernel->c);
    }

    report_error("Unrecognized kernel type.");
}
