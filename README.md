# SAVer
SAVer (**S**VM **A**bstract **Ver**ifier) is an abstract interpretation based tool for proving properties of SVMs, in particular we aim at proving robustness or vulnerability properties of classifiers.

Given a point *x* and perturbation function *P*, SAVer symbolically computes an overapproximation of *P(x)*, the region of (possibly infinite) points which corresponds to perturbations of *x*, and runs an abstract version of the SVM on it, returning a superset of the labels associated to points in *P(x)*. Whenever such set contains a single label, the concrete SVM classifier is definitively robust on point *x* for perturbation *P*.
![Example of provable robustness](https://raw.githubusercontent.com/svm-abstract-verifier/saver/master/doc/images/example-right.svg?sanitize=true)

When SAVer returns more than one label, it may happen either due to the classifier really being not robust, or because of a loss of precision induced by the abstraction process.
![Example of not provable robustness](https://raw.githubusercontent.com/svm-abstract-verifier/saver/master/doc/images/example-top.svg?sanitize=true)

## Requirements

 - Any C99-compatible C compiler

## Installation
To install SAVer you need to clone or download the source files from this repository and compile them. There are no additional requirement nor dependencies:

    git clone https://github.com/svm-abstract-verifier/saver.git
or:

    wget https://github.com/svm-abstract-verifier/saver/archive/master.zip
    unzip master.zip
then:

    cd saver/src
    make
    make install
The executable file will be available under `saver/bin/saver`.

Every piece of code is documented using [Doxygen](http://www.doxygen.nl/). If you have Doxygen installed and wish to generate the documentation pages (HTML), run:

    cd saver/src
    make doc
Documentation will be available under `saver/doc/html/index.html`.

## Usage
Run `saver` without arguments for a quick online help message. Full syntax is:

    bin/saver <path to classifier> <path to dataset> [abstraction] [perturbation] [perturbation parameters]
Where

 - *path to classifier*: path to classifier file; must be in the appropriate format (see below)
 - *path to dataset*: path to dataset; must be in the appropriate format (see below)
 - *abstraction*: optional, tells which abstract domain to use for analysis; must be one of *interval*, *raf*, *hybrid* (default: *hybrid*)
 - *perturbation*: optional, type of perturbation; must be one of *l_inf*, *frame* (default: *frame*)
 - *perturbation parameters*: list of parameter specific for the type of perturbation; all perturbation have at least the magnitude parameter

You can find data sets and trained SVM classifiers in our [data-collection repository](https://github.com/svm-abstract-verifier/data-collection/).

## Example
    bin/saver ../data/mnist/svm-rbf-60k.dat ../data/mnist/test-set-normalized.csv raf l_inf 0.15
will evaluate whether the classifier in `../data/mnist/svm-rbf-60k.dat` is robust on each point taken from the data set in `../data/mnist/test-set-normalized.csv`, with respect to an L-infinity perturbation of magnitude 15% using the RAF abstract domain for the analysis.

## Classifier format
SAVer supports OVO SVM classifier files in the following format:

    ovo
    <feature space size>
    <number of classes>
    <kernel type and parameters>
    <class 1> <number of support vectors for class 1>
    <class 2> <number of support vectors for class 2>
    ...
    <class N> <number of support vectors for class N>
    <alpha coefficients>
    <support vectors>
    <biases>
Where alpha, support vectors and biases are stored as described by [Support Vector Machines - scikit-learn](https://scikit-learn.org/stable/modules/svm.html#multi-class-classification) documentation. You may also want to check [our data collection repository](https://github.com/svm-abstract-verifier/data-collection) for some examples, in particular the easy-to-read [Iris linear SVM classifier](https://github.com/svm-abstract-verifier/data-collection/blob/master/iris/svm-linear.dat.zip).

In particular, every SVM trained with `scikit-learn` and `libsvm` is compatible with this format.

## Data set format
Data sets are comma-separated-values (CSV) files in the following format:

    # <number of samples> <feature space size>
    label,value-1,...,value-M
    label,value-1,...,value-M
    ...
    label,value-1,...,value-M
First row begins with a *#* and shows number of samples and size of the feature space, that is, number of components for each sample. Actual data is stored in the following rows, one per sample, consisting in the label followed by the values of the components of the vector representing the sample. See our [Iris data set](https://github.com/svm-abstract-verifier/data-collection/blob/master/iris/data-set.csv.zip) for an easy-to-read example.
