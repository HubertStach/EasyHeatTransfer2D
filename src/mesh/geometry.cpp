#include "geometry.h"
#include "raylib.h"
#include <cmath>
#include <iostream>

geo::Node::Node()
{
    this->x = 0.0;
    this->y = 0.0;
    this->is_bc = true;
}

geo::Node::Node(float x, float y)
{
    this->x = x;
    this->y = y;
    this->is_bc = true;
}

geo::Edge::Edge()
{
    this->node_ids[0] = -1;
    this->node_ids[1] = -1;
}

geo::Edge::Edge(int n1, int n2)
{
    this->node_ids[0] = n1;
    this->node_ids[1] = n2;
}

geo::Polygon::Polygon()
{
    std::vector<int> node_ids_temp(1);
    this->node_ids = node_ids_temp;
}

geo::Polygon::Polygon(std::vector<int> node_ids)
{
    this->node_ids = node_ids;
}

geo::Triangle::Triangle()
{
    this->node_ids[0] = -1;
    this->node_ids[1] = -1;
    this->node_ids[2] = -1;
}

geo::Triangle::Triangle(int n1, int n2, int n3)
{
    this->node_ids[0] = n1;
    this->node_ids[1] = n2;
    this->node_ids[2] = n3;
}

geo::Mesh::Mesh()
{

}

void geo::Mesh::add_point(float x, float y, bool is_bc)
{
    for(Node n:this->nodes){
        if(n.x == x && n.y == y){
            return;
        }
    }

    Node temp_node(x, y);
    temp_node.is_bc = is_bc;
    this->nodes.push_back(temp_node);
    this->initial_bc_nodes.push_back(temp_node);
}

void geo::Mesh::pop_point()
{
    if(this->mesh_created){
        this->triangles.clear();
        this->nodes.clear();
        this->edges.clear();
        mesh_created = false;
    }

    if(this->nodes.size()>0){
        this->nodes.pop_back();
        this->initial_bc_nodes.pop_back();
    }
}

//------------------- wyświetlanie -------------------
void geo::Mesh::draw_nodes(float size)
{
    Color color;

    for(Node &n:this->nodes){
        if(n.is_bc){
            color = RED;
        }
        else{
            color = GREEN;
        }

        Vector2 pos;
        pos.x = n.x;
        pos.y = n.y;
        DrawCircleV(pos, size, color);
    }
}

void geo::Mesh::draw_edges()
{   
    size_t init_bc_size = this->initial_bc_nodes.size();
    if(init_bc_size==1 ||init_bc_size==0){return;}//nie da się narysować linii

    if(init_bc_size < 1){ 
        return;
    }
    
    if(!this->mesh_created){
        for(size_t i =0; i< init_bc_size; i++){
            Vector2 start_pos;
            Vector2 end_pos;

            start_pos.x = this->initial_bc_nodes[i].x;
            start_pos.y = this->initial_bc_nodes[i].y;
            end_pos.x = this->initial_bc_nodes[(i+1)%init_bc_size].x;
            end_pos.y = this->initial_bc_nodes[(i+1)%init_bc_size].y;

            DrawLineV(start_pos, end_pos, BLUE);
        }
    }
}
void geo::Mesh::draw_tr()
{
    for (const geo::Triangle& tr : this->triangles) {
        
        geo::Node& node1 = this->nodes[tr.node_ids[0]];
        geo::Node& node2 = this->nodes[tr.node_ids[1]];
        geo::Node& node3 = this->nodes[tr.node_ids[2]];

        Vector2 pos1 = { node1.x, node1.y };
        Vector2 pos2 = { node2.x, node2.y };
        Vector2 pos3 = { node3.x, node3.y };

        DrawLineV(pos1, pos2, WHITE);
        DrawLineV(pos2, pos3, WHITE);
        DrawLineV(pos3, pos1, WHITE);
    }
}


//-------------------triangulacja siatki---------------

//źródło interpolacji: The Finite Element Method: Its Basis and Fundamentals Seventh Edition, 17.3.3.1 Boundary node generation
//uproszczona wersja bez siatki tła

float geo::len(geo::Node A, geo::Node B){
    return sqrt(pow(B.x-A.x,2)+pow(B.y-A.y,2));
}

float geo::tr_size(geo::Triangle &tr, std::vector<geo::Node> nodes){
    geo::Node A = nodes[tr.node_ids[0]];
    geo::Node B = nodes[tr.node_ids[1]];
    geo::Node C = nodes[tr.node_ids[2]];

    float a = len(A, B);
    float b = len(B, C);
    float c = len(C, A);

    return 0.25*(std::sqrt((a+b+c)*(-a+b+c)*(a-b+c)*(a+b-c)));
}

