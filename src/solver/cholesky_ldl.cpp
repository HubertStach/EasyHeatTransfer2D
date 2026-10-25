#include "cholesky_ldl.h"

#include <stdexcept>

void CholeskyLDL::factorize(const SparseMatrix& A) {
    if (A.rows() != A.cols()) {
        throw std::invalid_argument("Macierz A musi byc kwadratowa dla rozkladu Choleskiego!");
    }

    ldlt.compute(A);

    if (ldlt.info() != Eigen::Success) {
        throw std::runtime_error("Rozklad Choleskiego nie powiodl sie! Macierz moze nie byc dodatnio okreslona lub jest osobliwa.");
    }
}

Eigen::VectorXd CholeskyLDL::solve(const Eigen::VectorXd& b) const {
    Eigen::VectorXd x = ldlt.solve(b);

    if (ldlt.info() != Eigen::Success) {
        throw std::runtime_error("Rozwiazywanie ukladu nie powiodlo sie!");
    }
    return x;
}