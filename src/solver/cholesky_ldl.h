//
// Created by marci on 30.03.2026.
//

#ifndef EASYFEM_CHOLESKY_LDL_H
#define EASYFEM_CHOLESKY_LDL_H

#include <../../include/Eigen/Dense>
#include "../mes/matrix/matrix.h"


Eigen::MatrixXd convert_matrix(Fem::Matrix& mat);
std::vector<double> cholesky_ldl(Fem::Matrix& A, std::vector<double>& b);

#endif //EASYFEM_CHOLESKY_LDL_H