void geo::Mesh::interpolate_bc_points(float spacing)
{
    size_t node_count = nodes.size();

    if(node_count == 0 || node_count == 1){return;}

    std::vector<geo::Node> new_nodes;

    for (size_t i = 0; i < node_count; i++) {
        
        //pętla po dwóch kolejnych punktach tworzących bok
        geo::Node& A = this->nodes[i];
        geo::Node& B = this->nodes[(i + 1) % node_count];
        
        //obliczanie ile punktów przydzielić na bok
        float len = sqrt(pow(B.x-A.x,2) + pow(B.y-A.y,2));

        int N = (int)len/spacing;
        //std::cout<<N<<"\n";
        
        auto x_u = [&](float u) { return A.x + (B.x - A.x) * u; };
        auto y_u = [&](float u) { return A.y + (B.y - A.y) * u; };
        
        for (int j = 0; j < N; j++) {
            float u = static_cast<float>(j) / static_cast<float>(N);
            
            geo::Node temp(x_u(u), y_u(u));
            temp.is_bc = true;
            new_nodes.push_back(temp);
        }
    }

    this->nodes = new_nodes;

    std::vector<geo::Edge> edges;
    for(size_t i =0; i<this->nodes.size();i++){
        geo::Edge temp(i, (i+1)%3);
        edges.push_back(temp);
    }
    this->edges = edges;
}


std::vector<geo::Node> geo::Mesh::super_triangle()
{
    float max_x, max_y, min_x, min_y;

    max_x = nodes[0].x;
    max_y = nodes[0].y;
    min_x = nodes[0].x;
    min_y = nodes[0].y;

    for(auto&it:nodes){
        if(it.x > max_x){
            max_x = it.x;
        }

        if(it.x < min_x){
            min_x = it.x;
        }

        if(it.y > max_y){
            max_y = it.y;
        }

        if(it.y < min_y){
            min_y = it.y;
        }
    }

    float w = max_x - min_x;
    float h = max_y - min_y;

    Node p1(min_x - w*0.1f, min_y-h);
    Node p2(min_x+w*1.7f, min_y+h*0.5f);
    Node p3(min_x-w*0.1f, min_y+h*2.0f);
    
    std::vector<geo::Node> result = {p1, p2, p3};
    return result;
}

bool geo::Mesh::inside_circumcircle(geo::Triangle tr, int node_id)
{
    geo::Node A = this->nodes[tr.node_ids[0]];
    geo::Node B = this->nodes[tr.node_ids[1]];
    geo::Node C = this->nodes[tr.node_ids[2]];
    geo::Node D = this->nodes[node_id];

    double adx = A.x - D.x;
    double ady = A.y - D.y;
    double bdx = B.x - D.x;
    double bdy = B.y - D.y;
    double cdx = C.x - D.x;
    double cdy = C.y - D.y;
        
    double bcdet = bdx * cdy - bdy * cdx;
    double cadet = cdx * ady - cdy * adx;
    double abdet = adx * bdy - ady * bdx;
        
    double alift = adx * adx + ady * ady;
    double blift = bdx * bdx + bdy * bdy;
    double clift = cdx * cdx + cdy * cdy;
        
    double det = alift * bcdet + blift * cadet + clift * abdet;
        
    double area_ABC = (B.x - A.x) * (C.y - A.y) - (C.x - A.x) * (B.y - A.y);
        
    if (area_ABC == 0.0) {
        return false;
    }
        
    return (det * area_ABC > 0);
}

bool geo::Mesh::same_triangle(geo::Triangle A, geo::Triangle B )
{

    int matches = 0;
        
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if ((A.node_ids[i] == B.node_ids[j]) && (A.node_ids[i] == B.node_ids[j])) {
                matches++;
                break;
            }
        }
    }
        
    return matches == 3;
}

bool geo::Mesh::is_boundary_edge(std::vector<geo::Triangle> &triangles, geo::Edge edge)
{
    int count = 0;

    for (const auto& triangle : triangles) {
        for (int i=0; i<3; i++) {
            if ((triangle.node_ids[i] == edge.node_ids[0] && triangle.node_ids[(i+1)%3] == edge.node_ids[1]) ||
                (triangle.node_ids[(i+1)%3] == edge.node_ids[0] && triangle.node_ids[i] == edge.node_ids[1]))
            {
                count++;
                if (count > 1) return false;
            }
        }
    }
    return count == 1;
}

