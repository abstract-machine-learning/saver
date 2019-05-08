#include "genetic_algorithm.h"

#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>

#ifndef DEFAULT_POPULATION_SIZE
#define DEFAULT_POPULATION_SIZE 32
#endif

#ifndef DEFAULT_ELITISM
#define DEFAULT_ELITISM 2 / 10
#endif

#ifndef DEFAULT_MUTATION_PROBABILITY
#define DEFAULT_MUTATION_PROBABILITY 0.1
#endif


/***********************************************************************
 * Genetic algorithm internal support functions.
 **********************************************************************/

/**
 * Sorts individuals and fitnesses.
 *
 * Sorting is done using fitness as ket, in ascending order.
 *
 * @param[in,out] status Status of the genetic algorithm
 * @note Internally uses a selection sort algorithm
 */
static void sort_by_fitness(GeneticAlgorithmStatus status) {
    unsigned int i, j, min_individual;
    double fitness_swap;
    GeneticAlgorithmIndividual individual_swap;

    for (i = 0; i < status.population_size; ++i) {
        min_individual = i;

        for (j = i + 1; j < status.population_size; ++j) {
            if (status.fitness[j] < status.fitness[min_individual]) {
                min_individual = j;
            }
        }

        individual_swap = status.population[i];
        status.population[i] = status.population[min_individual];
        status.population[min_individual] = individual_swap;

        fitness_swap = status.fitness[i];
        status.fitness[i] = status.fitness[min_individual];
        status.fitness[min_individual] = fitness_swap;
    }
}



/**
 * Normalizes fitness values.
 *
 * @param[in,out] status
 */
static void normalize_fitness(GeneticAlgorithmStatus *status) {
    unsigned int i = 0;
    double sum = 0.0;

    for (i = 0; i < status->population_size; ++i) {
        sum += status->fitness[i];
    }
    status->fitness_mean = sum / status->population_size;
    status->fitness_variance = 0.0;

    for (i = 0; i < status->population_size; ++i) {
        status->normalized_fitness[i] = status->fitness[i] / sum;
        status->fitness_variance = (status->fitness[i] - status->fitness_mean) * (status->fitness[i] - status->fitness_mean);
    }
    status->fitness_variance /= status->population_size;
}



/**
 * Allocates space for a population.
 *
 * @param[out] population Pointer to population to create
 * @param[in] size Maximum number of individuals of the population
 * @param[in] individual_size Size of an individual, in bytes
 * @note population_free must be called to ensure memory deallocation
 */
static void population_create(
    GeneticAlgorithmPopulation *population,
    const unsigned int size,
    const unsigned int individual_size
) {
    unsigned int i;

    *population = (GeneticAlgorithmPopulation) malloc(size * sizeof(GeneticAlgorithmIndividual));

    for (i = 0; i < size; ++i) {
        (*population)[i] = (GeneticAlgorithmIndividual) malloc(individual_size);
    }
}



/**
 * Deallocates a population.
 *
 * @param[out] population Pointer to population to delete
 * @param[in] size Maximum number of individual in the population
 */
static void population_delete(
    GeneticAlgorithmPopulation *population,
    const unsigned int size
) {
    unsigned int i;

    for (i = 0; i < size; ++i) {
        free((*population)[i]);
    }
    free(*population);
}



static void compute_fitness(GeneticAlgorithmStatus *status, const GeneticAlgorithmFitness fitness) {
    unsigned int i;

    for (i = 0; i < status->population_size; ++i) {
        status->fitness[i] = fitness(status->population[i], *status);
    }

    /* Sorts population by fitness */
    sort_by_fitness(*status);

    /* Computes normalized fitness */
    normalize_fitness(status);
}






/***********************************************************************
 * Public functions.
 **********************************************************************/

unsigned int genetic_algorithm_next_size_constant(const GeneticAlgorithmStatus status) {
    (void) status;

    return DEFAULT_POPULATION_SIZE;
}



unsigned int genetic_algorithm_elitism_constant(const GeneticAlgorithmStatus status) {
    (void) status;

    return status.population_size * DEFAULT_ELITISM;
}



GeneticAlgorithmIndividual genetic_algorithm_select_uniform(const GeneticAlgorithmStatus status) {
    return status.population[rand() % status.population_size];
}


GeneticAlgorithmIndividual genetic_algorithm_select_roulette_wheel(const GeneticAlgorithmStatus status) {
    unsigned int i;
    const double p = (double) rand() / RAND_MAX;
    double cumulative = 0.0;

    for (i = 0; i < status.population_size; ++i) {
        cumulative += status.normalized_fitness[i];
        if (p < cumulative) {
            break;
        }
    }

    return status.population[i];
}



void genetic_algorithm_crossover_average(
    GeneticAlgorithmIndividual offspring,
    const GeneticAlgorithmIndividual parent_a,
    const GeneticAlgorithmIndividual parent_b,
    const GeneticAlgorithmStatus status
) {
    unsigned int i;

    for (i = 0; i < status.genes; ++i) {
        ((double *) offspring)[i] = 0.5 * (((double *) parent_a)[i] + ((double *) parent_b)[i]);
    }
}



void genetic_algorithm_crossover_one_point(
    GeneticAlgorithmIndividual offspring,
    const GeneticAlgorithmIndividual parent_a,
    const GeneticAlgorithmIndividual parent_b,
    const GeneticAlgorithmStatus status
) {
    const unsigned int index = rand() % status.genes,
                       gene_size = status.individual_size / status.genes ;
    void *first = parent_a,
         *second = parent_b;

    if (rand() % 2) {
        first = parent_b;
        second = parent_a;
    }

    memcpy(offspring, first, index * gene_size);
    memcpy(
        (char *) offspring + index * gene_size,
        (char *) second + index * gene_size,
        (status.genes - index) * gene_size
    );
}



