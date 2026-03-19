#pragma once

#include "matrix/matrix.h"

namespace Fem{
    extern

    float N1(float xi, float eta);
    float N2(float xi, float eta);
    float N3(float xi, float eta);

    void showProgress(int current, int max);

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

    struct Element{
        int id;
        int node_ids[3];
        float area;

        Matrix H_local;
        Matrix H_bc;
        Matrix P;
        Matrix C;

        Element(int n1, int n2, int n3) : H_local(3, 3), H_bc(3,3), P(3,1), C(3,3) {
            this->node_ids[0] = n1;
            this->node_ids[1] = n2;
            this->node_ids[2] = n3;
        }

        float dN1dx(std::vector<Node> &nodes);
        float dN2dx(std::vector<Node> &nodes);
        float dN3dx(std::vector<Node> &nodes);
        float dN1dy(std::vector<Node> &nodes);
        float dN2dy(std::vector<Node> &nodes);
        float dN3dy(std::vector<Node> &nodes);
    };

    std::vector<Fem::Node> load_nodes(std::string file_name);
    std::vector<Fem::Element> load_elements(std::string file_name);
    std::vector<Fem::Node> load_bc(std::string file_name, std::vector<Fem::Node>& nodes);
    
    struct GlobalData{
        float total_time;
        float time_step;
        float conductivity;
        float init_temperature;
        float density;
        float specific_heat;
        int node_number;
        int elem_number;

        GlobalData(){
            this->total_time=500;
            this->time_step=50;
            this->conductivity=25;
            this->init_temperature=100;
            this->density=7800;
            this->specific_heat=700;
            this->node_number=0;
            this->elem_number=0;
        }
    };

    GlobalData load_configuration(std::string file_name);
    void print_config(Fem::GlobalData configuration);

    Matrix calc_local_H(Element &local_el, std::vector<Node> &nodes, float conductivity);
    Matrix calc_local_Hbc(Element &local_el, std::vector<Node> &nodes);
    Matrix calc_p_vec(Element &local_el, std::vector<Node> &nodes);

    Matrix calc_c(Element &local_el, std::vector<Node> &nodes, 
        float density, float specific_heat);

    void aggregate(Matrix &Global, Element element, Matrix &Local);
    void aggregate_p_vec(Matrix &P_vec, Element element, Matrix &Local);
    void write_to_vtu_file(int step, const std::vector<Fem::Node> &nodes, 
        const std::vector<double> &temp, const std::vector<Fem::Element> &elements);
    
    struct Solution{
        Matrix Global_H;
        Matrix Global_C;
        Matrix Global_P;
        GlobalData conf;
        std::vector<Node> nodes;
        std::vector<Element> elements;
        
        Solution(std::string filename): Global_H(3,3), Global_C(3,3), Global_P(3,1)
        {
            std::string filepath = filename;
            this->conf = load_configuration(filepath);
            //std::cout<<"loaded config\n";
            this->nodes = load_nodes(filepath);
            //std::cout<<"loaded nodes, nodes size = "<<nodes.size()<<std::endl;
            this->elements = load_elements(filepath);
            //std::cout<<"loaded elements, elements size = "<<elements.size()<<std::endl;
            this->nodes = load_bc(filepath, this->nodes);
            //std::cout<<"loaded BC\n";

            print_config(this->conf);

            this->Global_H = Matrix(conf.node_number,conf.node_number);
            this->Global_C = Matrix(conf.node_number,conf.node_number);
            this->Global_P = Matrix(conf.node_number,1);

            //tworzenie macierzy dla każdego elementu
            std::cout<<"Assembling elemental matrices...\n";
            int i =0;
            int max_iter = this->elements.size();
            for(Element &element: this->elements){
                
                element.H_local = calc_local_H(element, this->nodes, this->conf.conductivity);
                element.H_bc = calc_local_Hbc(element, this->nodes);
                element.P = calc_p_vec(element, this->nodes);
                element.C = calc_c(element, this->nodes, this->conf.density, this->conf.specific_heat);
                
                //sumowanie H_l i H_bc
                for(int row=0; row<3; row++){
                    for(int col=0; col<3; col++){
                        element.H_local[row][col] += element.H_bc[row][col];
                    }
                }
                
                i++;
                showProgress(i, max_iter);
            }

            //agreagacja
            std::cout<<"Agregating matrices...\n";
            i=0;
            for(Fem::Element &it:elements){
                aggregate(this->Global_H, it, it.H_local);
                aggregate(this->Global_C, it, it.C);
                aggregate_p_vec(this->Global_P, it, it.P);
                i++;
                showProgress(i, max_iter);
            }
        }

        /*
        Solution(std::vector<Fem::Node> &nodes, std::vector<Fem::Element> &elements, GlobalData &conf): Global_H(4,4), Global_C(4,4), Global_P(4,4){
            
        }*/

        void solve(bool write_vtu, bool print_conf, const std::string& solver_type);
    };
}

void fem_solve(std::string solver_type);