//algorytm bowyera-watsona
void geo::Mesh::triangulate()
{
    size_t original_nodes_count = this->nodes.size();
    if (original_nodes_count < 3) {
        return;
    }

    std::vector<geo::Node> original_nodes = this->nodes;

    std::vector<geo::Node> super_triangle_nodes = this->super_triangle();
    this->nodes.insert(this->nodes.begin(), super_triangle_nodes.begin(), super_triangle_nodes.end());

    std::vector<geo::Triangle> triangulation;
    triangulation.push_back(geo::Triangle(0, 1, 2));

 
    for (size_t i = 0; i < original_nodes_count; ++i) {
        size_t node_id = i + 3;

        std::vector<geo::Triangle> bad_triangles;
        std::vector<geo::Edge> polygon_edges;

        for (const geo::Triangle& tr : triangulation) {
            if (inside_circumcircle(tr, node_id)) {
                bad_triangles.push_back(tr);
            }
        }

        for (const auto& bad_tr : bad_triangles) {
            int n[3] = {bad_tr.node_ids[0], bad_tr.node_ids[1], bad_tr.node_ids[2]};
            for (int j = 0; j < 3; ++j) {
                geo::Edge edge(n[j], n[(j + 1) % 3]);
                
                bool is_shared = false;
                for (const auto& other_bad_tr : bad_triangles) {
                    if (&bad_tr == &other_bad_tr) continue; 
                    
                    int other_n[3] = {other_bad_tr.node_ids[0], other_bad_tr.node_ids[1], other_bad_tr.node_ids[2]};

                    for (int k = 0; k < 3; ++k) {
                        if ((edge.node_ids[0] == other_n[k] && edge.node_ids[1] == other_n[(k + 1) % 3]) ||
                            (edge.node_ids[0] == other_n[(k + 1) % 3] && edge.node_ids[1] == other_n[k])) {
                            is_shared = true;
                            break;
                        }
                    }
                    if (is_shared) break;
                }

                if (!is_shared) {
                    polygon_edges.push_back(edge);
                }
            }
        }

        std::vector<geo::Triangle> temp_triangulation;
        for (const auto& tr : triangulation) {
            bool is_bad = false;
            for (const auto& bad_tr : bad_triangles) {
                if (same_triangle(tr, bad_tr)) {
                    is_bad = true;
                    break;
                }
            }
            if (!is_bad) {
                temp_triangulation.push_back(tr);
            }
        }
        triangulation = temp_triangulation;

        for (const auto& edge : polygon_edges) {
            triangulation.push_back(geo::Triangle(edge.node_ids[0], edge.node_ids[1], node_id));
        }
    }

    this->nodes = original_nodes;

    this->triangles.clear();
    for (const auto& tr : triangulation) {
        if (tr.node_ids[0] >= 3 && tr.node_ids[1] >= 3 && tr.node_ids[2] >= 3) {
            this->triangles.push_back(geo::Triangle(tr.node_ids[0] - 3, tr.node_ids[1] - 3, tr.node_ids[2] - 3));
        }
    }
}

void geo::Mesh::create_mesh(float spacing)
{
    if(this->mesh_created){
        this->nodes.clear();
        this->triangles.clear();
        this->edges.clear();
        this->nodes = this->initial_bc_nodes;
        this->mesh_created = false;
    }

    //interpolujemy punkty
    this->interpolate_bc_points(spacing);

    //początkowa triangulacja
    this->triangulate();

    //liczymy średnią wielkość trójkątów
    float mean_size=0.0f;
    float sum_size=0.0f;

    for(geo::Triangle tr:triangles){
        sum_size += tr_size(tr, this->nodes);
    }
    mean_size = sum_size/static_cast<int>(triangles.size());

    constexpr float divider = (4.0f / 1.73205f);
    
    int max_iter = 10;
    int current_iter = 0;
    while((std::sqrt(divider*mean_size) > spacing) && (current_iter < max_iter)){
        current_iter++;
        //std::cout<<"sqrt("<<divider<<"*"<<mean_size<<") = "<<std::sqrt(divider*mean_size)<<" <= " << spacing<<"\n";

        sum_size=0.0f;
        //liczymy srednia wielkosc trojkata
        for(geo::Triangle tr:triangles){
            sum_size += tr_size(tr, this->nodes);
        }
        mean_size = sum_size/static_cast<int>(triangles.size());

        
        //dodajemy nowe punkty
        for(geo::Triangle &tr:this->triangles){

            if(tr_size(tr, this->nodes) > mean_size){
                float x_p = (this->nodes[tr.node_ids[0]].x +this->nodes[tr.node_ids[1]].x+this->nodes[tr.node_ids[2]].x) /3;
                float y_p = (this->nodes[tr.node_ids[0]].y +this->nodes[tr.node_ids[1]].y+this->nodes[tr.node_ids[2]].y) /3;

                geo::Node center(x_p, y_p);
                center.is_bc = false;

                this->nodes.push_back(center);
            }

        }
        
        this->triangulate();
        //std::cout<<mean_size<<'\n';
    }

    this->mesh_created = true;
}
