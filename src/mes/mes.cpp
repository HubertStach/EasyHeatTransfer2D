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

    float N1(float xi, float eta)
    {
        return 1-xi-eta;
    }
    float N2(float xi, float eta)
    {
        return xi;
    }
    float N3(float xi, float eta)
    {
        return eta;
    }

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
                    nodes[node_id-1].bc = temp_bc;
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

    Matrix calc_jacobian_mat(Element &element, std::vector<Node> &nodes){
        Matrix jacobian(2,2);

        Node n1 = nodes[element.node_ids[0]-1];
        Node n2 = nodes[element.node_ids[1]-1];
        Node n3 = nodes[element.node_ids[2]-1];

        jacobian[0][0] = n2.x-n1.x;
        jacobian[0][1] = n3.x-n1.x;
        jacobian[1][0] = n2.y-n1.y;
        jacobian[1][1] = n3.y-n1.y;

        return jacobian;
    }

    float calc_jacobian(Matrix &J){
        return (J[0][0]*J[1][1])-(J[1][0]*J[0][1]);
    }

    Matrix inverse_jacobian_matrix(Matrix &J){
        float inv_det_J = 1/calc_jacobian(J);

        Matrix inv_jac(2,2);

        inv_jac[0][0] = inv_det_J*J[1][1];
        inv_jac[1][1] = inv_det_J*J[0][0];
        inv_jac[1][0] = -inv_det_J*J[1][0];
        inv_jac[0][1] = -inv_det_J*J[0][1];

        return inv_jac;
    }

    Matrix calc_local_H(Element &local_el, std::vector<Node> &nodes, 
        float conductivity)
    {
        Matrix J = calc_jacobian_mat(local_el, nodes);
        double detJ = calc_jacobian(J);
        Matrix invJ = inverse_jacobian_matrix(J);

        Matrix dNdx(3,1), dNdy(3,1);
        for(int i=0; i<3; ++i){
            dNdx[i][0] = invJ[0][0]*dNdxi[i] + invJ[0][1]*dNdeta[i];
            dNdy[i][0] = invJ[1][0]*dNdxi[i] + invJ[1][1]*dNdeta[i];
        }

        Matrix H_local = (dNdx*dNdx.transpose()) + (dNdy*dNdy.transpose());

        double area = 0.5 * std::fabs(detJ);
        double scale = conductivity * area;

        for(int i=0; i<3; ++i)
            for(int j=0; j<3; ++j)
                H_local[i][j] *= scale;

        return H_local;
    }

    Matrix calc_local_Hbc(Element &local_el, std::vector<Node> &nodes)
    {
        Matrix hbc(3,3);

        for(int i=0; i<3; i++){//pętla po bokach

            //odejmujemy tutaj 1 od id żeby mieć przeniesienia
            //z indeksowania o bazie 1, które jest w elemencie
            //na indeksowanie z bazą 0, które jest w vector nodes
            int id1 = local_el.node_ids[i]-1;
            int id2 = local_el.node_ids[(i+1)%3]-1;
            
            if (!(nodes[id1].bc.exist && nodes[id2].bc.exist)) continue;

            float alfa = nodes[id1].bc.alfa;
            double det_J = 0.5*dist(nodes[id1], nodes[id2]);
            Matrix hbc_i(3,3);

            for(int j=0; j<3; j++){//pętla po pc
                int k = i*3;
                //std::cout<<"k = "<<k<<"\n";
                //std::cout<<"k+j = "<<k+j<<"\n";
                Matrix h_pc(3,1);
                h_pc[0][0] = n1_bc_val[j+k];
                h_pc[1][0] = n2_bc_val[j+k];
                h_pc[2][0] = n3_bc_val[j+k];

                Matrix h_pc_temp(3,3);

                h_pc_temp = h_pc*h_pc.transpose();

                //std::cout<<h_pc_temp;
                
                for(int a=0; a<3; a++){
                    for(int b=0; b<3; b++){
                        h_pc_temp[a][b] *= weights_bc[j];
                    }
                }

                hbc_i = hbc_i + h_pc_temp;
            }
            
            for(int i=0; i<3; i++){
                for(int j=0; j<3; j++){
                    hbc_i[i][j] *= det_J*alfa;
                }
            }

            //std::cout<<hbc_i<<"\n";
            hbc = hbc+hbc_i;
        }

        return hbc;
    }

    Matrix calc_p_vec(Element &local_el, std::vector<Node> &nodes)
    {
        Matrix p_vec(3,1);

        for(int edge =0; edge<3; edge++){
            int id1 = local_el.node_ids[edge]-1;
            int id2 = local_el.node_ids[(edge+1)%3]-1;
            
            if (!(nodes[id1].bc.exist && nodes[id2].bc.exist)) continue;

            //W.B. Robina
            float t_ext = nodes[id1].bc.t_ext;
            float alfa = nodes[id1].bc.alfa;

            //W.B. Neumanna;
            float flux = nodes[id1].bc.flux;

            double det_J = 0.5*dist(nodes[id1], nodes[id2]);

            Matrix p_edge(3,1);

            for(int pc=0; pc<3;pc++){//pętla po pc na jednym boku
                int k = edge*3;//upewniamy się że przechodzimy przez wszystkie 9 boków

                Matrix NS(3,1);
                NS[0][0] = n1_bc_val[pc+k];
                NS[1][0] = n2_bc_val[pc+k];
                NS[2][0] = n3_bc_val[pc+k];

                for(int i=0; i<3; i++){
                    p_edge[i][0] += NS[i][0]*t_ext*weights_bc[pc];
                }
            }
            
            //W.B. Robina
            for(int i=0; i<3; i++){
                p_vec[i][0] += p_edge[i][0]*alfa*det_J;
            }

            //W.B. Neumanna

            for(int i=0; i<3; i++){
                p_vec[i][0] += p_edge[i][0]*det_J*flux;
            }
        }

        return p_vec;
    }

    Matrix calc_c(Element &local_el, std::vector<Node> &nodes, 
        float density, float specific_heat)
    {
        Matrix c_mat(3,3);
        Matrix el_jac = calc_jacobian_mat(local_el, nodes);
        float det_J = calc_jacobian(el_jac);
        //std::cout<<det_J<<"\n";

        for(int i=0; i<3; i++){
            Matrix c_i(3,1);
            c_i[0][0] = N1(pc_xi[i], pc_eta[i]);
            c_i[1][0] = N2(pc_xi[i], pc_eta[i]);
            c_i[2][0] = N3(pc_xi[i], pc_eta[i]);

            //std::cout<<c_i<<"\n";
            Matrix c_temp = c_i*c_i.transpose();
            //std::cout<<"Before\n"<<c_temp<<"\n";

            double A = 0.5*std::fabs(det_J);

            for(int j=0; j<3; j++){
                for(int k=0; k<3; k++){
                    c_temp[j][k] *= weights_pc[i]*A*density*specific_heat;
                }
            }
            //std::cout<<"After\n"<<c_temp<<"\n";

            c_mat = c_mat + c_temp;
            //std::cout<<c_mat<<"\n";
        }

        return c_mat;
    }

    void aggregate(Matrix &Global, Element element, Matrix &Local){
        for(int i=0; i<3;i++){
            for(int j=0; j<3;j++){
                int glob_i = element.node_ids[i]-1;
                int glob_j = element.node_ids[j]-1;
                Global[glob_i][glob_j] += Local[i][j]; 
            }
        }
    }

    void aggregate_p_vec(Matrix &P_vec, Element element, Matrix &Local){
        for(int i=0; i<3; i++){
            int glob_i = element.node_ids[i]-1;
            P_vec[glob_i][0] += Local[i][0];

        }
    }

    void write_to_vtu_file(int step, const std::vector<Fem::Node> &nodes, const std::vector<double> &temp, const std::vector<Fem::Element> &elements)
    {

        std::ostringstream fname;
        fname << "sol_" << step << ".vtu";
        const std::string path = "../Data/"+fname.str();

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
                << (e.node_ids[0] - 1) << " "
                << (e.node_ids[1] - 1) << " "
                << (e.node_ids[2] - 1) << "\n";
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

            t0=t;
        }
    }
}