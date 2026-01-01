#include "geometry.h"

#include <algorithm>

#include "raylib.h"
#include <cmath>
#include <iostream>

#include "imgui.h"

geo::Node::Node()
{
    this->x = 0.0;
    this->y = 0.0;
    this->bc.is_bc = true;
}

geo::Node::Node(float x, float y)
{
    this->x = x;
    this->y = y;
    this->bc.is_bc = true;
}

geo::Node::Node(const float x, const float y, const bool is_bc) {
    this->x = x;
    this->y = y;
    this->bc.is_bc = is_bc;

    if (!is_bc) {
        this->color = GREEN;
    }
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
    temp_node.bc.is_bc = is_bc;
    if (!is_bc) {
        temp_node.color = GREEN;
    }
    this->nodes.push_back(temp_node);
    this->initial_bc_nodes.push_back(temp_node);
}

void geo::Mesh::pop_point()
{
    if(this->mesh_created){
        this->triangles.clear();
        this->nodes.clear();
        this->edges.clear();
        this->initial_bc_nodes.clear();
        mesh_created = false;
    }

    if(!this->nodes.empty()){
        this->nodes.pop_back();
        this->initial_bc_nodes.pop_back();
    }

    /* Problem: po usunięciu jednego punktu
        wszystkie inne punkty się usuwają
        ale draw_edges i tak rysuje zapisane w pamięci
        punkty brzegowe

        chwilowa naprawa: pop_point będzie czyścić wszystko
     */
}

//------------------- wyświetlanie -------------------
void geo::Mesh::draw_nodes(float size)
{
    for(Node &n:this->nodes){
        Color color=n.color;
        Vector2 pos;
        pos.x = n.x;
        pos.y = n.y;

        if (n.bc.initialised) {
            color = ORANGE;
        }

        DrawCircleV(pos, size, color);
    }
}

void geo::Mesh::draw_edges()
{
    const size_t init_bc_size = this->initial_bc_nodes.size();
    if(init_bc_size==1 ||init_bc_size==0){return;} //nie da się narysować linii
    
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

        const geo::Node& node1 = this->nodes[tr.node_ids[0]];
        const geo::Node& node2 = this->nodes[tr.node_ids[1]];
        const geo::Node& node3 = this->nodes[tr.node_ids[2]];

        const Vector2 pos1 = { node1.x, node1.y };
        const Vector2 pos2 = { node2.x, node2.y };
        const Vector2 pos3 = { node3.x, node3.y };

        DrawLineV(pos1, pos2, WHITE);
        DrawLineV(pos2, pos3, WHITE);
        DrawLineV(pos3, pos1, WHITE);
    }
}

void geo::Mesh::draw_tr(std::vector<double> temp, float max, float min)
{
    for (const geo::Triangle& tr : this->triangles) {
        const geo::Node& node1 = this->nodes[tr.node_ids[0]];
        const geo::Node& node2 = this->nodes[tr.node_ids[1]];
        const geo::Node& node3 = this->nodes[tr.node_ids[2]];

        float t1 = temp[tr.node_ids[0]];
        float t2 = temp[tr.node_ids[1]];
        float t3 = temp[tr.node_ids[2]];

        float t_mean = (t1 + t2 + t3)/3.0f;

        Color col;

        if (fabs(max - min) < 0.0001f) {
            col = GREEN;
        }

        // Normalizacja do zakresu 0.0 - 1.0
        float t = (t_mean - min) / (max - min);
        t = std::clamp(t, 0.0f, 1.0f);

        unsigned char r = 0, g = 0, b = 0;

        if (t < 0.5f) {
            // Przejście Niebieski -> Biały
            float local_t = t * 2.0f; // skalujemy 0..0.5 na 0..1
            r = (unsigned char)(255 * local_t);
            g = (unsigned char)(255 * local_t);
            b = 255;
        } else {
            // Przejście Biały -> Czerwony
            float local_t = (t - 0.5f) * 2.0f; // skalujemy 0.5..1 na 0..1
            r = 255;
            g = (unsigned char)(255 * (1.0f - local_t));
            b = (unsigned char)(255 * (1.0f - local_t));
        }
        col = Color{r,g,b, 255};

        const Vector2 pos1 = { node1.x, node1.y };
        const Vector2 pos2 = { node2.x, node2.y };
        const Vector2 pos3 = { node3.x, node3.y };

        DrawTriangle(pos1, pos2, pos3, col);
        DrawTriangle(pos1, pos3, pos2, col);
    }
}


//-------------------triangulacja siatki---------------

//źródło interpolacji: The Finite Element Method: Its Basis and Fundamentals Seventh Edition, 17.3.3.1 Boundary node generation
//uproszczona wersja bez siatki tła

float geo::len(geo::Node A, geo::Node B){
    return sqrt(pow(B.x-A.x,2)+pow(B.y-A.y,2));
}

float geo::tr_size(geo::Triangle &tr, std::vector<geo::Node> nodes){
    const geo::Node A = nodes[tr.node_ids[0]];
    const geo::Node B = nodes[tr.node_ids[1]];
    const geo::Node C = nodes[tr.node_ids[2]];

    const float a = len(A, B);
    const float b = len(B, C);
    const float c = len(C, A);

    return 0.25f*(std::sqrt((a+b+c)*(-a+b+c)*(a-b+c)*(a+b-c)));
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
            temp.bc.is_bc = true;

            if (A.bc.initialised) {
                temp.bc.flux = A.bc.flux;
                temp.bc.alfa = A.bc.alfa;
                temp.bc.t_ext = A.bc.t_ext;
                temp.bc.initialised = true;
            }

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
        
    for (const int node_id : A.node_ids) {
        for (const int j : B.node_ids) {
            if ((node_id == j) && (node_id == j)) {
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
    triangulation.emplace_back(0, 1, 2);

 
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
            triangulation.emplace_back(edge.node_ids[0], edge.node_ids[1], node_id);
        }
    }

    this->nodes = original_nodes;

    this->triangles.clear();
    for (const auto& tr : triangulation) {
        if (tr.node_ids[0] >= 3 && tr.node_ids[1] >= 3 && tr.node_ids[2] >= 3) {
            this->triangles.emplace_back(tr.node_ids[0] - 3, tr.node_ids[1] - 3, tr.node_ids[2] - 3);
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
                bool is_bc = false;

                geo::Node center(x_p, y_p, false);
                this->nodes.push_back(center);
            }

        }
        
        this->triangulate();
        //std::cout<<mean_size<<'\n';
    }

    this->mesh_created = true;
}


//dobieranie warunków brzegowych
int geo::get_node_clicked(const std::vector<geo::Node>& nodes, float x_pos, float y_pos) {
    const float clickRadius = 0.1f;


    for (int i = 0; i < (int) nodes.size(); i++) {
        if (std::abs(nodes[i].x - x_pos) < clickRadius && std::abs(nodes[i].y - y_pos) < clickRadius) {
            return i;
        }
    }

    return -1; // Nie kliknięto w żaden węzeł
}