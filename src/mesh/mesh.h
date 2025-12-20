/*
#pragma once
#include <vector>

#include "../mes/mes.h"
#include "geometry.h"

void add_new_node(Vector2 worldPos, go::Vertex &polygon);
void pop_node(go::Vertex &polygon);

namespace msh{
    bool is_node_same(const go::Node &n1, const go::Node &n2);
    void write_node_ids(std::vector<go::Node> &nodes);
    void remove_duplicate_nodes(std::vector<go::Node> &nodes);
    std::vector<go::Node> add_boundary_nodes_on_edge(const go::Segment &seg, int N);
    std::vector<go::Node> add_boundary_nodes_on_vertex(const go::Vertex shape, float spacing);
    go::Triangle super_trian(const std::vector<go::Node> &node_list);
    bool inside_circumcircle(const go::Triangle &triangle, const go::Node &point);
    bool same_triangle(const go::Triangle tr1, const go::Triangle tr2);
    inline bool same_edge(const go::Segment& e1, const go::Segment& e2);
    bool is_boundary_edge(const go::Segment& edge, const std::vector<go::Triangle>& bad_triangles);
    void filter_triangles(std::vector<go::Triangle> &triangles, go::Vertex &polygon);

    std::vector<go::Triangle> bowyer_watson(std::vector<go::Node>& node_list);
    
    //std::vector<go::Vertex> triangles_to_quads(std::vector<go::Triangle> &triangles, std::vector<go::Node> &nodes);
    void create_mesh(go::Vertex polygon, float spacing, std::vector<go::Triangle> &triangles, std::vector<go::Node> &nodes);

    bool is_node_on_segment(const go::Segment& edge, const go::Node &node);
    std::vector<go::Node> create_bcs(const std::vector<go::Node> &all_nodes, const go::Vertex polygon);
}


namespace to_fem{
    struct Tri_ref{
        int node_ids[3];

        Tri_ref(){
            node_ids[0] = 0.0f;
            node_ids[1] = 0.0f;
            node_ids[2] = 0.0f;
        }

        Tri_ref(int id1, int id2, int id3){
            node_ids[0] = id1;
            node_ids[1] = id2;
            node_ids[2] = id3;
        }
    };

    std::vector<Tri_ref> write_to_FEM(const std::vector<go::Node> &nodes, const std::vector<go::Vertex> &tri_mesh);
    void print_mesh(const std::vector<go::Node> &nodes, std::vector<Tri_ref> tri_refs);
    void write_to_FEM(const std::vector<go::Node> &nodes, const std::vector<go::Node> &bc_nodes, const Fem::GlobalData &conf, const std::vector<go::Triangle> mesh);
}
    */