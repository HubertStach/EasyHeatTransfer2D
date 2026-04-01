#include "geometry.h"

#include <algorithm>

#include "raylib.h"
#include <cmath>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>

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

geo::Quad::Quad() = default;

geo::Quad::Quad(int n1, int n2, int n3, int n4) {
    this->node_ids[0] = n1;
    this->node_ids[1] = n2;
    this->node_ids[2] = n3;
    this->node_ids[3] = n4;
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

void geo::Mesh::draw_q() {
    for (const geo::Quad& q : this->quads) {

        const geo::Node& node1 = this->nodes[q.node_ids[0]];
        const geo::Node& node2 = this->nodes[q.node_ids[1]];
        const geo::Node& node3 = this->nodes[q.node_ids[2]];
        const geo::Node& node4 = this->nodes[q.node_ids[3]];

        const Vector2 pos1 = { node1.x, node1.y };
        const Vector2 pos2 = { node2.x, node2.y };
        const Vector2 pos3 = { node3.x, node3.y };
        const Vector2 pos4 = { node4.x, node4.y };

        DrawLineV(pos1, pos2, WHITE);
        DrawLineV(pos2, pos3, WHITE);
        DrawLineV(pos3, pos4, WHITE);
        DrawLineV(pos4, pos1, WHITE);
    }
}

void geo::Mesh::draw_q_grad(std::vector<double> &temp, float max, float min) const {
    auto get_color_for_temp = [max, min](float t_val) -> Color {
        if (std::fabs(max - min) < 0.0001f) {
            return GREEN;
        }

        // Normalizacja do zakresu 0.0 - 1.0
        float t = (t_val - min) / (max - min);
        t = std::clamp(t, 0.0f, 1.0f);

        unsigned char r = 0, g = 0, b = 0;

        if (t < 0.5f) {
            float local_t = t * 2.0f;
            r = (unsigned char)(255 * local_t);
            g = (unsigned char)(255 * local_t);
            b = 255;
        } else {
            float local_t = (t - 0.5f) * 2.0f;
            r = 255;
            g = (unsigned char)(255 * (1.0f - local_t));
            b = (unsigned char)(255 * (1.0f - local_t));
        }

        return Color{r, g, b, 255};
    };

    // Używamy bezpiecznych trójkątów zamiast QUADS
    rlBegin(RL_TRIANGLES);

    for (const geo::Quad& q : this->quads) {
        const geo::Node& node1 = this->nodes[q.node_ids[0]];
        const geo::Node& node2 = this->nodes[q.node_ids[1]];
        const geo::Node& node3 = this->nodes[q.node_ids[2]];
        const geo::Node& node4 = this->nodes[q.node_ids[3]];

        float t1 = temp[q.node_ids[0]];
        float t2 = temp[q.node_ids[1]];
        float t3 = temp[q.node_ids[2]];
        float t4 = temp[q.node_ids[3]];

        // 1. Obliczamy pozycję X, Y oraz Temperaturę dla ŚRODKA czworokąta
        float cx = (node1.x + node2.x + node3.x + node4.x) / 4.0f;
        float cy = (node1.y + node2.y + node3.y + node4.y) / 4.0f;
        float tc = (t1 + t2 + t3 + t4) / 4.0f;

        // 2. Pobieramy kolory wierzchołków
        Color col1 = get_color_for_temp(t1);
        Color col2 = get_color_for_temp(t2);
        Color col3 = get_color_for_temp(t3);
        Color col4 = get_color_for_temp(t4);
        Color colC = get_color_for_temp(tc); // Kolor centralny

        // Wyciągamy rysowanie fragmentu do prostej lambdy,
        // rysującej trójkąt w 2 kierunkach, aby zabezpieczyć go przed cullingiem (ukryciem przez silnik)
        auto draw_triangle_both_ways = [&](const geo::Node& nA, Color cA, const geo::Node& nB, Color cB) {
            // Rysowanie z jedną rotacją
            rlColor4ub(cA.r, cA.g, cA.b, cA.a); rlVertex2f(nA.x, nA.y);
            rlColor4ub(cB.r, cB.g, cB.b, cB.a); rlVertex2f(nB.x, nB.y);
            rlColor4ub(colC.r, colC.g, colC.b, colC.a); rlVertex2f(cx, cy);

            // Rysowanie z odwrotną rotacją (gwarantuje widoczność 2D)
            rlColor4ub(cA.r, cA.g, cA.b, cA.a); rlVertex2f(nA.x, nA.y);
            rlColor4ub(colC.r, colC.g, colC.b, colC.a); rlVertex2f(cx, cy);
            rlColor4ub(cB.r, cB.g, cB.b, cB.a); rlVertex2f(nB.x, nB.y);
        };

        // 3. Rysujemy 4 "wewnętrzne" trójkąty opierające się o wierzchołek centralny
        draw_triangle_both_ways(node1, col1, node2, col2); // Górny (zależnie od ułożenia)
        draw_triangle_both_ways(node2, col2, node3, col3); // Prawy
        draw_triangle_both_ways(node3, col3, node4, col4); // Dolny
        draw_triangle_both_ways(node4, col4, node1, col1); // Lewy
    }

    // Kończymy rysowanie
    rlEnd();
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

        // --- RYSOWANIE TRÓJKĄTA (Zgodnie z kierunkiem wskazówek zegara / przeciw) ---
        // Należy zdefiniować kolor ZANIM zdefiniuje się wierzchołek

        // Trójkąt pierwszy

        rlColor4ub(col1.r, col1.g, col1.b, col1.a);
        rlVertex2f(node1.x, node1.y);

        rlColor4ub(col2.r, col2.g, col2.b, col2.a);
        rlVertex2f(node2.x, node2.y);

        rlColor4ub(col3.r, col3.g, col3.b, col3.a);
        rlVertex2f(node3.x, node3.y);

        // Trójkąt drugi (Odwrotna kolejność wierzchołków, odpowiada to rysowaniu
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


//---------------Ładowanie siatki------------------

void geo::Mesh::load_nodes(const std::string &filepath) {
    std::vector<Node> loaded_nodes;
    std::fstream file;

    file.open(filepath);
    if(!file.good()){
        std::cout << "Couldn't load text file\n";
        return ;
    }

    std::string line;
    bool node_selection = false;

    while (std::getline(file, line)) {
        if (line == "*Nodes") {
            node_selection = true;
            continue;
        }

        if (line == "*Triangles" || line == "*BC" || line == "*Quads") {
            node_selection = false;
        }

        if (node_selection) {
            int id;
            float x, y;
            char comma;
            std::istringstream iss(line);
            if (iss >> id >> comma >> x >> comma >> y) {
                loaded_nodes.emplace_back(x*100, y*100, false);
            }
        }
    }
    file.close();
    this->nodes = loaded_nodes;
}

void geo::Mesh::load_tr_elements(const std::string &filepath) {
    std::vector<Triangle> tr_elements;
    std::fstream file;

    file.open(filepath);
    if(!file.good()){
        std::cout << "Couldn't load text file\n";
        return ;
    }

    std::string line;
    bool tr_selection = false;

    while (std::getline(file, line)) {
        if (line == "*Triangles") {
            tr_selection = true;
            continue;
        }

        if (line == "*Nodes" || line == "*BC" || line == "*Quads") {
            tr_selection = false;
        }

        if (tr_selection) {
            int id;
            int node_id1, node_id2, node_id3;
            char comma;
            std::istringstream iss(line);
            if (iss >> id >> comma >> node_id1 >> comma >> node_id2 >> comma >> node_id3) {
                tr_elements.emplace_back(node_id1, node_id2, node_id3);
            }
        }
    }
    file.close();
    this->triangles = tr_elements;
}

void geo::Mesh::load_q_elements(const std::string &filepath) {
    std::vector<Quad> q_elements;
    std::fstream file;

    file.open(filepath);
    if(!file.good()){
        std::cout << "Couldn't load text file\n";
        return ;
    }

    std::string line;
    bool tr_selection = false;

    while (std::getline(file, line)) {
        if (line == "*Quads") {
            tr_selection = true;
            continue;
        }

        if (line == "*Nodes" || line == "*BC" || line=="*Triangles") {
            tr_selection = false;
        }

        if (tr_selection) {
            int id;
            int node_id1, node_id2, node_id3, node_id4;
            char comma;
            std::istringstream iss(line);
            if (iss >> id >> comma >> node_id1 >> comma >> node_id2 >> comma >> node_id3 >> comma >>node_id4) {
                q_elements.emplace_back(node_id1, node_id2, node_id3, node_id4);
            }
        }
    }
    file.close();
    this->quads = q_elements;
}

void geo::Mesh::load_bcs(const std::string &filepath) {
    std::fstream file;

    file.open(filepath);
    if(!file.good()){
        std::cout << "Couldn't load text file\n";
        return ;
    }

    std::string line;
    bool bc_selection = false;

    while (std::getline(file, line)) {
        if (line == "*BC") {
            bc_selection = true;
            continue;
        }

        if (line == "*Nodes" || line == "*Triangles" || line == "*Quads") {
            bc_selection = false;
        }

        if (bc_selection) {
            int node_id;
            float flux, alfa, t_ext;
            char comma;
            std::istringstream iss(line);
            if (iss >> node_id >> comma >> flux >> comma >> alfa >> comma >> t_ext) {
                this->nodes[node_id].bc.flux = flux;
                this->nodes[node_id].bc.alfa = alfa;
                this->nodes[node_id].bc.t_ext = t_ext;
                this->nodes[node_id].bc.is_bc = true;
                this->nodes[node_id].bc.initialised = true;

            }
            else {
                std::cout << "Failed to parse line: " << line << "\n"; // Debug output
            }
        }
    }

    file.close();
}

//
void geo::Mesh::reconstruct_edges() {
    this->edges.clear();

    std::map<std::pair<int, int>, int> edge_count;

    for (const geo::Triangle& tr : this->triangles) {
        for (int i = 0; i < 3; ++i) {
            int n1 = tr.node_ids[i];
            int n2 = tr.node_ids[(i + 1) % 3];

            int min_n = std::min(n1, n2);
            int max_n = std::max(n1, n2);

            edge_count[{min_n, max_n}]++;
        }
    }

    for (const geo::Quad& q : this->quads) {
        for (int i = 0; i < 4; ++i) {
            int n1 = q.node_ids[i];
            int n2 = q.node_ids[(i + 1) % 4];

            int min_n = std::min(n1, n2);
            int max_n = std::max(n1, n2);

            edge_count[{min_n, max_n}]++;
        }
    }

    for (const auto& pair : edge_count) {
        if (pair.second == 1) {
            int n1 = pair.first.first;
            int n2 = pair.first.second;

            geo::Edge new_edge(n1, n2);

            if (this->nodes[n1].bc.is_bc && this->nodes[n2].bc.is_bc) {
                new_edge.bc_edge.is_bc = true;

                if (this->nodes[n1].bc.initialised && this->nodes[n2].bc.initialised) {
                    new_edge.bc_edge.initialised = true;
                    new_edge.bc_edge.flux = this->nodes[n1].bc.flux;
                    new_edge.bc_edge.alfa = this->nodes[n1].bc.alfa;
                    new_edge.bc_edge.t_ext = this->nodes[n1].bc.t_ext;
                }
            }

            this->edges.push_back(new_edge);
        }
    }

    this->initial_edges = this->edges;
}


void geo::Mesh::load_mesh_from_txt(const std::string& filepath) {
    this->load_nodes(filepath);
    this->load_tr_elements(filepath);
    this->load_q_elements(filepath);
    this->load_bcs(filepath);
    this->reconstruct_edges();
    this->mesh_created = true;
}


//-------------------triangulacja siatki---------------

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
    std::vector<geo::Edge> new_edges;

    int global_node_index = 0;

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

            geo::Edge temp_edge(global_node_index, global_node_index + 1);
            temp_edge.bc_edge.is_bc = true;

            if (parent_edge.bc_edge.initialised) {
                temp_edge.bc_edge = parent_edge.bc_edge;
                temp_edge.bc_edge.initialised = true;
            }

            new_edges.push_back(temp_edge);

            global_node_index++;
        }
    }

    // Podmiana węzłów
    this->nodes = new_nodes;

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

