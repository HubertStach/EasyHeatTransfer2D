//
// Created by hubert on 12/30/25.
//

#include "visualisation.h"

#include <algorithm>
#include <filesystem>
#include <fstream>

#include "rlgl.h"
#include "../mes/matrix/matrix.h"
#include "../mesh/geometry.h"


Visualisation::Visualisation() : temps_matrix(1,1) {}

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
                    std::string number_str = name_no_ext.substr(4); // "sol_" ma 4 znaki
                    int number = std::stoi(number_str);
                    sorted_files_info.push_back({number, entry.path().string()});
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

    this->temps_matrix = Fem::Matrix(mesh.nodes.size(), sorted_files_info.size());

    for (size_t col_idx = 0; col_idx < sorted_files_info.size(); ++col_idx) {

        const auto& file_info = sorted_files_info[col_idx];
        const std::string& filename = file_info.second;

        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Blad: Nie mozna otworzyc pliku " << filename << std::endl;

            for (size_t node_row = 0; node_row < mesh.nodes.size(); ++node_row) {
                temps_matrix[node_row][col_idx] = 0.0f;
            }
            continue;
        }

        std::string line;
        bool inside_temp_array = false;
        int current_node_idx = 0;

        while (std::getline(file, line)) {
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
                    if (current_node_idx < mesh.nodes.size()) {
                        temps_matrix[current_node_idx][col_idx] = temp_val;
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

Color Visualisation::get_color_from_temp(float val, float min_val, float max_val) {
    if (fabs(max_val - min_val) < 0.0001f) return GREEN;

    // Normalizacja do zakresu 0.0 - 1.0
    float t = (val - min_val) / (max_val - min_val);
    t = std::clamp(t, 0.0f, 1.0f);

    unsigned char r = 0, g = 0, b = 0;

    if (t < 0.5f) {
        // Przejście Niebieski -> Zielony
        float local_t = t * 2.0f; // skalujemy 0..0.5 na 0..1
        r = 0;
        g = (unsigned char)(255 * local_t);
        b = (unsigned char)(255 * (1.0f - local_t));
    } else {
        // Przejście Zielony -> Czerwony
        float local_t = (t - 0.5f) * 2.0f; // skalujemy 0.5..1 na 0..1
        r = (unsigned char)(255 * local_t);
        g = (unsigned char)(255 * (1.0f - local_t));
        b = 0;
    }

    return Color{r, g, b, 255};
}