void genetic_algorithm_crossover_two_points(
    GeneticAlgorithmIndividual offspring,
    const GeneticAlgorithmIndividual parent_a,
    const GeneticAlgorithmIndividual parent_b,
    const GeneticAlgorithmStatus status
) {
    unsigned int index_1 = rand() % status.genes,
                 index_2 = rand() % status.genes;
    const unsigned int gene_size = status.individual_size / status.genes;
    void *first = parent_a,
         *second = parent_b;

    if (index_1 > index_2) {
        const unsigned int index_swap = index_1;
        index_1 = index_2;
        index_2 = index_swap;
    }

    if (rand() % 2) {
        first = parent_b;
        second = parent_a;
    }

    memcpy(offspring, first, index_1 * gene_size);
    memcpy(
        (char *) offspring + index_1 * gene_size,
        (char *) second + index_1 * gene_size,
        (index_2 - index_1) * gene_size
    );
    memcpy(
        (char *) offspring + index_2 * gene_size,
        (char *) first + index_2 * gene_size,
        (status.genes - index_2) * gene_size
    );
}



double genetic_algorithm_mutation_probability_constant(const GeneticAlgorithmStatus status) {
    (void) status;

    return DEFAULT_MUTATION_PROBABILITY;
}



void genetic_algorithm_mutation_swap(
    GeneticAlgorithmIndividual offspring,
    const GeneticAlgorithmStatus status
) {
    const unsigned int i = rand() % status.genes,
                       j = rand() % status.genes,
                       gene_size = status.individual_size / status.genes;

    memcpy(status.buffer, (char *) offspring + i * gene_size, gene_size);
    memcpy((char *) offspring + i * gene_size, (char *) offspring + j * gene_size, gene_size);
    memcpy((char *) offspring + j * gene_size, status.buffer, gene_size);
}



void genetic_algorithm_callback_simple(const GeneticAlgorithmStatus status) {
    printf(
        "Generation: %u\t Pop.size: %u\t Elitism: %u\t Min. fitness: %f\t Mean fitness: %f\t Max. fitness: %f\t Variance: %f\n",
        status.generation,
        status.population_size,
        status.elitism,
        status.fitness[0],
        status.fitness_mean,
        status.fitness[status.population_size - 1],
        status.fitness_variance
    );
}



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
) {
    GeneticAlgorithmStatus status;
    GeneticAlgorithmPopulation population_buffer, population_swap;
    unsigned int solution_found = 0;

    /* Initializes status */
    status.population_size = 0;
    status.max_population_size = max_population_size;
    status.generation = 0;
    status.max_generation = max_generation;
    status.elitism = elitism(status);
    status.genes = genes;
    status.individual_size = individual_size;
    population_create(&status.population, status.max_population_size, status.individual_size);
    status.fitness = (double *) malloc(max_population_size * sizeof(double));
    status.fitness_mean = 0.0;
    status.fitness_variance = 0.0;
    status.normalized_fitness = (double *) malloc(max_population_size * sizeof(double));
    status.data = (void *) data;
    status.buffer = malloc(individual_size);


    /* Initializes support structures */
    population_create(&population_buffer, status.max_population_size, status.individual_size);


    /* Inizializes population */
    status.population_size = next_generation_size(status);
    if (status.population_size > max_population_size) {
        status.population_size = max_population_size;
    }
    population_generator(status.population, status.population_size);
    compute_fitness(&status, fitness);


    /* Iterates over generations */
    while (status.generation < max_generation) {
        unsigned int i, next_size;

        /* Stops if an individual is a solution */
        for (i = 0; i < status.population_size; ++i) {
            if (is_solution(status.population[i], status)) {
                memcpy(solution, status.population[i], status.individual_size);
                solution_found = 1;
                break;
            }
        }
        if (solution_found) {
            break;
        }


        /* Prepares next generation */
        ++status.generation;
        next_size = next_generation_size(status);
        if (next_size > max_population_size) {
            next_size = max_population_size;
        }

        /* Preserves best individuals */
        status.elitism = elitism(status);
        for (i = 0; i < status.elitism && i < next_size && i < status.population_size; ++i) {
            memcpy(
                population_buffer[i],
                status.population[status.population_size - i - 1],
                status.individual_size
            );
        }

        /* Generates new offsprings until next generation is ready */
        for (i = status.elitism; i < next_size; ++i) {
            /* Breeds parents */
            const GeneticAlgorithmIndividual parent_a = select_parent(status),
                                             parent_b = select_parent(status);
            crossover(population_buffer[i], parent_a, parent_b, status);

            /* Has a chance to mutate offspring */
            if ((double) rand() / RAND_MAX < mutation_probability(status)) {
                mutation(population_buffer[i], status);
            }
        }

        /* Swaps old population with new population */
        population_swap = status.population;
        status.population = population_buffer;
        population_buffer = population_swap;
        status.population_size = next_size;

        /* Computes fitness, normalized fitness, mean, etc. */
        compute_fitness(&status, fitness);

        /* Calls callback function, if any */
        if (callback != NULL) {
            callback(status);
        }
    }

    /* Frees memory */
    population_delete(&status.population, status.max_population_size);
    population_delete(&population_buffer, status.max_population_size);
    free(status.fitness);
    free(status.normalized_fitness);
    free(status.buffer);

    return solution_found;
}