void geo::Mesh::circumcenter(const geo::Triangle &tr, geo::Node &center) {
    geo::Node A = this->nodes[tr.node_ids[0]];
    geo::Node B = this->nodes[tr.node_ids[1]];
    geo::Node C = this->nodes[tr.node_ids[2]];

    const double D = 2*(A.x*(B.y-C.y)+B.x*(C.y-A.y)+C.x*(A.y-B.y));

    if (std::abs(D) < 1e-10f) {
        return;
    }

    const double X = (1/D)*((pow(A.x,2)+pow(A.y,2))*(B.y-C.y) + (pow(B.x,2)+pow(B.y,2))*(C.y-A.y)+
        (pow(C.x,2)+pow(C.y,2))*(A.y-B.y));

    const double Y = (1/D)*((pow(A.x,2)+pow(A.y,2))*(C.x-B.x) + (pow(B.x,2)+pow(B.y,2))*(A.x-C.x)+
        (pow(C.x,2)+pow(C.y,2))*(B.x-A.x));

    geo::Node temp(X, Y, false);

    center = temp;
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


void geo::Mesh::create_nodes_SH_LO(float spacing) {
    // -------------------------------------------------------
    // generowanie punktów (wg S.H. Lo, 1985)

    if (!this->nodes.empty()) {
        float y_min = this->nodes[0].y;
        float y_max = this->nodes[0].y;
        for (const auto& n : this->nodes) {
            if (n.y < y_min) y_min = n.y;
            if (n.y > y_max) y_max = n.y;
        }
        float D = spacing;
        float h_spacing = D * 0.866025f;
        float min_dist_sq = (0.7f * D) * (0.7f * D);

        bool stagger = false;

        for (float H = y_min + h_spacing; H < y_max; H += h_spacing) {
            std::vector<float> intersections;

            for (const auto& edge : this->edges) {
                const Node& p1 = this->nodes[edge.node_ids[0]];
                const Node& p2 = this->nodes[edge.node_ids[1]];

                if ((p1.y <= H && p2.y > H) || (p2.y <= H && p1.y > H)) {
                    float t = (H - p1.y) / (p2.y - p1.y);
                    float x_int = p1.x + t * (p2.x - p1.x);
                    intersections.push_back(x_int);
                }
            }

            std::sort(intersections.begin(), intersections.end());

            for (size_t i = 0; i + 1 < intersections.size(); i += 2) {
                float x_start = intersections[i];
                float x_end = intersections[i+1];

                float current_x = x_start + (stagger ? D * 0.5f : D);

                while (current_x < x_end - (0.3f * D)) {
                    float cand_x = current_x;
                    float cand_y = H;

                    bool valid = true;
                    for (const auto& n : this->nodes) {
                        float dist_sq = (n.x - cand_x) * (n.x - cand_x) + (n.y - cand_y) * (n.y - cand_y);
                        if (dist_sq < min_dist_sq) {
                            valid = false;
                            break;
                        }
                    }

                    if (valid) {
                        this->nodes.emplace_back(cand_x, cand_y, false);
                    }

                    current_x += D;
                }
            }
            stagger = !stagger;
        }
    }
}


void geo::Mesh::create_nodes_2(float spacing) {
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

                if (point_in_mesh(x_p, y_p)) {
                    geo::Node center(x_p, y_p, false);
                    this->nodes.push_back(center);
                }


                //wzięta część z:
                //Chew, L.. (1989). Guaranteed-quality triangular meshes.
                //dodaje punkty wewnątrz okręgów opisanych na trójkątach a nie w ich środkach
                /*
                geo::Node center;
                this->circumcenter(tr, center);
                if (point_in_mesh(center.x, center.y)) {
                    this->nodes.push_back(center);
                }*/
            }

        }

        this->triangulate();
        //std::cout<<mean_size<<'\n';
    }
}

