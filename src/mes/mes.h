#pragma once

#include <utility>

#include "matrix/matrix.h"
#include  "../progress_bar.h"

namespace Fem{

    float N1_tr(float xi, float eta);
    float N2_tr(float xi, float eta);
    float N3_tr(float xi, float eta);

    float N1_q(float xi, float eta);
    float N2_q(float xi, float eta);
    float N3_q(float xi, float eta);
    float N4_q(float xi, float eta);

    float dN1dxi_q(float eta);
    float dN2dxi_q(float eta);
    float dN3dxi_q(float eta);
    float dN4dxi_q(float eta);

    float dN1deta_q(float xi);
    float dN2deta_q(float xi);
    float dN3deta_q(float xi);
    float dN4deta_q(float xi);

    struct BC_node{
        int id;

        bool exist=false;

        float flux;
        float alfa;
        float t_ext;

        BC_node(){
            this->id=0;
            this->flux=0.0;
            this->alfa=0.0;
            this->t_ext=0.0;
        }

        BC_node(int id, float flux, float alfa, float t_ext){
            this->exist=true;
            this->id=id;
            this->flux=flux;
            this->alfa=alfa;
            this->t_ext=t_ext;
        }
    };

    struct Node{
        float x;
        float y;

        BC_node bc;

        Node(){
        this-> x = 0; 
        this-> y = 0; 
        }

        Node(float x, float y){
            this->x = x;
            this->y = y;
        }
    };

    struct Triangle{
        int id{};
        int node_ids[3]{};
        float area{};

        Matrix H_local;
        Matrix H_bc;
        Matrix P;
        Matrix C;

        Triangle(int n1, int n2, int n3) : H_local(3, 3), H_bc(3,3), P(3,1), C(3,3) {
            this->node_ids[0] = n1;
            this->node_ids[1] = n2;
            this->node_ids[2] = n3;
        }

        float dN1dx_tr(std::vector<Node> &nodes);
        float dN2dx_tr(std::vector<Node> &nodes);
        float dN3dx_tr(std::vector<Node> &nodes);
        float dN1dy_tr(std::vector<Node> &nodes);
        float dN2dy_tr(std::vector<Node> &nodes);
        float dN3dy_tr(std::vector<Node> &nodes);
    };

    struct Quad{
        int id;
        int node_ids[4];

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

        Matrix jacobian_mat(Quad &element, std::vector<Node> &nodes, float pc_xi, float pc_eta);
        float det_jacobian(Matrix jacobian_mat);
        Matrix inv_jacobian_mat(Matrix jacobian_mat);
    };

    std::vector<Node> load_nodes(const std::string& file_name);
    std::vector<Triangle> load_triangles(const std::string& file_name);
    std::vector<Quad> load_quad_elements(std::string file_name);
    std::vector<Node> load_bc(const std::string& file_name, std::vector<Node>& nodes);
    
    struct GlobalData{
        float total_time;
        float time_step;
        float conductivity;
        float init_temperature;
        float density;
        float specific_heat;
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
    Matrix calc_local_H_tr(Triangle &local_el, std::vector<Node> &nodes, float conductivity);
    Matrix calc_local_Hbc_tr(Triangle &local_el, std::vector<Node> &nodes);
    Matrix calc_p_vec_tr(const Triangle &local_el, const std::vector<Node> &nodes);
    Matrix calc_c_tr(const Triangle &local_el, const std::vector<Node> &nodes, float density, float specific_heat, int c_lump);
    void aggregate_tr(Matrix &Global, const Triangle& element, Matrix &Local);
    void aggregate_p_vec_tr(Matrix &P_vec, const Triangle& element, Matrix &Local);
    //----------------------------

    //---------Czworokąty-----------
    Matrix calc_local_H_q(Quad &element, std::vector<Node> &nodes, float conductivity);
    Matrix calc_local_Hbc_q(Quad &element, std::vector<Node> &nodes);
    Matrix calc_P_q(Quad &element, std::vector<Node> &nodes);
    Matrix calc_local_C_q(Quad &element, std::vector<Node> &nodes, float rho, float c);
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
            this->nodes = load_bc(filepath, this->nodes);

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