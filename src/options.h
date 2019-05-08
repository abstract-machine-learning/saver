/**
 * Defines program options.
 *
 * Reads and sets program options.
 *
 * @file options.h
 * @author Marco Zanella <marco.zanella.1991@gmail.com>
 */
#ifndef OPTIONS_H
#define OPTIONS_H

/** Type of program options. */
typedef struct options Options;


/**
 * Structure of program options.
 */
struct options {
    char *counterexamples_file;  /**< Path to file in which store counterexamples */
    unsigned int debug_output;   /**< Prints verbose debug information if set to 1 */
};



/**
 * Reads additional command-line options.
 *
 * @param[out] options Options
 * @param[in] argc ARGument Counter
 * @param[in] argv ARGument Vector
 */
void read_options(Options *options, const int argc, const char *argv[]);

#endif
