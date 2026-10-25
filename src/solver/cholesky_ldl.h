#ifndef EASYFEM_CHOLESKY_LDL_H
#define EASYFEM_CHOLESKY_LDL_H

#include <../../include/Eigen/Sparse>
#include <../../include/Eigen/SparseCholesky>

using SparseMatrix = Eigen::SparseMatrix<double>;

// Rozkład A = L D L^T macierzy rzadkiej, symetrycznej i dodatnio określonej.
// Faktoryzacja (factorize) jest kosztowna i wykonywana jednokrotnie,
// rozwiązanie (solve) to tylko podstawienie w przód i wstecz.
class CholeskyLDL {
public:
    void factorize(const SparseMatrix& A);
    Eigen::VectorXd solve(const Eigen::VectorXd& b) const;

private:
    Eigen::SimplicialLDLT<SparseMatrix> ldlt;
};

#endif //EASYFEM_CHOLESKY_LDL_H