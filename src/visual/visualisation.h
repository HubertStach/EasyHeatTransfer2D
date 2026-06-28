#pragma once

#include <float.h>
#include <../../include/Eigen/Dense>
#include "../mesh/geometry.h"

struct Visualisation {
    bool solved = false;
    float min_temp=FLT_MAX;
    float max_temp=0.0;

    int current_step = -1;

    const std::string data_path = "Data";

    Eigen::MatrixXd temps_matrix;

    std::vector<int> time_ids;

    Visualisation();
    void init_visualisation(geo::Mesh &mesh);

};

static Color get_color_from_temp(float val, float min_val, float max_val);
