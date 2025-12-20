#include <algorithm>
#include <cmath>
#include <stdexcept>

#include "gauss.h"
/*
std::vector<double> Gauss(Fem::Matrix& A, std::vector<double>& b) {
    int n = A.getRows();
    Fem::Matrix augmentedMatrix = A;
    for (int i = 0; i < n; ++i) {
        augmentedMatrix[i].push_back(b[i]);
    }
    for (int i = 0; i < n; ++i) {
        int maxRow = i;
        for (int k = i + 1; k < n; ++k) {
            if (std::abs(augmentedMatrix[k][i]) > std::abs(augmentedMatrix[maxRow][i])) {
                maxRow = k;
            }
        }
        std::swap(augmentedMatrix[i], augmentedMatrix[maxRow]);
        double pivot = augmentedMatrix[i][i];
        for (int j = i; j <= n; ++j) {
            augmentedMatrix[i][j] /= pivot;
        }
        for (int k = i + 1; k < n; ++k) {
            double factor = augmentedMatrix[k][i];
            for (int j = i; j <= n; ++j) {
                augmentedMatrix[k][j] -= factor * augmentedMatrix[i][j];
            }
        }
    }
    for (int i = n - 1; i >= 0; --i) {
        b[i] = augmentedMatrix[i][n];
        for (int j = i + 1; j < n; ++j) {
            b[i] -= augmentedMatrix[i][j] * b[j];
        }
    }

    return b;
}
*/

std::vector<double> Gauss(Fem::Matrix& A, std::vector<double>& b) {
    int n = A.getRows();
    if ((int)b.size() != n) {
        throw std::invalid_argument("Gauss: size of b must equal number of rows of A");
    }

    //tworzenie macierzy rozszerzone A+b
    Fem::Matrix aug(n, n + 1);
    for (int i = 0; i < n; ++i) {
         auto& src = A[i];
        auto& row = aug[i];
        for (int j = 0; j < n; ++j) row[j] = src[j];
        row[n] = b[i]; 
    }

    constexpr double EPS = 1e-14;

    for (int i = 0; i < n; ++i) {
        int piv = i;
        double best = std::fabs(aug[i][i]);
        for (int k = i + 1; k < n; ++k) {
            double v = std::fabs(aug[k][i]);
            if (v > best) { best = v; piv = k; }
        }
        if (best < EPS) {
            throw std::runtime_error("Gauss: matrix is singular or nearly singular (zero pivot)");
        }
        if (piv != i) std::swap(aug[i], aug[piv]);

        // --- Normalize pivot row once (reduce future divisions) ---
        auto& ri = aug[i];
        double inv_pivot = 1.0 / ri[i];
        for (int j = i; j <= n; ++j) ri[j] *= inv_pivot; // ri[i] becomes 1

        // --- Eliminate below pivot (row-wise, cache-friendly) ---
        for (int k = i + 1; k < n; ++k) {
            auto& rk = aug[k];
            double f = rk[i];
            if (f == 0.0) { rk[i] = 0.0; continue; }
            rk[i] = 0.0;

            for (int j = i + 1; j <= n; ++j) {
                rk[j] -= f * ri[j];
            }
        }
    }

    //podstawienie
    for (int i = n - 1; i >= 0; --i) {
        double xi = aug[i][n];
        for (int j = i + 1; j < n; ++j) {
            xi -= aug[i][j] * b[j];
        }
        b[i] = xi;
    }
    return b;
}