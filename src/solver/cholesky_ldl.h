#ifndef EASYFEM_CHOLESKY_LDL_H
#define EASYFEM_CHOLESKY_LDL_H

#include <../../include/Eigen/Dense>
#include <vector>

std::vector<double> cholesky_ldl(Eigen::MatrixXd& A, std::vector<double>& b);

#endif //EASYFEM_CHOLESKY_LDL_H
