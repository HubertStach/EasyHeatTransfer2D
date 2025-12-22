//
// Created by hubert on 12/21/25.
//

#include "saving_data.h"
#include <fstream>
#include <vector>

void save_fem_data(geo::Mesh &mesh, Fem::GlobalData conf) {
    // 1. Use std::ofstream for outputting files
    // 2. No need for std::fstream::in; ofstream defaults to writing mode
    std::ofstream plik("Data/fem_data.txt");

    if (!plik.is_open()) { // Better way to check if file is ready
        std::cerr << "Could not create file!" << std::endl;
        return;
    }

    // 1. zapis konfiguracji
    plik << "SimulationTime " << conf.total_time << "\n";
    plik << "SimulationStepTime " << conf.time_step << "\n";
    plik << "Conductivity " << conf.conductivity << "\n";
    plik << "InitialTemp " << conf.init_temperature << "\n";
    plik << "Density " << conf.density << "\n";
    plik << "SpecificHeat " << conf.specific_heat << "\n";
    plik << "Nodes_number " << mesh.nodes.size() << "\n";
    plik << "Elements_number " << mesh.triangles.size() << "\n";

    // 2. zapis siatki
    // 2.1 zapis punktów
    plik << "*Nodes\n";
    for (size_t i = 0; i < mesh.nodes.size(); i++) {
        plik << i << ", " << mesh.nodes[i].x << ", " << mesh.nodes[i].y << "\n";
    }

    // 2.2 zapis elementów
    plik << "*Elements\n";
    for (size_t i = 0; i < mesh.triangles.size(); i++) {
        plik << i << ", "
             << mesh.triangles[i].node_ids[0] << ", "
             << mesh.triangles[i].node_ids[1] << ", "
             << mesh.triangles[i].node_ids[2] << "\n";
    }

    // 2.3 zapis warunkow brzegowych
    plik << "*BC\n";
    float neumann = 0.0f;
    float alpha = 300.0f;
    float t_ext = 1200.0f;
    for (size_t i = 0; i < mesh.nodes.size(); i++) {
        if (mesh.nodes[i].is_bc) {
            plik << i  << ", " << neumann << ", " << alpha << ", " << t_ext << "\n";
        }
    }

    plik.close();
    //std::cout << "Data saved successfully to fem_data.txt" << std::endl;
}
