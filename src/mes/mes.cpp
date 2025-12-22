#include <vector>
#include <string>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <chrono>
#include <algorithm>

#include "mes.h"
#include "../gauss/gauss.h"

namespace Fem {
    std::vector<double> pc_xi = {0.1667, 0.6667, 0.1667};
    std::vector<double> pc_eta = {0.1667, 0.1667, 0.6667};
    std::vector<double> weights_pc = {1.0/6.0, 1.0/6.0, 1.0/6.0};

    std::vector<double> n1_bc_val = {0.8873, 0.5000, 0.1127, 0, 0, 0, 0.1127, 0.5000, 0.8873 };
    std::vector<double> n2_bc_val = {0.1127, 0.5000, 0.8873, 0.8873, 0.5000, 0.1127, 0, 0, 0};
    std::vector<double> n3_bc_val = {0, 0, 0, 0.1127, 0.5000, 0.8873, 0.8873, 0.5000, 0.1127,};

    std::vector<double> weights_bc = {5.0/9.0, 8.0/9.0, 5.0/9.0};

    std::vector<float> dNdxi = {-1, 1, 0};
    std::vector<float> dNdeta = {-1, 0, 1};

    void showProgress(int current, int max)
    {
        if (max <= 0) return;
        
        float percentage = (float)current / max * 100.0f;
        
        const int barWidth = 60;
        int filledWidth = (int)(percentage / 100.0f * barWidth);
        
        std::string bar = "[";
        for (int i = 0; i < barWidth; i++) {
            if (i < filledWidth) {
                bar += "="; 
            } else {
                bar += " ";  
            }
        }
        bar += "]";
        
        std::cout << "\r" << bar << " " 
                << std::fixed << std::setprecision(1) << percentage << "% "
                << "(" << current<<"/"<<max << ")" << std::flush;
        

        if (current >= max) {
            std::cout << std::endl;
        }
    }

    float Element::N1(float x, float y, std::vector<Node> &nodes) {
        float x1, x2, x3, y1, y2, y3;
        x1 = nodes[this->node_ids[0]].x;
        y1 = nodes[this->node_ids[0]].y;
        x2 = nodes[this->node_ids[1]].x;
        y2 = nodes[this->node_ids[1]].y;
        x3 = nodes[this->node_ids[2]].x;
        y3 = nodes[this->node_ids[2]].y;

        const float half_area = ((x2-x1)*(y3-y1))-((x3-x1)*(y2-y1));

        return (((x3-x2)*(y-y2)-(x-x2)*(y3-y2)) / half_area);
    }

    float Element::N2(float x, float y, std::vector<Node> &nodes) {
        float x1, x2, x3, y1, y2, y3;
        x1 = nodes[this->node_ids[0]].x;
        y1 = nodes[this->node_ids[0]].y;
        x2 = nodes[this->node_ids[1]].x;
        y2 = nodes[this->node_ids[1]].y;
        x3 = nodes[this->node_ids[2]].x;
        y3 = nodes[this->node_ids[2]].y;

        const float half_area = ((x2-x1)*(y3-y1))-((x3-x1)*(y2-y1));

        return (((x1-x3)*(y-y3)-(x-x3)*(y1-y3)) / half_area);
    }

    float Element::N3(float x, float y, std::vector<Node> &nodes) {
        float x1, x2, x3, y1, y2, y3;
        x1 = nodes[this->node_ids[0]].x;
        y1 = nodes[this->node_ids[0]].y;
        x2 = nodes[this->node_ids[1]].x;
        y2 = nodes[this->node_ids[1]].y;
        x3 = nodes[this->node_ids[2]].x;
        y3 = nodes[this->node_ids[2]].y;

        const float half_area = ((x2-x1)*(y3-y1))-((x3-x1)*(y2-y1));

        return (((x2-x1)*(y-y1)-(x-x1)*(y2-y1)) / half_area);
    }

    float Element::dN1dx(std::vector<Node> &nodes) {
        float x1, x2, x3, y1, y2, y3;

        x1 = nodes[this->node_ids[0]].x;
        y1 = nodes[this->node_ids[0]].y;
        x2 = nodes[this->node_ids[1]].x;
        y2 = nodes[this->node_ids[1]].y;
        x3 = nodes[this->node_ids[2]].x;
        y3 = nodes[this->node_ids[2]].y;

        const float half_area = ((x2-x1)*(y3-y1))-((x3-x1)*(y2-y1));

        return ((y2-y3)/half_area);
    }

