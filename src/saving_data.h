//
// Created by hubert on 12/21/25.
//

#include "mes/mes.h"
#include "mesh/geometry.h"

#ifndef EASYFEM_SAVING_DATA_H
#define EASYFEM_SAVING_DATA_H

void save_fem_data(geo::Mesh &mesh, Fem::GlobalData conf);
void clean_vtu_files();
void load_inp_mesh(geo::Mesh &mesh);

#endif //EASYFEM_SAVING_DATA_H
