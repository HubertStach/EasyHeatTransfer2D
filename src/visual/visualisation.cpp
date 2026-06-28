#include "visualisation.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <string>
#include <sstream>
#include <iostream>
#include <cmath>

#include "rlgl.h"
#include "../mesh/geometry.h"


Visualisation::Visualisation() : temps_matrix(Eigen::MatrixXd::Zero(1,1)) {}

void Visualisation::init_visualisation(geo::Mesh &mesh) {
    if (!std::filesystem::exists(this->data_path) || !std::filesystem::is_directory(this->data_path)) {
        std::cerr << "Nie ma dostępu do folderu Data\n";
        return;
    }

    std::vector<std::pair<int, std::string>> sorted_files_info;

    for (const auto& entry : std::filesystem::directory_iterator(this->data_path)) {
        if (entry.is_regular_file() && entry.path().extension() == ".vtu") {
            std::string name_no_ext = entry.path().stem().string();

            if (name_no_ext.find("sol_") == 0) {
                try {
                    std::string number_str = name_no_ext.substr(4);
                    int number = stoi(number_str);
                    sorted_files_info.emplace_back(number, entry.path().string());
                }
                catch (...) {
                    std::cerr << "Ostrzezenie: Niepoprawna nazwa pliku VTU: " << entry.path().string() << std::endl;
                }
            }
        }
    }

    std::sort(sorted_files_info.begin(), sorted_files_info.end(),
              [](const auto& a, const auto& b) {
                  return a.first < b.first;
              });

    this->time_ids.clear();

    for(const auto& file_info : sorted_files_info) {
        this->time_ids.push_back(file_info.first);
    }

    this->temps_matrix = Eigen::MatrixXd::Zero(mesh.nodes.size(), sorted_files_info.size());

    for (size_t col_idx = 0; col_idx < sorted_files_info.size(); ++col_idx) {

        const auto& file_info = sorted_files_info[col_idx];
        const std::string& filename = file_info.second;

        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Blad: Nie mozna otworzyc pliku " << filename << std::endl;
            continue;
        }

        std::string line;
        bool inside_temp_array = false;
        int current_node_idx = 0;

        while (getline(file, line)) {
            if (!inside_temp_array && line.find("Name=\"Temperature\"") != std::string::npos) {
                inside_temp_array = true;
                continue;
            }

            if (inside_temp_array) {
                if (line.find("</DataArray>") != std::string::npos) {
                    break;
                }

                std::stringstream ss(line);
                float temp_val;

                while (ss >> temp_val) {
                    if (current_node_idx < (int)mesh.nodes.size()) {
                        temps_matrix(current_node_idx, col_idx) = temp_val;
                        current_node_idx++;
                    }

                    if (temp_val >= this->max_temp) {
                        max_temp = temp_val;
                    }
                    if (temp_val <= this->min_temp) {
                        min_temp = temp_val;
                    }
                }
            }
        }
        file.close();
    }

    this->solved = true;
    this->current_step = 0;
}
