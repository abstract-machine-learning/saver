/**
 * Error reporting utility.
 * 
 * Defines function to report errors and warnings.
 * 
 * @file report_error.h
 * @author Marco Zanella <marco.zanella.1991@gmail.com>
 */
#ifndef REPORT_ERROR_H
#define REPORT_ERROR_H

#include <stdlib.h>

/**
 * Reports a warning.
 * 
 * Prints a warning on standard error, showing involved file and line.
 * 
 * @param[in] message Message to display
 */
#define report_warning(message) \
    fprintf(stderr, "[%s: %d] WARNING: %s\n", __FILE__, __LINE__, (message))


/**
 * Reports an error and stops computation.
 * 
 * Prints an error on standard error, showing involved file and line
 * and halting the computation with EXIT_FAILURE.
 * 
 * @param[in] message Message to display
 */
#define report_error(message) \
    fprintf(stderr, "[%s: %d] ERROR: %s\n", __FILE__, __LINE__, (message)); \
    exit(EXIT_FAILURE)

#endif
