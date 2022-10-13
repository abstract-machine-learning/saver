/**
 * Attempts to solve a combinatorial optimization problem through a
 * genetic algorithm.
 *
 * @file genetic_algorithm.h
 * 
 */
#ifndef GENETIC_ALGORITHM_H
#define GENETIC_ALGORITHM_H

/** Type of an individual. */
typedef void *GeneticAlgorithmIndividual;


/** Type of a population. */
typedef GeneticAlgorithmIndividual *GeneticAlgorithmPopulation;


/** Type of a genetic algorithm status. */
typedef struct genetic_algorithm_status GeneticAlgorithmStatus;


/** Type of a population generation function. */
typedef void (*GeneticAlgorithmPopulationGenerator)(
    GeneticAlgorithmPopulation,
    const unsigned int
);


/** Type of a fitness function. */
typedef double (*GeneticAlgorithmFitness)(
    const GeneticAlgorithmIndividual,
    const GeneticAlgorithmStatus
);


/** Type of a population size function. */
typedef unsigned int (*GeneticAlgorithmNextGenerationSize)(
    const GeneticAlgorithmStatus
);


/** Type of an elitism function. */
typedef unsigned int (*GeneticAlgorithmElitism)(
    const GeneticAlgorithmStatus
);


/** Type of a parent selection function. */
typedef GeneticAlgorithmIndividual (*GeneticAlgorithmSelect)(
    const GeneticAlgorithmStatus
);


/** Type of a crossover function. */
typedef void (*GeneticAlgorithmCrossover)(
    GeneticAlgorithmIndividual,
    const GeneticAlgorithmIndividual,
    const GeneticAlgorithmIndividual,
    const GeneticAlgorithmStatus
);


/** Type of a mutation pobability function. */
typedef double (*GeneticAlgorithmMutationProbability)(
    const GeneticAlgorithmStatus
);


/** Type of a mutation function. */
typedef void (*GeneticAlgorithmMutation)(
    GeneticAlgorithmIndividual,
    const GeneticAlgorithmStatus
);


/** Type of a predicate telling whether an individual is a solution. */
typedef unsigned int (*GeneticAlgorithmIsSolution)(
    const GeneticAlgorithmIndividual,
    const GeneticAlgorithmStatus
);


/** Type of a callback function to be called after every genration cycle. */
typedef void (*GeneticAlgorithmCallback)(const GeneticAlgorithmStatus);


/** Structure of a genetic algorithm status. */
struct genetic_algorithm_status {
    unsigned int population_size;      /**< Current population size. */
    unsigned int max_population_size;  /**< Maximum population size. */
    unsigned int generation;           /**< Current generation cycle. */
    unsigned int max_generation;       /**< Maximum generation cycles. */
    unsigned int elitism;              /**< Number of current elite individuals. */
    unsigned int genes;                /**< Number of genes for each individual. */
    unsigned int individual_size;      /**< Size of an individual. */
    GeneticAlgorithmPopulation population;  /**< Current population. */
    double *fitness;                   /**< Array of fitness value for each individual. */
    double fitness_mean;               /**< Mean fitness. */
    double fitness_variance;           /**< Fitness variance. */
    double *normalized_fitness;        /**< Array of fitness (normalized). */
    void *data;                        /**< Custom data. */
    void *buffer;                      /**< Internal buffer. */
};



/**
 * Returns a constant, default size for a next generation.
 *
 * @param[in] status Current genetic algorithm status
 * @return Size of next generation of the population
 */
unsigned int genetic_algorithm_next_size_constant(
    const GeneticAlgorithmStatus status
);


/**
 * Returns a constant fraction of the current population size.
 *
 * @param[in] status Current genetic algorithm status
 * @return Number of elite individuals
 */
unsigned int genetic_algorithm_elitism_constant(
    const GeneticAlgorithmStatus status
);


/**
 * Selects a parent individual with unifom distribution.
 *
 * Fitness of individuals is not taken into account.
 *
 * @param[in] status Current genetic algorithm status
 * @return Chosen individual
 */
GeneticAlgorithmIndividual genetic_algorithm_select_uniform(
    const GeneticAlgorithmStatus status
);


/**
 * Selects a parent individual using the standard roulette wheel algorithm.
 *
 * @param[in] status Current genetic algoritm status
 * @return Chosen individual
 */
GeneticAlgorithmIndividual genetic_algorithm_select_roulette_wheel(
    const GeneticAlgorithmStatus status
);


