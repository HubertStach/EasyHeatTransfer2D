#pragma once
#include <string>
#include <vector>
#include "raylib.h"
#include "afm.h"

namespace geo {
    struct Bc {
        bool is_bc=false;
        bool initialised=false;
        float flux=0.0f;
        float alfa=0.0f;
        float t_ext=0.0f;
    };
    
    struct Node{
        float x;
        float y;
        Bc bc;
        Color color = RED;

        Node();
        Node(float x, float y);
        Node(float x, float y, bool is_bc);
    };

    struct Edge{
        int node_ids[2]{};
        Bc bc_edge;

        Edge();
        Edge(int n1, int n2);
    };

    struct Polygon{
        std::vector<int> node_ids;
        std::vector<int> bc_node_ids;
        std::vector<int> edge_ids;

        Polygon();
        explicit Polygon(std::vector<int> node_ids);
    };

    struct Triangle{
        int node_ids[3]{};

        Triangle();
        Triangle(int n1, int n2, int n3);
    };

    struct Quad {
        int node_ids[4]{};

        Quad();
        Quad(int n1, int n2, int n3, int n4);
    };

    struct Mesh{
        std::vector<Node> nodes;
        std::vector<Node> initial_bc_nodes;
        std::vector<Edge> edges;
        std::vector<Edge> initial_edges;
        std::vector<Triangle> triangles;
        std::vector<Quad> quads;
        std::vector<Polygon> polygons;
        AdvancingFront* afm=nullptr;

        bool mesh_created = false;

        Mesh();
        void add_point(float x, float y, bool is_bc = true);
        void pop_point();

        float max_bc_len = 0.0f;

        //------ tworzenie siatki ------
        void interpolate_bc_points(float spacing);
        std::vector<geo::Node> super_triangle();
        bool inside_circumcircle(geo::Triangle tr, int node_id);
        void circumcenter(const geo::Triangle &tr, geo::Node &center);
        bool same_triangle(geo::Triangle A, geo::Triangle B);
        static bool is_boundary_edge(std::vector<geo::Triangle> &triangles, geo::Edge edge);
        void triangulate();
        bool point_in_mesh(float x, float y);

        void create_nodes_SH_LO(float spacing);
        void create_nodes_2(float spacing);

        void init_afm();
        void create_mesh(float spacing);
        

        //------Rysowanie siatki itd.------
        void draw_nodes(float size) const;
        void draw_edges();
        void draw_tr();
        void draw_q();
        void draw_tr(std::vector<double> &temp, float max, float min) const;
        void draw_tr_grad(std::vector<double> &temp, float max, float min) const;

        //-------Ładowanie siatki---------
        void load_nodes(const std::string& filepath);
        void load_tr_elements(const std::string& filepath);
        void load_q_elements(const std::string& filepath);
        void load_bcs(const std::string& filepath);
        void reconstruct_edges();
        void load_mesh_from_txt(const std::string& filepath = "Data/fem_data.txt");
    };

    float len(geo::Node A, geo::Node B);
    float tr_size(geo::Triangle &tr, std::vector<geo::Node> nodes);
    int get_node_clicked(const std::vector<geo::Node> &nodes, float x_pos, float y_pos);
    float distSq(float x1, float y1, float x2, float y2);
    int get_edge_clicked(const std::vector<geo::Edge>& edges, const std::vector<geo::Node>& nodes, float x_pos, float y_pos);

}