    float Element::dN2dx(std::vector<Node> &nodes) {
        float x1, x2, x3, y1, y2, y3;

        x1 = nodes[this->node_ids[0]].x;
        y1 = nodes[this->node_ids[0]].y;
        x2 = nodes[this->node_ids[1]].x;
        y2 = nodes[this->node_ids[1]].y;
        x3 = nodes[this->node_ids[2]].x;
        y3 = nodes[this->node_ids[2]].y;

        const float half_area = ((x2-x1)*(y3-y1))-((x3-x1)*(y2-y1));

        return ((y3-y1)/half_area);
    }

    float Element::dN3dx(std::vector<Node> &nodes) {
        float x1, x2, x3, y1, y2, y3;

        x1 = nodes[this->node_ids[0]].x;
        y1 = nodes[this->node_ids[0]].y;
        x2 = nodes[this->node_ids[1]].x;
        y2 = nodes[this->node_ids[1]].y;
        x3 = nodes[this->node_ids[2]].x;
        y3 = nodes[this->node_ids[2]].y;


        return ((y1-y2)/(((x2-x1)*(y3-y1))-((x3-x1)*(y2-y1))));
    }

    float Element::dN1dy(std::vector<Node> &nodes) {
        float x1, x2, x3, y1, y2, y3;

        x1 = nodes[this->node_ids[0]].x;
        y1 = nodes[this->node_ids[0]].y;
        x2 = nodes[this->node_ids[1]].x;
        y2 = nodes[this->node_ids[1]].y;
        x3 = nodes[this->node_ids[2]].x;
        y3 = nodes[this->node_ids[2]].y;


        return ((x3-x2)/(((x2-x1)*(y3-y1))-((x3-x1)*(y2-y1))));
    }

    float Element::dN2dy(std::vector<Node> &nodes) {
        float x1, x2, x3, y1, y2, y3;

        x1 = nodes[this->node_ids[0]].x;
        y1 = nodes[this->node_ids[0]].y;
        x2 = nodes[this->node_ids[1]].x;
        y2 = nodes[this->node_ids[1]].y;
        x3 = nodes[this->node_ids[2]].x;
        y3 = nodes[this->node_ids[2]].y;


        return ((x1-x3)/(((x2-x1)*(y3-y1))-((x3-x1)*(y2-y1))));
    }

    float Element::dN3dy(std::vector<Node> &nodes) {
        float x1, x2, x3, y1, y2, y3;

        x1 = nodes[this->node_ids[0]].x;
        y1 = nodes[this->node_ids[0]].y;
        x2 = nodes[this->node_ids[1]].x;
        y2 = nodes[this->node_ids[1]].y;
        x3 = nodes[this->node_ids[2]].x;
        y3 = nodes[this->node_ids[2]].y;


        return ((x2-x1)/(((x2-x1)*(y3-y1))-((x3-x1)*(y2-y1))));
    }

    std::vector<Node> load_nodes(std::string file_name)
    {
        std::fstream file;
        std::vector<Node> result;

        file.open(file_name);
        if(!file.good()){
            std::cout << "Couldn't load text file\n";
            return result;
        }

        std::string line;
        bool node_selection = false;

        while (std::getline(file, line)) {
            if (line == "*Nodes") {
                node_selection = true;
                continue;
            }
            
            if (line == "*Elements" || line == "*BC") {
                node_selection = false;
            }

            if (node_selection) {
                int id;
                float x, y;
                char comma;
                std::istringstream iss(line);
                if (iss >> id >> comma >> x >> comma >> y) {
                    result.emplace_back(x, y);
                }
            }
        }

        file.close();
        return result;
    }

    std::vector<Element> load_elements(std::string file_name)
    {
        std::fstream file;
        std::vector<Element> result;

        file.open(file_name);
        if(!file.good()){
            std::cout << "Couldn't load text file\n";
            return result;
        }

        std::string line;
        bool element_selection = false;

        while (std::getline(file, line)) {
            if (line == "*Elements") {
                element_selection = true;
                continue;
            }
            
            if (line == "*Nodes" || line == "*BC") {
                element_selection = false;
            }

            if (element_selection) {
                int id;
                int node_id1, node_id2, node_id3;
                char comma;
                std::istringstream iss(line);
                if (iss >> id >> comma >> node_id1 >> comma >> node_id2 >> comma >> node_id3) {
                    result.emplace_back(node_id1, node_id2, node_id3);
                }
            }
        }

        file.close();
        return result;
    }

