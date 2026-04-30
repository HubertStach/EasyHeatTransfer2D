#pragma once

#include <utility>

#include "matrix/matrix.h"
#include  "../progress_bar.h"

namespace Fem{

    double N1_tr(double xi, double eta);
    double N2_tr(double xi, double eta);
    double N3_tr(double xi, double eta);

    double N1_q(double xi, double eta);
    double N2_q(double xi, double eta);
    double N3_q(double xi, double eta);
    double N4_q(double xi, double eta);

    double dN1dxi_q(double eta);
    double dN2dxi_q(double eta);
    double dN3dxi_q(double eta);
    double dN4dxi_q(double eta);

    double dN1deta_q(double xi);
    double dN2deta_q(double xi);
    double dN3deta_q(double xi);
    double dN4deta_q(double xi);

    struct Node{
        double x;
        double y;

        Node(){
        this-> x = 0; 
        this-> y = 0; 
        }

        Node(double x, double y){
            this->x = x;
            this->y = y;
        }
    };

    struct EdgeBC {
        bool exist = false;
        double flux = 0.0;
        double alfa = 0.0;
        double t_ext = 0.0;

        EdgeBC() = default;
        EdgeBC(double flux, double alfa, double t_ext) : exist(true), flux(flux), alfa(alfa), t_ext(t_ext) {}
    };

    struct Triangle{
        int id{};
        int node_ids[3]{};
        double area;
        EdgeBC edge_bc[3];

        Matrix H_local;
        Matrix H_bc;
        Matrix P;
        Matrix C;

        Triangle(int n1, int n2, int n3) : H_local(3, 3), H_bc(3,3), P(3,1), C(3,3) {
            this->node_ids[0] = n1;
            this->node_ids[1] = n2;
            this->node_ids[2] = n3;
        }

        double dN1dx_tr(std::vector<Node> &nodes);
        double dN2dx_tr(std::vector<Node> &nodes);
        double dN3dx_tr(std::vector<Node> &nodes);
        double dN1dy_tr(std::vector<Node> &nodes);
        double dN2dy_tr(std::vector<Node> &nodes);
        double dN3dy_tr(std::vector<Node> &nodes);
    };

    struct Quad{
        int id;
        int node_ids[4];
        EdgeBC edge_bc[4];

        Matrix H_local;
        Matrix H_bc;
        Matrix P;
        Matrix C;

        Quad(int id, int n1, int n2, int n3, int n4) : H_local(4, 4), H_bc(4,4), P(4,1), C(4,4) {
            this->id = id;
            this->node_ids[0] = n1;
            this->node_ids[1] = n2;
            this->node_ids[2] = n3;
            this->node_ids[3] = n4;
        }

        Matrix jacobian_mat(Quad &element, std::vector<Node> &nodes, double pc_xi, double pc_eta);
        double det_jacobian(Matrix jacobian_mat);
        Matrix inv_jacobian_mat(Matrix jacobian_mat);
    };

    std::vector<Node> load_nodes(const std::string& file_name);
    std::vector<Triangle> load_triangles(const std::string& file_name);
    std::vector<Quad> load_quad_elements(std::string file_name);
    void load_bc(const std::string& file_name, std::vector<Triangle>& triangles, std::vector<Quad>& quads);
    
    struct GlobalData{
        double total_time;
        double time_step;
        double conductivity;
        double init_temperature;
        double density;
        double specific_heat;
        int node_number;
        int trian_number;
        int quad_number;

        GlobalData(){
            this->total_time=500;
            this->time_step=50;
            this->conductivity=25;
            this->init_temperature=100;
            this->density=7800;
            this->specific_heat=700;
            this->node_number=0;
            this->trian_number=0;
            this->quad_number=0;
        }
    };

    GlobalData load_configuration(const std::string& file_name);
    void print_config(Fem::GlobalData configuration);

    //---------Trójkąty-----------
    Matrix calc_local_H_tr(Triangle &local_el, std::vector<Node> &nodes, double conductivity);
    Matrix calc_local_Hbc_tr(Triangle &local_el, std::vector<Node> &nodes);
    Matrix calc_p_vec_tr(const Triangle &local_el, const std::vector<Node> &nodes);
    Matrix calc_c_tr(const Triangle &local_el, const std::vector<Node> &nodes, double density, double specific_heat, int c_lump);
    void aggregate_tr(Matrix &Global, const Triangle& element, Matrix &Local);
    void aggregate_p_vec_tr(Matrix &P_vec, const Triangle& element, Matrix &Local);
    //----------------------------

