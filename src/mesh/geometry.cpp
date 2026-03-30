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

void geo::Mesh::create_nodes(float spacing) {
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
    this->create_nodes(spacing);
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