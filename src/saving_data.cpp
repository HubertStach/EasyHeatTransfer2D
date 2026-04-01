//
// Created by hubert on 12/21/25.
//

#include "saving_data.h"
#include <fstream>
#include <vector>

#include <filesystem>
#include <string>
#include <iostream>

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
    plik << "Triangle_number " << mesh.triangles.size() << "\n";
    plik << "Quad_number " << mesh.quads.size() << "\n";

    // 2. zapis siatki
    // 2.1 zapis punktów
    plik << "*Nodes\n";
    for (size_t i = 0; i < mesh.nodes.size(); i++) {
        plik << i << ", " << mesh.nodes[i].x*0.01f << ", " << mesh.nodes[i].y*0.01f << "\n";
    }

    // 2.2 zapis elementów trójkątnych
    plik << "*Triangles\n";
    for (size_t i = 0; i < mesh.triangles.size(); i++) {
        plik << i << ", "
             << mesh.triangles[i].node_ids[0] << ", "
             << mesh.triangles[i].node_ids[1] << ", "
             << mesh.triangles[i].node_ids[2] << "\n";
    }

    // 2.3 zapis elementów czworokątnych
    plik << "*Quads\n";
    for (size_t i = 0; i < mesh.quads.size(); i++) {
        plik << i << ", "
             << mesh.quads[i].node_ids[0] << ", "
             << mesh.quads[i].node_ids[1] << ", "
             << mesh.quads[i].node_ids[2] << ", "
             << mesh.quads[i].node_ids[3] << "\n";
    }

    // 2.4 zapis warunkow brzegowych
    plik << "*BC\n";
    for (size_t i = 0; i < mesh.nodes.size(); i++) {
        if (mesh.nodes[i].bc.is_bc) {
            plik << i  << ", " << mesh.nodes[i].bc.flux << ", " << mesh.nodes[i].bc.alfa << ", " << mesh.nodes[i].bc.t_ext << "\n";
        }
    }

    plik.close();
}

void clean_vtu_files() {
    std::string path = "Data";

    if (!std::filesystem::exists(path) || !std::filesystem::is_directory(path)) {
        return;
    }

    try {
        for (const auto& entry : std::filesystem::directory_iterator(path)) {
            if (entry.is_regular_file()) {
                std::string filename = entry.path().filename().string();

                if (filename.find("sol_") == 0 && entry.path().extension() == ".vtu") {
                    std::filesystem::remove(entry.path());
                }
            }
        }
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Blad podczas usuwania plikow: " << e.what() << std::endl;
    }
}
