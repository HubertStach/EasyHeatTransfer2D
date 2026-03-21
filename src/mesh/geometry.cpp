#include "geometry.h"

#include <algorithm>

#include "raylib.h"
#include <cmath>
#include <iostream>
#include "rlgl.h"
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

geo::Mesh::Mesh() {

}


void geo::Mesh::add_point(float x, float y, bool is_bc)
{
    for(Node n : this->nodes){
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

    size_t s = this->nodes.size();

    if (s == 2) {
        geo::Edge e(0, 1);
        e.bc_edge.is_bc = true;
        this->edges.push_back(e);
    }
    else if (s == 3) {
        geo::Edge e1(1, 2); // B->C
        e1.bc_edge.is_bc = true;
        this->edges.push_back(e1);

        geo::Edge e2(2, 0);
        e2.bc_edge.is_bc = true;
        this->edges.push_back(e2);
    }
    else if (s > 3) {
        this->edges.pop_back();

        geo::Edge e1(static_cast<int>(s) - 2, static_cast<int>(s) - 1);
        e1.bc_edge.is_bc = true;
        this->edges.push_back(e1);

        geo::Edge e2(static_cast<int>(s) - 1, 0);
        e2.bc_edge.is_bc = true;
        this->edges.push_back(e2);
    }
}

void geo::Mesh::pop_point()
{
    if(this->mesh_created){
        this->triangles.clear();
        this->nodes.clear();
        this->edges.clear();
        this->initial_bc_nodes.clear();
        mesh_created = false;
        return;
    }

    if(this->nodes.empty()) return;

    size_t s = this->nodes.size();

    this->nodes.pop_back();
    this->initial_bc_nodes.pop_back();

    if (s == 2) {
        this->edges.clear();
    }
    else if (s == 3) {
        this->edges.pop_back();
        this->edges.pop_back();
    }
    else if (s > 3) {
        this->edges.pop_back();
        this->edges.pop_back();

        geo::Edge closing(static_cast<int>(s) - 2, 0);
        closing.bc_edge.is_bc = true;
        this->edges.push_back(closing);
    }
}


//------------------- wyświetlanie -------------------

void geo::Mesh::draw_nodes(const float size) const {
    for(const Node &n:this->nodes){
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
    for (const auto& edge : this->edges) {
        if (edge.node_ids[0] < 0 || edge.node_ids[0] >= (int)nodes.size() || edge.node_ids[1] < 0 || edge.node_ids[1] >= (int)nodes.size())
        {
            continue;
        }

        const geo::Node& n1 = this->nodes[edge.node_ids[0]];
        const geo::Node& n2 = this->nodes[edge.node_ids[1]];

        Vector2 start_pos = { n1.x, n1.y };
        Vector2 end_pos   = { n2.x, n2.y };

        Color col = BLUE;
        if (edge.bc_edge.initialised) {
            col = RED;
        }

        DrawLineV(start_pos, end_pos, col);
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

//zastąpione przed draw_tr_grad
void geo::Mesh::draw_tr(std::vector<double> &temp, float max, float min) const {
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

void geo::Mesh::draw_tr_grad(std::vector<double> &temp, float max, float min) const {

    auto get_color_for_temp = [max, min](float t_val) -> Color {
        if (std::fabs(max - min) < 0.0001f) {
            return GREEN;
        }

        // Normalizacja do zakresu 0.0 - 1.0
        float t = (t_val - min) / (max - min);
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

        return Color{r, g, b, 255};
    };

    rlBegin(RL_TRIANGLES);

    for (const geo::Triangle& tr : this->triangles) {
        const geo::Node& node1 = this->nodes[tr.node_ids[0]];
        const geo::Node& node2 = this->nodes[tr.node_ids[1]];
        const geo::Node& node3 = this->nodes[tr.node_ids[2]];

        float t1 = temp[tr.node_ids[0]];
        float t2 = temp[tr.node_ids[1]];
        float t3 = temp[tr.node_ids[2]];

        Color col1 = get_color_for_temp(t1);
        Color col2 = get_color_for_temp(t2);
        Color col3 = get_color_for_temp(t3);

        // dwustronnemu z oryginalnego kodu - DrawTriangle(pos1, pos3, pos2))
        rlColor4ub(col1.r, col1.g, col1.b, col1.a);
        rlVertex2f(node1.x, node1.y);

        rlColor4ub(col3.r, col3.g, col3.b, col3.a);
        rlVertex2f(node3.x, node3.y);

        rlColor4ub(col2.r, col2.g, col2.b, col2.a);
        rlVertex2f(node2.x, node2.y);
    }

    // Kończymy rysowanie
    rlEnd();
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
    size_t parent_edge_count = this->edges.size();
    if(parent_edge_count == 0) return;

    std::vector<geo::Node> new_nodes;
    std::vector<geo::Edge> new_edges; // Tworzymy wektor krawędzi od razu

    int global_node_index = 0; // Licznik wszystkich nowych węzłów

    for (size_t i = 0; i < parent_edge_count; i++) {

        // Pobieramy oryginalną krawędź (rodzica)
        const geo::Edge& parent_edge = this->edges[i];

        geo::Node& A = this->nodes[parent_edge.node_ids[0]];
        geo::Node& B = this->nodes[parent_edge.node_ids[1]];

        float len = sqrt(pow(B.x-A.x,2) + pow(B.y-A.y,2));

        int N = (int)(len/spacing);
        if (N < 1) N = 1;

        auto x_u = [&](float u) { return A.x + (B.x - A.x) * u; };
        auto y_u = [&](float u) { return A.y + (B.y - A.y) * u; };

        float x_diff = (B.x-A.x)*(1.0f/static_cast<float>(N));
        float y_diff = (B.y-A.y)*(1.0f/static_cast<float>(N));
        float temp_bc_max_len = std::sqrt(std::pow(x_diff,2) + std::pow(y_diff,2));
        if (this->max_bc_len <= temp_bc_max_len) {
            this->max_bc_len = temp_bc_max_len;
        }

        // Generujemy N segmentów dla tej krawędzi
        for (int j = 0; j < N; j++) {
            float u = static_cast<float>(j) / static_cast<float>(N);

            // 1. Tworzenie węzła
            geo::Node temp_node(x_u(u), y_u(u));
            temp_node.bc.is_bc = true;

            // Przypisanie BC do węzła (opcjonalne, zależnie czy potrzebujesz w node czy edge)
            if (parent_edge.bc_edge.initialised) {
                temp_node.bc = parent_edge.bc_edge;
                temp_node.bc.initialised = true;
            }
            new_nodes.push_back(temp_node);

            // 2. Tworzenie krawędzi
            // Łączymy aktualnie tworzony węzeł z następnym.
            // (Tymczasowo ustawiamy id następnego jako index + 1,
            // naprawimy zapętlenie ostatniego elementu po pętli).
            geo::Edge temp_edge(global_node_index, global_node_index + 1);
            temp_edge.bc_edge.is_bc = true;

            // KLUCZOWA ZMIANA: Przepisanie warunku brzegowego z RODZICA na NOWĄ PODKRAWĘDŹ
            if (parent_edge.bc_edge.initialised) {
                temp_edge.bc_edge = parent_edge.bc_edge; // Kopiujemy wartości (flux, alfa, temp)
                temp_edge.bc_edge.initialised = true;
            }

            new_edges.push_back(temp_edge);

            global_node_index++;
        }
    }

    // Podmiana węzłów
    this->nodes = new_nodes;

    // Naprawa ostatniej krawędzi (zamknięcie pętli)
    // Ostatnia dodana krawędź wskazuje na index równy liczbie węzłów (out of bounds),
    // powinna wskazywać na 0.
    if (!new_edges.empty()) {
        new_edges.back().node_ids[1] = 0;
    }

    // Podmiana krawędzi
    this->edges = new_edges;
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

bool geo::Mesh::point_in_mesh(float x, float y) {
    bool inside = false;
    size_t count = this->initial_bc_nodes.size();

    for (size_t i = 0, j = count - 1; i < count; j = i++) {
        float xi = this->initial_bc_nodes[i].x;
        float yi = this->initial_bc_nodes[i].y;
        float xj = this->initial_bc_nodes[j].x;
        float yj = this->initial_bc_nodes[j].y;

        bool intersect = ((yi > y) != (yj > y)) && (x < (xj - xi) * (y - yi) / (yj - yi) + xi);

        if (intersect) {
            inside = !inside;
        }
    }

    return inside;
}

void geo::Mesh::cut_ext_elements() {
    float max_len = this->max_bc_len;

    auto it = this->triangles.begin();

    while (it != this->triangles.end()) {
        const auto& n1 = this->nodes[it->node_ids[0]];
        const auto& n2 = this->nodes[it->node_ids[1]];
        const auto& n3 = this->nodes[it->node_ids[2]];

        bool remove_triangle = false;

        if (n1.bc.is_bc && n2.bc.is_bc && n3.bc.is_bc) {

            float d1 = geo::len(n1, n2);
            float d2 = geo::len(n2, n3);
            float d3 = geo::len(n3, n1);

            if (d1 - max_len || d2 - max_len || d3 - max_len) {
                remove_triangle = true;
            }
        }

        if (remove_triangle) {
            it = this->triangles.erase(it);
        } else {
            ++it;
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

        this->edges = this->initial_edges;

        this->mesh_created = false;
    } else {
        this->initial_edges = this->edges;
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

    constexpr float divider = (3.0f / 1.73205f);
    
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

                if (point_in_mesh(x_p, y_p)) {
                    bool is_bc = false;
                    geo::Node center(x_p, y_p, false);
                    this->nodes.push_back(center);
                }
            }

        }
        
        this->triangulate();
        //std::cout<<mean_size<<'\n';
    }

    cut_ext_elements();
    this->mesh_created = true;
}


//dobieranie warunków brzegowych
int geo::get_node_clicked(const std::vector<geo::Node>& nodes, float x_pos, float y_pos) {
    constexpr float clickRadius = 0.1f;

    for (int i = 0; i < static_cast<int>(nodes.size()); i++) {
        if (std::abs(nodes[i].x - x_pos) < clickRadius && std::abs(nodes[i].y - y_pos) < clickRadius) {
            return i;
        }
    }

    return -1; // Nie kliknięto w żaden węzeł
}

float geo::distSq(float x1, float y1, float x2, float y2) {
    return (x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2);
}

int geo::get_edge_clicked(const std::vector<geo::Edge>& edges, const std::vector<geo::Node>& nodes, float x_pos, float y_pos) {
    constexpr float clickThreshold = 0.1f;
    constexpr float clickThresholdSq = clickThreshold * clickThreshold;

    for (int i = 0; i < static_cast<int>(edges.size()); i++) {
        const auto& n1 = nodes[edges[i].node_ids[0]];
        const auto& n2 = nodes[edges[i].node_ids[1]];

        const float x1 = n1.x; float y1 = n1.y;
        const float x2 = n2.x; float y2 = n2.y;

        float dx = x2 - x1;
        float dy = y2 - y1;

        if (dx == 0 && dy == 0) {
            if (distSq(x_pos, y_pos, x1, y1) <= clickThresholdSq) return i;
            continue;
        }

        float t = ((x_pos - x1) * dx + (y_pos - y1) * dy) / (dx * dx + dy * dy);

        t = std::max(0.0f, std::min(1.0f, t));

        float closestX = x1 + t * dx;
        float closestY = y1 + t * dy;

        float dist = distSq(x_pos, y_pos, closestX, closestY);

        if (dist <= clickThresholdSq) {
            return i;
        }
    }

    return -1;
}