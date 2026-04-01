//
// Created by hubert on 12/21/25.
//

#include "saving_data.h"

#include <algorithm>
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

void load_inp_mesh(geo::Mesh &mesh) {
    std::fstream file_inp("Data/mes_siatka.inp");

    if(!file_inp.is_open()) {
        std::cout << "Cannot find Job-1.inp file in Data folder!\n";
        return;
    }

    std::vector<geo::Node> geo_nodes;
    std::vector<geo::Triangle> geo_triangle;
    std::vector<geo::Quad> geo_quads;

    std::string line;
    bool node_selection = false;
    bool element_tri_selection = false;
    bool element_quad_selection = false;
    bool bc_selection = false;

    while (std::getline(file_inp, line)) {
        // Zabezpieczenie przed znakami karetki (CRLF z Windowsa)
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }

        if (line.empty()) continue;

        // 1. IGNORUJEMY KOMENTARZE ABAQUSA (Zaczynają się od **)
        // Dzięki temu nie zresetujemy przypadkowo flag parsowania!
        if (line.size() >= 2 && line[0] == '*' && line[1] == '*') {
            continue;
        }

        if (line[0] == '*') {
            node_selection = false;
            element_tri_selection = false;
            element_quad_selection = false;
            bc_selection = false;

            if (line.find("*Node") != std::string::npos) {
                node_selection = true;
            }
            else if (line.find("CPS3") != std::string::npos ||
                     line.find("CPE3") != std::string::npos ||
                     line.find("CAX3") != std::string::npos ||
                     line.find("S3")   != std::string::npos)
            {
                element_tri_selection = true;
            }
            else if (line.find("CPS4") != std::string::npos ||
                     line.find("CPE4") != std::string::npos ||
                     line.find("CAX4") != std::string::npos ||
                     line.find("S4")   != std::string::npos)
            {
                element_quad_selection = true;
            }
            else if (line.find("*Nset") != std::string::npos && line.find("BC") != std::string::npos) {
                bc_selection = true;
            }
            continue;
        }

        // --- Przetwarzanie danych w zależności od aktywnej sekcji ---
        if (node_selection) {
            std::replace(line.begin(), line.end(), ',', ' ');
            std::istringstream iss(line);
            iss.imbue(std::locale::classic());

            int id;
            float x, y;
            if (iss >> id >> x >> y) {
                geo_nodes.emplace_back(x, y, false);
            }
        }
        else if (element_tri_selection) {
            std::replace(line.begin(), line.end(), ',', ' ');
            std::istringstream iss(line);
            iss.imbue(std::locale::classic());

            int id;
            int n1, n2, n3;
            if (iss >> id >> n1 >> n2 >> n3) {
                geo::Triangle tri_temp(n1 - 1, n2 - 1, n3 - 1);
                geo_triangle.push_back(tri_temp);
            }
        }
        else if (element_quad_selection) {
            std::replace(line.begin(), line.end(), ',', ' ');
            std::istringstream iss(line);
            iss.imbue(std::locale::classic());

            int id;
            int n1, n2, n3, n4;
            if (iss >> id >> n1 >> n2 >> n3 >> n4) {
                geo::Quad quad_temp(n1 - 1, n2 - 1, n3 - 1, n4 - 1);
                geo_quads.push_back(quad_temp);
            }
        }
        else if (bc_selection) {
            std::replace(line.begin(), line.end(), ',', ' ');
            std::istringstream iss(line);
            iss.imbue(std::locale::classic());

            int node_id;
            while (iss >> node_id) {
                if (node_id > 0 && node_id <= geo_nodes.size()) {
                    geo_nodes[node_id - 1].bc.is_bc = true;
                    geo_nodes[node_id - 1].bc.initialised = true;

                    geo_nodes[node_id - 1].bc.flux = 0.0f;
                    geo_nodes[node_id - 1].bc.alfa = 0.0f;
                    geo_nodes[node_id - 1].bc.t_ext = 0.0f;
                }
            }
        }
    }

    file_inp.close();
    mesh.nodes = geo_nodes;
    mesh.triangles = geo_triangle;
    mesh.quads = geo_quads;
}