    //---------Czworokąty-----------
    Matrix calc_local_H_q(Quad &element, std::vector<Node> &nodes, double conductivity);
    Matrix calc_local_Hbc_q(Quad &element, std::vector<Node> &nodes);
    Matrix calc_P_q(Quad &element, std::vector<Node> &nodes);
    Matrix calc_local_C_q(Quad &element, std::vector<Node> &nodes, double rho, double c);
    void aggregate_q(Matrix &Global, Quad element, Matrix &Local);
    void aggregate_p_vec_q(Matrix &P_vec, Quad element, Matrix &Local);
    //----------------------------

    void write_to_vtu_file(int step, const std::vector<Fem::Node> &nodes, const std::vector<double> &temp,
        const std::vector<Fem::Triangle> &elements, const std::vector<Fem::Quad> &quads);
    
    struct Solution{
        Matrix Global_H;
        Matrix Global_C;
        Matrix Global_P;
        GlobalData conf;
        std::vector<Node> nodes;
        std::vector<Triangle> triangles;
        std::vector<Quad> quads;

        std::string solver_type;

        double x_char = 100.0; //shortest edge in mesh -> characteristic for CFL number in explicit Euler
        void calc_x_char();

        explicit Solution(std::string filename, const std::string& solver_type): Global_H(3,3), Global_C(3,3), Global_P(3,1)
        {
            this->solver_type = solver_type;
            std::string filepath = std::move(filename);
            this->conf = load_configuration(filepath);
            this->nodes = load_nodes(filepath);
            this->triangles = load_triangles(filepath);
            this->quads = load_quad_elements(filepath);
            load_bc(filepath, this->triangles, this->quads);

            this->calc_x_char();

            print_config(this->conf);

            this->Global_H = Matrix(conf.node_number,conf.node_number);
            this->Global_C = Matrix(conf.node_number,conf.node_number);
            this->Global_P = Matrix(conf.node_number,1);

            //tworzenie macierzy dla każdego elementu
            int i =0;
            int max_iter = this->triangles.size();
            int c_lump = 0;
            if (this->solver_type == "explicit_euler") {
                c_lump = 1;
            }

            std::cout<<"Assembling triangular elements matrices...\n";
            for(Triangle &element: this->triangles){
                element.H_local = calc_local_H_tr(element, this->nodes, this->conf.conductivity);
                element.H_bc = calc_local_Hbc_tr(element, this->nodes);
                element.P = calc_p_vec_tr(element, this->nodes);
                element.C = calc_c_tr(element, this->nodes, this->conf.density, this->conf.specific_heat, c_lump);
                
                //sumowanie H_l i H_bc
                for(int row=0; row<3; row++){
                    for(int col=0; col<3; col++){
                        element.H_local[row][col] += element.H_bc[row][col];
                    }
                }
                
                i++;
                showProgress(i, max_iter);
            }

            i=0;
            std::cout<<"Assembling quad elements matrices...\n";
            for (Quad &quad: this->quads) {
                quad.H_local = calc_local_H_q(quad, this->nodes, this->conf.conductivity);
                quad.H_bc = calc_local_Hbc_q(quad, this->nodes);
                quad.P = calc_P_q(quad, this->nodes);
                quad.C = calc_local_C_q(quad, this->nodes, this->conf.density, this->conf.specific_heat);

                for(int row=0; row<4; row++){
                    for(int col=0; col<4; col++){
                        quad.H_local[row][col] += quad.H_bc[row][col];
                    }
                }

                i++;
                showProgress(i, max_iter);
            }

            //agreagacja
            std::cout<<"Agregating matrices...\n";
            i=0;
            for(Fem::Triangle &it:triangles){
                aggregate_tr(this->Global_H, it, it.H_local);
                aggregate_tr(this->Global_C, it, it.C);
                aggregate_p_vec_tr(this->Global_P, it, it.P);
                i++;
                showProgress(i, max_iter);
            }
            i=0;
            for(Fem::Quad &it:quads){
                aggregate_q(this->Global_H, it, it.H_local);
                aggregate_q(this->Global_C, it, it.C);
                aggregate_p_vec_q(this->Global_P, it, it.P);
                i++;
                showProgress(i, max_iter);
            }
        }

        /*
        Solution(std::vector<Fem::Node> &nodes, std::vector<Fem::Element> &elements, GlobalData &conf): Global_H(4,4), Global_C(4,4), Global_P(4,4){
            
        }*/

        void solve(bool write_vtu, bool print_conf);
    };
}

void fem_solve(const std::string& solver_type);