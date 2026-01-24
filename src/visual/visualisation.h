//
// Created by hubert on 12/30/25.
//

#ifndef EASYFEM_VISUALISATION_H
#define EASYFEM_VISUALISATION_H

#include <float.h>

#include "../mes/matrix/matrix.h"
#include "../mesh/geometry.h"

struct Visualisation {
    bool solved = false;
    float min_temp=FLT_MAX;
    float max_temp=0.0;

    int current_step = -1;

    const std::string data_path = "Data";

    //macierz temperatur w każdym momencie czasowym
    Fem::Matrix temps_matrix;

    //wektor id kolejnych momentów czasowych
    std::vector<int> time_ids;

    Visualisation();
    void init_visualisation(geo::Mesh &mesh);

    static Color get_color_from_temp(float val, float min_val, float max_val);
    void draw_temp_grad(const geo::Mesh &mesh);
};

#endif //EASYFEM_VISUALISATION_H