    GlobalData load_configuration(std::string file_name)
    {
        std::fstream file;
        GlobalData data;

        file.open(file_name);
        if(!file.good()){
            std::cout << "Couldn't load text file\n";
            return data;
        }

        std::string line;

        while (std::getline(file, line)) {
            std::istringstream iss(line);
            std::string key;
            float value;

            if (iss >> key >> value) {
                if (key == "SimulationTime") data.total_time = value;
                else if (key == "SimulationStepTime") data.time_step = value;
                else if (key == "Conductivity") data.conductivity = value;
                else if (key == "InitialTemp") data.init_temperature = value;
                else if (key == "Density") data.density = value;
                else if (key == "SpecificHeat") data.specific_heat = value;
                else if (key == "Nodes_number") data.node_number = value;
                else if (key == "Elements_number") data.elem_number = value;
            }
        }

        file.close();
        return data;
    }

    std::vector<Node> load_bc(std::string file_name, 
        std::vector<Node>& nodes)
    {
        std::fstream file;

        file.open(file_name);
        if(!file.good()){
            std::cout << "Couldn't load text file\n";
            return nodes;
        }

        std::string line;
        bool bc_selection = false;

        while (std::getline(file, line)) {
            if (line == "*BC") {
                bc_selection = true;
                continue;
            }

            if (line == "*Nodes" || line == "*Elements") {
                bc_selection = false;
            }

            if (bc_selection) {
                int node_id;
                float flux, alfa, t_ext;
                char comma;
                std::istringstream iss(line);
                if (iss >> node_id >> comma >> flux >> comma >> alfa >> comma >> t_ext) {
                    BC_node temp_bc(node_id, flux, alfa, t_ext);
                    nodes[node_id].bc = temp_bc;
                }
                else {
                    std::cout << "Failed to parse line: " << line << "\n"; // Debug output
                }
            }
        }

        file.close();
        return nodes;
    }

    //prints current configuration from file
    void print_config(GlobalData configuration)
    {
        std::cout<<"-----------------Configuration-----------------\n";
        std::cout<<"Total time = "<<configuration.total_time<<"\n";
        std::cout<<"Time step = "<<configuration.time_step<<"\n";
        std::cout<<"Conductivity = "<<configuration.conductivity<<"\n";
        std::cout<<"Initial temperature = "<<configuration.init_temperature<<"\n";
        std::cout<<"Density = "<<configuration.density<<"\n";
        std::cout<<"Specific heat = "<<configuration.specific_heat<<"\n";
        std::cout<<"Number of nodes = "<<configuration.node_number<<"\n";
        std::cout<<"Number of elements = "<<configuration.elem_number<<"\n";
        std::cout<<"-----------------Configuration-----------------\n";
    }


    float dist(Node a, Node b){
        return sqrt(pow( a.x - b.x,2)+pow(a.y - b.y,2));
    }

    Matrix calc_local_H(Element &local_el, std::vector<Node> &nodes, float conductivity)
    {
        Matrix dNdx(3,1), dNdy(3,1);

        dNdx[0][0] = local_el.dN1dx(nodes);
        dNdx[1][0] = local_el.dN2dx(nodes);
        dNdx[2][0] = local_el.dN3dx(nodes);
        dNdy[0][0] = local_el.dN1dy(nodes);
        dNdy[1][0] = local_el.dN2dy(nodes);
        dNdy[2][0] = local_el.dN3dy(nodes);

        float x1 = nodes[local_el.node_ids[0]].x;
        float y1 = nodes[local_el.node_ids[0]].y;
        float x2 = nodes[local_el.node_ids[1]].x;
        float y2 = nodes[local_el.node_ids[1]].y;
        float x3 = nodes[local_el.node_ids[2]].x;
        float y3 = nodes[local_el.node_ids[2]].y;

        float D = (x2-x1)*(y3-y1) - (x3-x1)*(y2-y1);
        float Area = std::abs(D) / 2.0f;

        Matrix H_local = (dNdx*dNdx.transpose()) + (dNdy*dNdy.transpose());

        for(int i=0; i<3; ++i) {
            for(int j=0; j<3; ++j) {
                H_local[i][j] *= conductivity*Area;
            }
        }

        return H_local;
    }

