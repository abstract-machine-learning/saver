# SAVer
SVM Abstract Verifier tool

## Requirements

 - Any C compiler compatible with C99

## Installation
To install SAVer you need to clone or download the source files from this repository and compile them. There are no additional requirement nor dependencies:

    git clone https://github.com/svm-abstract-verifier/saver.git
or:

    wget https://github.com/svm-abstract-verifier/saver/archive/master.zip
then:

    cd saver/src
    make
    make install
The executable file will be available under `saver/bin/saver`.

## Usage
Run `saver` without arguments for a quick online help message. Full syntax is:

    bin/saver <path to classifier> <path to dataset> [abstraction] [perturbation] [perturbation parameters]
Where

 - *path to classifier*: path to classifier file; must be in the appropriate format (see below)
 - *path to dataset*: path to dataset; must be in the appropriate format (see below)
 - *abstraction*: optional, tells which abstract domain to use for analysis; must be one of *interval*, *raf*, *hybrid* (default: *hybrid*)
 - *perturbation*: optional, type of perturbation; must be one of *l_inf*, *frame* (default: *frame*)
 - *perturbation parameters*: list of parameter specific for the type of perturbation; all perturbation have at least the magnitude parameter

You can find data sets and trained SVM classifiers in our [data-collection repository](https://github.com/svm-abstract-verifier/data-collection/blob/master/iris/data-set.csv.zip).

## Example
    bin/saver ../data/mnist/svm-rbf-60k.dat ../data/mnist/test-set-normalized.csv raf l_inf 0.15
will evaluate whether the classifier in `../data/mnist/svn-rbf-60k.dat` is robust on the points taken from the data set in `../data/mnist/test-set-normalized.csv`, with respect to an L-infinity perturbation of magnitude 15% using the RAF abstract domain for the analysis.

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

## Data set format
Data sets are comma-separated-values (CSV) files in the following format:

    # <number of samples> <feature space size>
    label,value-1,...,value-M
    label,value-1,...,value-M
    ...
    label,value-1,...,value-M
First row begins with a *#* and shows number of samples and size of the feature space, that is, number of components for each sample. Actual data is stored in the following rows, one per sample, consisting in the label followed by the values of the components of the vector representing the sample. See our [Iris data set](https://github.com/svm-abstract-verifier/data-collection/blob/master/iris/data-set.csv.zip) for an easy-to-read example.
