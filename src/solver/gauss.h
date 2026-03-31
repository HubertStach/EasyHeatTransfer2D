#include <vector>
#include "../mes/matrix/matrix.h"

std::vector<double> Gauss(Fem::Matrix& A, std::vector<double>& b);
std::vector<double> Gauss_pivot(Fem::Matrix& A, std::vector<double>& b);