/**
 * Generates a new individual whose genes are the average of its parents.
 *
 * @param[out] offspring Offspring individual
 * @param[in] parent_a First parent
 * @param[in] parent_b Second parent
 * @param[in] status Current genetic algorithm status
 * @note This function assumes individuals to be represented as arrays.
 */
void genetic_algorithm_crossover_average(
    GeneticAlgorithmIndividual offspring,
    const GeneticAlgorithmIndividual parent_a,
    const GeneticAlgorithmIndividual parent_b,
    const GeneticAlgorithmStatus status
);


/**
 * Generates a new individual using the 1-point crossover.
 *
 * @param[out] offspring Offspring individual
 * @param[in] parent_a First parent
 * @param[in] parent_b Second parent
 * @param[in] status Current genetic algorithm status
 * @note This function assumes individuals to be represented as arrays.
 */
void genetic_algorithm_crossover_one_point(
    GeneticAlgorithmIndividual offspring,
    const GeneticAlgorithmIndividual parent_a,
    const GeneticAlgorithmIndividual parent_b,
    const GeneticAlgorithmStatus status
);


/**
 * Generates a new individual using the 2-points crossover.
 *
 * @param[out] offspring Offspring individual
 * @param[in] parent_a First parent
 * @param[in] parent_b Second parent
 * @param[in] status Current genetic algorithm status
 * @note This function assumes individuals to be represented as arrays.
 */
void genetic_algorithm_crossover_two_points(
    GeneticAlgorithmIndividual offspring,
    const GeneticAlgorithmIndividual parent_a,
    const GeneticAlgorithmIndividual parent_b,
    const GeneticAlgorithmStatus status
);


/**
 * Returns a constant probability for a mutation to occur.
 *
 * @param[in] status Current genetic algorithm status
 * @return Constant probability
 */
double genetic_algorithm_mutation_probability_constant(
    const GeneticAlgorithmStatus status
);


/**
 * Mutates an individual by swapping two genes of its.
 *
 * @param[in,out] offspring Individual to mutate
 * @param[in] status Current genetic algorithm status
 * @note This function assumes individuals to be represented as arrays.
 */ 
void genetic_algorithm_mutation_swap(
    GeneticAlgorithmIndividual offspring,
    const GeneticAlgorithmStatus status
);


/**
 * Simple callback function which displays a summary of current generation.
 *
 * @param[in] status Current genetic algorithm status
 */
void genetic_algorithm_callback_simple(const GeneticAlgorithmStatus status);


/**
 * Deploys a genetic algorithm to solve a combinatorial problem.
 *
 * @param[out] solution Solution, if found
 * @param[in] genes Number of genes of an individual
 * @param[in] individual_size Size of an individual, in bytes
 * @param[in] population_generator Generator of initial population
 * @param[in] fitness Evaluates fitness of an individual
 * @param[in] max_population_size Maximum size of population
 * @param[in] max_generation Maximum allowed generations
 * @param[in] next_generation_size Determines size of population for the
 *                                 next generation
 * @param[in] elitism Determines number of elite individuals for next
 *                    generation
 * @param[in] select_parent Parent selection function
 * @param[in] crossover Crossover function
 * @param[in] mutation_probability Returns probability of a mutation to occur
 * @param[in] mutation Mutation function
 * @param[in] is_solution Preticate telling whether an individual is
 *                        a solution
 * @param[in] callback Callback function to call at the end of every
 *                     generation, ignored if set to NULL
 * @param[in] data Additional data to be injected into the status
 * @return 1 if a solution is found, 0 otherwise
 */
unsigned int genetic_algorithm(
    GeneticAlgorithmIndividual solution,
    const unsigned int genes,
    const unsigned int individual_size,
    const GeneticAlgorithmPopulationGenerator population_generator,
    const GeneticAlgorithmFitness fitness,
    const unsigned int max_population_size,
    const unsigned int max_generation,
    const GeneticAlgorithmNextGenerationSize next_generation_size,
    const GeneticAlgorithmElitism elitism,
    const GeneticAlgorithmSelect select_parent,
    const GeneticAlgorithmCrossover crossover,
    const GeneticAlgorithmMutationProbability mutation_probability,
    const GeneticAlgorithmMutation mutation,
    const GeneticAlgorithmIsSolution is_solution,
    const GeneticAlgorithmCallback callback,
    const void *data
);

#endif
