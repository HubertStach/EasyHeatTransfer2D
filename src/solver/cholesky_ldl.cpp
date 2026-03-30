//
// Created by marci on 30.03.2026.
//

#include "cholesky_ldl.h"
#include "../../include/Eigen/Sparse"
#include "../../include/Eigen/SparseCholesky"

Eigen::MatrixXd convert_matrix(Fem::Matrix& mat) {
    int rows = mat.getRows();
    int cols = mat.getCols();

    Eigen::MatrixXd eigenMat(rows, cols);

    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            eigenMat(i, j) = mat.data[i][j];
        }
    }

    return eigenMat;
}

std::vector<double> cholesky_ldl(Fem::Matrix& A, std::vector<double>& b) {
    if (!A.isSquare()) {
        throw std::invalid_argument("Macierz A musi byc kwadratowa dla rozkladu Choleskiego!");
    }
    if (A.getRows() != b.size()) {
        throw std::invalid_argument("Wymiary macierzy A i wektora b nie zgadzaja sie!");
    }

    Eigen::MatrixXd denseA = convert_matrix(A);

    //konwersja na macierz rzadką
    Eigen::SparseMatrix<double> sparseA = denseA.sparseView();
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

    std::vector<double> x(eigenX.data(), eigenX.data() + eigenX.size());

    return x;
}
