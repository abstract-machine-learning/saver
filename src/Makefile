#-----------------------------------------------------------------------
# Configuration

# Floating point precision
# [FLOAT | DOUBLE | LONG_DOUBLE]
PRECISION = DOUBLE

# Enforce soundness
# [ false | true ]
ifeq ($(SOUND), true)
    ENFORCE_SOUNDNESS = -DENFORCE_SOUNDNESS
else
    ENFORCE_SOUNDNESS = 
endif

CC = gcc
CCOPT = -Wall -Wextra -pedantic -O2 -std=c99 -g -DPRECISION_$(PRECISION) $(ENFORCE_SOUNDNESS)
LDOPT = -lm
NAME = saver
INSTALL_FOLDER = ../bin
DOXYFILE_PATH = ../doc/Doxyfile


#-----------------------------------------------------------------------
# Dependencies
all: $(NAME)

$(NAME): kernel.o \
        classifier.o \
        perturbation.o \
        tier.o \
        abstract_domains/one_hot_interval.o \
        abstract_domains/one_hot_raf.o \
        adversarial_region.o \
        abstract_domains/raf.o \
        abstract_classifiers/abstract_classifier.o \
        abstract_classifiers/interval_classifier.o \
        abstract_classifiers/raf_classifier.o \
        abstract_classifiers/hybrid_classifier.o \
        counterexamples/counterexample.o \
        counterexamples/counterexample_seeker.o \
        counterexamples/robustness_vertex_heuristic.o counterexamples/robustness_genetic_algorithm.o counterexamples/robustness_divide_et_impera.o \
        stopwatch.o \
        options.o \
        dataset.o \
        saver.o

install: $(NAME)

.PHONY: clean, doc


#-----------------------------------------------------------------------
# Recipes
%.o: %.c
	@echo "Compiling $@..."
	@$(CC) $(CCOPT) -c -o $@ $^

saver:
	@echo "Linking $^ into $@..."
	@$(CC) $(CCOPT) -o $@ $^ $(LDOPT)

install:
	@echo "Moving into installation folder $(INSTALL_FOLDER)/$(NAME)..."
	@mkdir -p ../bin
	@mv $(NAME) $(INSTALL_FOLDER)/$(NAME)

clean:
	@echo "Cleaning..."
	@rm -fR *.o */*.o

doc:
	@echo "Generating documentation..."
	@doxygen $(DOXYFILE_PATH)
