#include "cholesky_ldl.h"
#include "../../include/Eigen/Sparse"
#include "../../include/Eigen/SparseCholesky"

std::vector<double> cholesky_ldl(Eigen::MatrixXd& A, std::vector<double>& b) {
    if (A.rows() != A.cols()) {
        throw std::invalid_argument("Macierz A musi byc kwadratowa dla rozkladu Choleskiego!");
    }
    if (A.rows() != (int)b.size()) {
        throw std::invalid_argument("Wymiary macierzy A i wektora b nie zgadzaja sie!");
    }

    Eigen::SparseMatrix<double> sparseA = A.sparseView();
    sparseA.makeCompressed();

    Eigen::Map<Eigen::VectorXd> eigenB(b.data(), b.size());

    Eigen::SimplicialLDLT<Eigen::SparseMatrix<double>> solver;
    solver.compute(sparseA);

    if(solver.info() != Eigen::Success) {
        throw std::runtime_error("Rozklad Choleskiego nie powiodl sie! Macierz moze nie byc dodatnio okreslona lub jest osobliwa.");
    }

    Eigen::VectorXd eigenX = solver.solve(eigenB);

    if(solver.info() != Eigen::Success) {
        throw std::runtime_error("Rozwiazywanie ukladu nie powiodlo sie!");
    }

    return std::vector<double>(eigenX.data(), eigenX.data() + eigenX.size());
}
