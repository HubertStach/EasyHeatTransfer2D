#pragma once

#include <utility>
#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <algorithm>
#include <../../include/Eigen/Dense>
#include "../progress_bar.h"
#include "../solver/cholesky_ldl.h"

namespace Fem {

using Matrix = Eigen::MatrixXd;

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

// Warunek Dirichleta jest warunkiem węzłowym.
struct BC_node{
    bool dirichlet = false;   // czy w węźle zadano temperaturę
    double dir_temp = 0.0;    // zadana temperatura
};

// Warunki Neumanna i Robina należą do krawędzi brzegowej.
struct EdgeBC{
    double flux = 0.0;   // strumień ciepła q_N
    double alfa = 0.0;   // współczynnik wymiany ciepła
    double t_ext = 0.0;  // temperatura otoczenia
};

// Klucz krawędzi niezależny od kolejności węzłów: (min, max)
using EdgeKey = std::pair<int,int>;
using EdgeBCMap = std::map<EdgeKey, EdgeBC>;

inline EdgeKey edge_key(int a, int b) { return {std::min(a,b), std::max(a,b)}; }

// Zwraca warunek przypisany krawędzi (a,b) albo nullptr, jeśli krawędź nie leży na brzegu z WB
inline const EdgeBC* find_edge_bc(const EdgeBCMap &edge_bc, int a, int b) {
    auto it = edge_bc.find(edge_key(a, b));
    return it == edge_bc.end() ? nullptr : &it->second;
}

struct Node{
    double x;
    double y;

    BC_node bc;

    Node(){
    this-> x = 0;
    this-> y = 0;
    }

    Node(double x, double y){
        this->x = x;
        this->y = y;
    }
};

struct Triangle{
    int id{};
    int node_ids[3]{};
    double area{};

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
std::vector<Node> load_dirichlet(const std::string& file_name, std::vector<Node>& nodes);
EdgeBCMap load_edge_bc(const std::string& file_name);

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
Matrix calc_local_Hbc_tr(Triangle &local_el, std::vector<Node> &nodes, const EdgeBCMap &edge_bc);
Matrix calc_p_vec_tr(const Triangle &local_el, const std::vector<Node> &nodes, const EdgeBCMap &edge_bc);
Matrix calc_c_tr(const Triangle &local_el, const std::vector<Node> &nodes, double density, double specific_heat, int c_lump);
void aggregate_tr(Matrix &Global, const Triangle& element, Matrix &Local);
void aggregate_p_vec_tr(Matrix &P_vec, const Triangle& element, Matrix &Local);
//----------------------------

//---------Czworokąty-----------
Matrix calc_local_H_q(Quad &element, std::vector<Node> &nodes, double conductivity);
Matrix calc_local_Hbc_q(Quad &element, std::vector<Node> &nodes, const EdgeBCMap &edge_bc);
Matrix calc_P_q(Quad &element, std::vector<Node> &nodes, const EdgeBCMap &edge_bc);
Matrix calc_local_C_q(Quad &element, std::vector<Node> &nodes, double rho, double c, int c_lump);
void aggregate_q(Matrix &Global, Quad element, Matrix &Local);
void aggregate_p_vec_q(Matrix &P_vec, Quad element, Matrix &Local);
//----------------------------

void write_to_vtu_file(int step, const std::vector<Fem::Node> &nodes, const std::vector<double> &temp, const std::vector<Fem::Triangle> &elements, const std::vector<Fem::Quad> &quads);

struct Solution{
    Matrix Global_H;
    Matrix Global_C;
    Matrix Global_P;
    GlobalData conf;
    std::vector<Node> nodes;
    std::vector<Triangle> triangles;
    std::vector<Quad> quads;
    EdgeBCMap edge_bc;

    std::string solver_type;

    struct DirichletData {
        std::vector<char> mask;   // 1 = węzeł z zadaną temperaturą
        Eigen::VectorXd values;   // zadane temperatury (0 w pozostałych węzłach)
        Eigen::VectorXd lift;     // A * values: wkład znanych temperatur do prawej strony
        Eigen::VectorXd diag;     // przekątna A
    };

    bool is_dirichlet(int node) const;
    DirichletData apply_dirichlet_matrix(SparseMatrix &A) const;
    static void apply_dirichlet_rhs(const DirichletData &d, Eigen::VectorXd &b);

    void solve_theta(double theta, bool write_vtu);

    explicit Solution(std::string filename, const std::string& solver_type): Global_H(3,3), Global_C(3,3), Global_P(3,1)
    {
        this->solver_type = solver_type;
        std::string filepath = std::move(filename);
        this->conf = load_configuration(filepath);
        this->nodes = load_nodes(filepath);
        this->triangles = load_triangles(filepath);
        this->quads = load_quad_elements(filepath);
        this->nodes = load_dirichlet(filepath, this->nodes);
        this->edge_bc = load_edge_bc(filepath);

        print_config(this->conf);

        this->Global_H = Matrix::Zero(conf.node_number,conf.node_number);
        this->Global_C = Matrix::Zero(conf.node_number,conf.node_number);
        this->Global_P = Matrix::Zero(conf.node_number,1);

        int i =0;
        int max_iter = this->triangles.size();
        int c_lump = 0;
        if (this->solver_type == "explicit_euler") {
            c_lump = 1;
        }

        std::cout<<"Assembling triangular elements matrices...\n";
        for(Triangle &element: this->triangles){
            element.H_local = calc_local_H_tr(element, this->nodes, this->conf.conductivity);
            element.H_bc = calc_local_Hbc_tr(element, this->nodes, this->edge_bc);
            element.P = calc_p_vec_tr(element, this->nodes, this->edge_bc);
            element.C = calc_c_tr(element, this->nodes, this->conf.density, this->conf.specific_heat, c_lump);

            element.H_local += element.H_bc;

            i++;
            showProgress(i, max_iter);
        }

        i=0;
        std::cout<<"Assembling quad elements matrices...\n";
        for (Quad &quad: this->quads) {
            quad.H_local = calc_local_H_q(quad, this->nodes, this->conf.conductivity);
            quad.H_bc = calc_local_Hbc_q(quad, this->nodes, this->edge_bc);
            quad.P = calc_P_q(quad, this->nodes, this->edge_bc);
            quad.C = calc_local_C_q(quad, this->nodes, this->conf.density, this->conf.specific_heat, c_lump);

            quad.H_local += quad.H_bc;

            i++;
            showProgress(i, max_iter);
        }

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

    void solve(bool write_vtu, bool print_conf);
    void solve_stationary(bool write_vtu);

    // Max stable dt for explicit Euler, from the assembled matrices (Gershgorin
    // bound on C^-1 H). Accounts for element shape AND convection BCs, unlike a
    // purely geometric CFL estimate.
    double stability_dt_max();
};
}

void fem_solve(const std::string& solver_type);
