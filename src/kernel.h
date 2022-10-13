/**
 * Kernel function.
 * 
 * Defines a kernel function.
 * 
 * @file kernel.h
 * 
 */
#ifndef KERNEL_H
#define KERNEL_H

#include "type.h"

/**
 * Available types of kernel.
 */
typedef enum {
    KERNEL_LINEAR,     /**< Linear kernel: \f$ K(x, y) = x \cdot y \f$. */
    KERNEL_RBF,        /**< Radial-Basis-Function kernel: \f$ K(x, y) = e^{-\gamma \| x - y \|^2}\f$. */
    KERNEL_POLYNOMIAL  /**< Polynomial kernel: \f$K(x, y) = (x \cdot y + c)^d\f$. */
} KernelType;

static inline char *kernelTypeStr(KernelType k)
{
    static char *strings[] = { "LINEAR", "RBF", "POLYNOMIAL"};
    return strings[k];
}

/**
 * Type of a kernel.
 */
typedef struct kernel *Kernel;


/**
 * Creates a kernel function.
 * 
 * @param[in] type   Kernel type
 * @param[in] gamma  Gamma parameter (only relevant for RBF)
 * @param[in] degree Degree of polynomial (only relevant for polynomial)
 * @param[in] c      Unhomogenous parameter (only relevant for polynomial)
 * @return Kernel function
 */
Kernel kernel_create(const KernelType type, const Real gamma, unsigned int degree, const Real c);


/**
 * Deletes a kernel function.
 * 
 * @param[out] kernel Pointer to kernel function
 */
void kernel_delete(Kernel *kernel);


/**
 * Returns type of given kernel.
 * 
 * @param[in] kernel Kernel
 * @return Type of kernel
 */
KernelType kernel_get_type(const Kernel kernel);


/**
 * Returns gamma parameter of given kernel.
 * 
 * @param[in] kernel Kernel
 * @return Gamma parameter
 */
Real kernel_get_gamma(const Kernel kernel);


/**
 * Returns degree of given kernel.
 * 
 * @param[in] kernel Kernel
 * @return Degree
 */
unsigned int kernel_get_degree(const Kernel kernel);


/**
 * Returns unhomogenous parameter of given kernel.
 * 
 * @param[in] kernel Kernel
 * @return Unhomogenous parameter
 */
Real kernel_get_c(const Kernel kernel);


/**
 * Computes kernel given function on two vectors.
 * 
 * @param[in] kernel Kernel
 * @param[in] x      First vector
 * @param[in] y      Second vector
 * @param[in] size   Size of the vector space
 * @return Result of the kernel evaluated on x and y
 */
Real kernel_compute(const Kernel kernel, const Real* x, const Real* y, const unsigned int size);

#endif