void geo::Mesh::init_afm() {
    std::vector<std::pair<double, double>> allPoints;
    std::vector<int> bc_node_ids;

    allPoints.reserve(this->nodes.size());
    for (const auto& n : this->nodes) {
        allPoints.emplace_back( static_cast<double>(n.x), static_cast<double>(n.y) );
    }

    int num_bc_nodes = this->edges.size();
    bc_node_ids.reserve(num_bc_nodes);
    for (int i = 0; i < num_bc_nodes; i++) {
        bc_node_ids.push_back(i);
    }

    //jeżeli pole ujemnie to punkty nie idą CCW
    double signed_area = 0.0;
    for (int i = 0; i < num_bc_nodes; i++) {
        int next = (i + 1) % num_bc_nodes;
        double x1 = allPoints[bc_node_ids[i]].first;
        double y1 = allPoints[bc_node_ids[i]].second;
        double x2 = allPoints[bc_node_ids[next]].first;
        double y2 = allPoints[bc_node_ids[next]].second;
        signed_area += (x1 * y2 - x2 * y1);
    }

    if (signed_area > 0) {
        std::reverse(bc_node_ids.begin(), bc_node_ids.end());
    }

    if (this->afm != nullptr) {
        delete this->afm;
        this->afm = nullptr;
    }

    this->afm = new AdvancingFront(allPoints, bc_node_ids);
}

void geo::Mesh::create_mesh(float spacing)
{
    // 1. Czyszczenie starych danych
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

    // 2. Interpolujemy punkty na brzegach
    this->interpolate_bc_points(spacing);
    // 3. Generujemy punkty wewnątrz (wg S.H. Lo, 1985)
    this->create_nodes_SH_LO(spacing);
    //this->create_nodes_2(spacing);
    // 4. Inicjowanie frontu
    this->init_afm();
    // 5. Uruchomienie frontu
    this->afm->collapse();

    // 6. Przepisanie trójkątów z frontu na trójkąty siatki
    this->triangles.clear();
    const auto& generated_triangles = this->afm->getTriangles();

    for (const auto& tr : generated_triangles) {
        this->triangles.emplace_back(tr.p1, tr.p2, tr.p3);
    }
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