    Matrix calc_local_Hbc(Element &local_el, std::vector<Node> &nodes)
    {
        Matrix hbc(3, 3); // Zakładam, że konstruktor wypełnia macierz zerami

        for (int i = 0; i < 3; i++) { // pętla po bokach trójkąta
            int id1 = local_el.node_ids[i];
            int id2 = local_el.node_ids[(i + 1) % 3];

            if (!(nodes[id1].bc.exist && nodes[id2].bc.exist)) continue;

            float alfa = nodes[id1].bc.alfa;
            double dx = nodes[id2].x - nodes[id1].x;
            double dy = nodes[id2].y - nodes[id1].y;
            double L = std::sqrt(dx * dx + dy * dy);

            double factor = (alfa * L) / 6.0;

            switch (i) {
                case 0:
                    hbc[0][0] += 2 * factor;
                    hbc[0][1] += 1 * factor;
                    hbc[1][0] += 1 * factor;
                    hbc[1][1] += 2 * factor;
                    break;

                case 1:
                    hbc[1][1] += 2 * factor;
                    hbc[1][2] += 1 * factor;
                    hbc[2][1] += 1 * factor;
                    hbc[2][2] += 2 * factor;
                    break;

                case 2:
                    hbc[2][2] += 2 * factor;
                    hbc[2][0] += 1 * factor;
                    hbc[0][2] += 1 * factor;
                    hbc[0][0] += 2 * factor;
                    break;
            }
        }

        return hbc;
    }

    Matrix calc_p_vec(Element &local_el, std::vector<Node> &nodes)
    {
        Matrix p_vec(3, 1);

        for (int edge = 0; edge < 3; edge++) {
            int local_idx1 = edge;
            int local_idx2 = (edge + 1) % 3;

            int id1 = local_el.node_ids[local_idx1];
            int id2 = local_el.node_ids[local_idx2];

            if (!(nodes[id1].bc.exist && nodes[id2].bc.exist)) continue;

            float t_ext = nodes[id1].bc.t_ext;
            float alfa = nodes[id1].bc.alfa;
            float flux = nodes[id1].bc.flux;

            double dx = nodes[id2].x - nodes[id1].x;
            double dy = nodes[id2].y - nodes[id1].y;
            double L = std::sqrt(dx * dx + dy * dy);

            double total_flux = (alfa * t_ext) + flux;
            double contribution = (total_flux * L) / 2.0;

            switch (edge) {
                case 0: // Bok 0-1
                    p_vec[0][0] += contribution;
                    p_vec[1][0] += contribution;
                    break;

                case 1: // Bok 1-2
                    p_vec[1][0] += contribution;
                    p_vec[2][0] += contribution;
                    break;

                case 2: // Bok 2-0
                    p_vec[2][0] += contribution;
                    p_vec[0][0] += contribution;
                    break;
            }
        }

        return p_vec;
    }

    Matrix calc_c(Element &local_el, std::vector<Node> &nodes, float density, float specific_heat)
    {
        Matrix c_matrix(3, 3);

        float x1 = nodes[local_el.node_ids[0]].x;
        float y1 = nodes[local_el.node_ids[0]].y;
        float x2 = nodes[local_el.node_ids[1]].x;
        float y2 = nodes[local_el.node_ids[1]].y;
        float x3 = nodes[local_el.node_ids[2]].x;
        float y3 = nodes[local_el.node_ids[2]].y;

        float D = (x2-x1)*(y3-y1) - (x3-x1)*(y2-y1);
        float Area = std::abs(D) / 2.0f;

        float factor = (density * specific_heat * Area) / 12.0f;

        c_matrix[0][0] = 2.0f * factor;
        c_matrix[1][1] = 2.0f * factor;
        c_matrix[2][2] = 2.0f * factor;

        c_matrix[0][1] = c_matrix[1][0] = 1.0f * factor;
        c_matrix[0][2] = c_matrix[2][0] = 1.0f * factor;
        c_matrix[1][2] = c_matrix[2][1] = 1.0f * factor;

        return c_matrix;
    }

    void aggregate(Matrix &Global, Element element, Matrix &Local){
        for(int i=0; i<3;i++){
            for(int j=0; j<3;j++){
                int glob_i = element.node_ids[i];
                int glob_j = element.node_ids[j];
                Global[glob_i][glob_j] += Local[i][j]; 
            }
        }
    }

    void aggregate_p_vec(Matrix &P_vec, Element element, Matrix &Local){
        for(int i=0; i<3; i++){
            int glob_i = element.node_ids[i];
            P_vec[glob_i][0] += Local[i][0];

        }
    }

    void write_to_vtu_file(int step, const std::vector<Fem::Node> &nodes, const std::vector<double> &temp, const std::vector<Fem::Element> &elements)
    {

        std::ostringstream fname;
        fname << "sol_" << step << ".vtu";
        const std::string path = "Data/"+fname.str();

        std::ofstream out(path);
        if (!out.is_open()) {
            std::perror(("Nie można otworzyć pliku " + path).c_str());
            return;
        }

        out << R"(<?xml version="1.0"?>)"
            << "\n<VTKFile type=\"UnstructuredGrid\" version=\"0.1\" byte_order=\"LittleEndian\">\n"
            "  <UnstructuredGrid>\n"
            "    <Piece NumberOfPoints=\"" << nodes.size()
            << "\" NumberOfCells=\"" << elements.size() << "\">\n";

        // --- 4) Points ---
        out << "      <Points>\n"
            "        <DataArray type=\"Float32\" NumberOfComponents=\"3\" format=\"ascii\">\n";
        out << std::setprecision(6);
        for (auto& n : nodes) {
            out << "          " << n.x << " " << n.y << " 0\n";
        }
        out << "        </DataArray>\n"
            "      </Points>\n";

        // --- 5) Cells ---
        out << "      <Cells>\n"
            "        <DataArray type=\"Int32\" Name=\"connectivity\" format=\"ascii\">\n";
        for (auto& e : elements) {
            out << "          "
                << (e.node_ids[0]) << " "
                << (e.node_ids[1]) << " "
                << (e.node_ids[2]) << "\n";
        }
        out << "        </DataArray>\n"
            "        <DataArray type=\"Int32\" Name=\"offsets\" format=\"ascii\">\n";
        int offset = 0;
        for (size_t i = 0; i < elements.size(); ++i) {
            offset += 3;  // Zmieniono z 3 na 4 (czworokąt ma 4 węzły)
            out << "          " << offset << "\n";
        }
        out << "        </DataArray>\n"
            "        <DataArray type=\"UInt8\" Name=\"types\" format=\"ascii\">\n";
        for (size_t i = 0; i < elements.size(); ++i) {
            out << "          5\n";  // Zmieniono z 5 na 9 (typ VTK_QUAD)
        }
        out << "        </DataArray>\n"
            "      </Cells>\n";

        // --- 6) PointData: temperatura ---
        out << "      <PointData Scalars=\"Temperature\">\n"
            "        <DataArray type=\"Float32\" Name=\"Temperature\" format=\"ascii\">\n";
        for (double t : temp) {
            out << "          " << t << "\n";
        }
        out << "        </DataArray>\n"
            "      </PointData>\n";

        out << "    </Piece>\n"
            "  </UnstructuredGrid>\n"
            "</VTKFile>\n";

        out.close();
    }

    void Solution::solve(bool write_vtu, bool print_conf) {
        Fem::Matrix Global(conf.node_number, conf.node_number);

        std::vector<double> t0(conf.node_number);
        std::vector<double> t(conf.node_number);

        for(int i=0; i<conf.node_number;i++){
            t0[i] = conf.init_temperature;
        }

        for(int i=0; i<conf.node_number; i++){
            for(int j=0; j<conf.node_number; j++){
                Global[i][j] = Global_H[i][j] + (Global_C[i][j] / conf.time_step);
            }
        }

        std::cout<<"Beginning time integration...\n";
        for(float i=conf.time_step; i<conf.total_time; i+=conf.time_step){
            for(int j=0; j<conf.node_number; j++){
                double rhs = 0;
                for(int k = 0; k<conf.node_number; k++){
                    rhs += (Global_C[j][k] / conf.time_step) * t0[k];
                }
                rhs += Global_P[j][0];
                t[j] = rhs;
            }

            t = Gauss(Global, t);
            //std::cout<<"\nTemperature at time " << i << "s:";

            //std::cout << "\nMIN: " << *std::min_element(t.begin(), t.end()) << " MAX: " << *std::max_element(t.begin(), t.end()) << std::endl;
            
            if(write_vtu){
                write_to_vtu_file((int)i, nodes, t, elements);
            }

            showProgress(i, conf.total_time);

            t0=t;
        }
    }
}