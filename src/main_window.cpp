#include "../../_deps/imgui-src/imgui.h"
#include "../../_deps/rlimgui-src/rlImGui.h"
#include "../../_deps/rlimgui-src/rlImGuiColors.h"

#include "raylib.h"
#include "../include/rlgl.h"
#include "../include/raymath.h"

#include <vector>

#include "app_ui/axis.h"
#include "app_ui/grid.h"

#include "mes/mes.h"
#include "mesh/geometry.h"
//#include "mesh/mesh.h"

#include "saving_data.h"
#include "visual/visualisation.h"
#include "main_window.h"
#include "thread"

MainWindow::MainWindow()
{
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(screenWidth, screenHeight, "easyFEM");
    SetTargetFPS(60);
    rlImGuiSetup(true);

    camera.offset = {screenWidth/2.0f + 0.5f*toolbarWidth, screenHeight/2.0f};
    camera.target = {0.0f, 0.0f};
    camera.rotation = 0.0f;
    camera.zoom = camera_base_zoom;

    geo::Mesh mesh;

    float bc_flux = 0.0f;
    float bc_alfa = 0.0f;
    float bc_text = 0.0f;

    int bc_edge_clicked = -1;
    //lista przechowuje krawędzie
    std::vector<int> selected_edges;
    bool bc_options_saved = false;
    bool problem_solved = false;

    //visualisation
    bool loading_visual = false;
    Visualisation vis;
    bool auto_play = false;
    bool display_nodes = true;
    bool display_triangles = true;
    bool display_quads = true;

    std::string solver_type_str = "implicit_euler";
    int current_solver = 1; // 0 = Explicit, 1 = Implicit, 2 = Crank-Nicolson

    //cleaning Data folder
    clean_vtu_files();
    std::cout<<"cleaning vtu files...\n";

    while (!WindowShouldClose())
    {
        // DRAW START
        BeginDrawing();
        ClearBackground(RAYWHITE);

        int currentWidth = GetRenderWidth();
        int currentHeight = GetRenderHeight();

        // ------------------ IMGUI LAYOUT ------------------
        rlImGuiBegin();

        ImGui::SetNextWindowPos({0,0}, ImGuiCond_Always);
        ImGui::SetNextWindowSize({(float)currentWidth,(float)currentHeight}, ImGuiCond_Always);
        ImGui::Begin("Root", nullptr,
                    ImGuiWindowFlags_NoDecoration |
                    ImGuiWindowFlags_NoBringToFrontOnFocus);

        // Toolbar
        ImGui::BeginChild("Toolbar", {toolbarWidth,0}, true);
        ImGui::Text("Tools");
        ImGui::Separator();

        if (ImGui::Button("Reset Camera")){
            camera.zoom   = camera_base_zoom;
            camera.target = { 0.0f, 0.0f };
        }

        if (ImGui::Button("Clean Data folder")){
            clean_vtu_files();
            std::cout<<"cleaning vtu files...\n";
        }
        if (ImGui::Button("Load .txt mesh")) {
            mesh.load_mesh_from_txt();
            mesh_created = true;
            problem_solved = false;
        }
        if (ImGui::Button("Load .inp file")){
            load_inp_mesh(mesh);
            //w tym przypadku dajemy możliwość użytkownikowi żeby sam ustawił BCs, jako że czasami są problemy z siatkami
            //i tym które węzły mają BC, dajemy możliwość dowolnego zdefiniowania brzegu z BC.
            //Ta funkcja tworzy ze wszytskich krawędzi (też i tych wewnątrz) krawędzie na których można ustawić BC.
            mesh.create_edges();
            mesh_created = true;
            problem_solved = false;
        }

        ImGui::BeginChild("##child1", ImVec2(0, 300), true);
        ImGui::Text("Mesh settings");
        ImGui::Checkbox("Start adding points", &creatingMesh);
        ImGui::TextDisabled("E: Place point | Q: Pop");

        if (ImGui::Button("Create Mesh")) {
            if (mesh.nodes.size() >= 3) {
                mesh_created = true;
                mesh.create_mesh(spacing);
                problem_solved = false;
            }
        }

        if (ImGui::Button("Reset nodes")){
            //resetowanie siatki
            mesh.nodes.clear();
            mesh.triangles.clear();
            mesh.edges.clear();
            mesh.quads.clear();
            bc_edge_clicked = -1;
            selected_edges.clear();
            mesh_created = false;
            problem_solved = false;
        }
        ImGui::Separator();

        if (bc_edge_clicked != -1) {
            if (selected_edges.size() > 1) {
                ImGui::Text("Selected %zu continuous edges", selected_edges.size());
            } else {
                ImGui::Text("BC edge %d id clicked", bc_edge_clicked);
            }

            ImGui::InputFloat("Flux", &bc_flux);
            ImGui::InputFloat("Alpha", &bc_alfa);
            ImGui::InputFloat("T_ext", &bc_text);

            if(ImGui::Button("Save")){
                for(int edge_idx : selected_edges) {
                    int node_id1 = mesh.edges[edge_idx].node_ids[0];
                    int node_id2 = mesh.edges[edge_idx].node_ids[1];

                    mesh.nodes[node_id1].bc.initialised = true;
                    mesh.nodes[node_id1].bc.flux = bc_flux;
                    mesh.nodes[node_id1].bc.alfa = bc_alfa;
                    mesh.nodes[node_id1].bc.t_ext = bc_text;

                    mesh.nodes[node_id2].bc.initialised = true;
                    mesh.nodes[node_id2].bc.flux = bc_flux;
                    mesh.nodes[node_id2].bc.alfa = bc_alfa;
                    mesh.nodes[node_id2].bc.t_ext = bc_text;

                    mesh.edges[edge_idx].bc_edge.initialised = true;
                    mesh.edges[edge_idx].bc_edge.flux = bc_flux;
                    mesh.edges[edge_idx].bc_edge.alfa = bc_alfa;
                    mesh.edges[edge_idx].bc_edge.t_ext = bc_text;
                }

                bc_flux = 0.0f;
                bc_alfa = 0.0f;
                bc_text = 0.0f;
            }
        }

        if (ImGui::CollapsingHeader("Mesh options")){
            ImGui::BeginChild("##child3", ImVec2(0, 52), true);
            ImGui::Text("Node spacing density");
            ImGui::SliderFloat("##", &spacing, 0.1f, 5.0f);

            ImGui::EndChild();
        }

        ImGui::EndChild();
        ImGui::BeginChild("SolverData", ImVec2(0, 300), true);

            ImGui::Text("Total time");
            ImGui::InputFloat("s", &configuration.total_time);
            ImGui::Text("Time step");
            ImGui::InputFloat("s deltaT", &configuration.time_step);
            ImGui::Text("Conductivity");
            ImGui::InputFloat("W/mk", &configuration.conductivity);
            ImGui::Text("Initial temperature");
            ImGui::InputFloat("°C T0", &configuration.init_temperature);
            ImGui::Text("Density");
            ImGui::InputFloat("kg/m^3", &configuration.density);
            ImGui::Text("Speific hest");
            ImGui::InputFloat("J/kgK", &configuration.specific_heat);

            ImGui::Text("Solver type");

            if (ImGui::RadioButton("Explicit Euler", &current_solver, 0)) {
                solver_type_str = "explicit_euler";
            }
            if (ImGui::RadioButton("Implicit Euler", &current_solver, 1)) {
                solver_type_str = "implicit_euler";
            }
            if (ImGui::RadioButton("Crank-Nicolson", &current_solver, 2)) {
                solver_type_str = "crank-nicolson";
            }

            if (ImGui::Button("Save")){
                std::cout<<"Save clicked\n";
                if(mesh_created){
                   save_fem_data(mesh, configuration);
                }
            }


            if (ImGui::Button("Solve")){
                if (mesh_created) {
                    clean_vtu_files();
                    try{
                        /*
                        1. tworzymy nowy watek który policzy nam cały problem
                        2. fem_solve wczytuje plik problemowy, tworząc plik rozwiązania (solution)
                        3. w konstruktorze solution wczytuje on siatke z pliku i dane problemowe
                        4. potem funkcja solve na obiekcie solution rozwiązuje problem i zapisuje
                        wynik w każdym momencie czasowym w formacie .vtu

                        */

                        std::thread fem_thread(fem_solve, solver_type_str);
                        fem_thread.join();

                        vis.init_visualisation(mesh);
                        problem_solved = true;
                    }
                    catch (const std::runtime_error& re) {
                        std::cout << "Runtime error: " << re.what() << std::endl;
                    }
                    catch(const std::exception& ex) {
                        std::cout << "Error occurred: " << ex.what() << std::endl;
                    }
                    catch(...){
                        std::cout << "Unknown error" <<std::endl;
                    }
                }
                else {
                    std::cout<<"Create mesh first\n";
                }
            }

        ImGui::EndChild();

        if (problem_solved) {
            ImGui::BeginChild("Visualisation", ImVec2(0, 200), true);

            ImGui::Checkbox("visual", &loading_visual);
            ImGui::InputFloat("Minimum", &vis.min_temp);
            ImGui::InputFloat("Maximum", &vis.max_temp);

            if (vis.solved && loading_visual) {
                ImGui::Checkbox("Autoplay", &auto_play);
                int max_idx = vis.time_ids.size() - 1;
                ImGui::Text("Krok czasowy: %d (Czas: %d)", vis.current_step, vis.time_ids[vis.current_step]);
                ImGui::SliderInt("Oś czasu", &vis.current_step, 0, max_idx);

                if (auto_play) {
                    int next_id = vis.current_step + 1;
                    if (next_id >= vis.time_ids.size()) {
                        next_id = 0;
                    }
                    vis.current_step = next_id;
                }

                ImGui::Checkbox("Display nodes", &display_nodes);
                ImGui::Checkbox("Display triangles", &display_triangles);
                ImGui::Checkbox("Display quads", &display_quads);
            }

            if (!loading_visual) {
                display_nodes = true;
                display_triangles = true;
                display_quads = true;
            }

            ImGui::EndChild();
        }

        if (ImGui::CollapsingHeader("Options")){

            ImGui::BeginChild("##child4", ImVec2(0, 200), true);
            ImGui::Checkbox("Show XY", &showXY);
            ImGui::Checkbox("Show grid", &show_grid_bool);

            ImGui::Text("Mouse sensitivity");
            ImGui::SliderFloat("##", &mouseSensitivity, 0.1, 20);
            if(ImGui::Button("Reset sensitivity")){
                mouseSensitivity = 8.0f;
            }
            ImGui::EndChild();
        }

        ImGui::EndChild(); //koniec lewego panelu

        ImGui::SameLine();

        ImGui::BeginChild("MainPanel", {0,0}, false);
        ImVec2 panelMin = ImGui::GetCursorScreenPos();
        ImVec2 panelSize = ImGui::GetContentRegionAvail();
        bool panelHover = ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup);
        ImGui::EndChild();

        ImGui::End();

        rlImGuiEnd();

        // ------------------ CAMERA INTERACTION ------------------
        // Only pan/zoom when hovering MainPanel
        camera.offset = { panelMin.x + panelSize.x / 2.0f, panelMin.y + panelSize.y / 2.0f };
        if (panelHover)
        {
            if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
            {
                Vector2 delta = GetMouseDelta();
                camera.target.x -= delta.x / camera.zoom;
                camera.target.y -= delta.y / camera.zoom;
            }
            float wheel = GetMouseWheelMove();
            if (wheel != 0.0f)
            {
                camera.zoom += wheel * mouseSensitivity;
                if (camera.zoom < 10.0f) camera.zoom = 10.0f;
            }
        }

        // ------------------ MAIN WINDOW ------------------
        // Creating scissors to cut main view part for the raylib
        int sx = (int)panelMin.x;
        int sy = currentHeight - (int)(panelMin.y + panelSize.y);
        int sw = (int)panelSize.x;
        int sh = (int)panelSize.y;

        rlEnableScissorTest();
        rlScissor(sx, sy, sw, sh);

        BeginMode2D(camera);
            // Whole rendering is being done here

            if(show_grid_bool){
                show_adaptive_grid_dim(camera);
                //show_grid_dim(camera);
            }

            //rysowanie punktów i wielokątów
            mesh.draw_edges();

            if (bc_edge_clicked != -1 && !selected_edges.empty()) {
                for (int edge_idx : selected_edges) {
                    if (edge_idx >= 0 && edge_idx < (int)mesh.edges.size()) {
                        int n1 = mesh.edges[edge_idx].node_ids[0];
                        int n2 = mesh.edges[edge_idx].node_ids[1];
                        Vector2 p1 = { mesh.nodes[n1].x, mesh.nodes[n1].y };
                        Vector2 p2 = { mesh.nodes[n2].x, mesh.nodes[n2].y };
                        DrawLineEx(p1, p2, 3.0f * (1.0f / camera.zoom), MAGENTA);
                    }
                }
            }

            if (loading_visual && vis.solved) {
                if (vis.current_step >= 0 && vis.current_step < vis.time_ids.size()) {

                    std::vector<double> current_temps;
                    current_temps.reserve(mesh.nodes.size());
                    for (size_t i = 0; i < mesh.nodes.size(); i++) {
                        if (i < vis.temps_matrix.rows) {
                            current_temps.push_back(vis.temps_matrix[i][vis.current_step]);
                        } else {
                            current_temps.push_back(0.0);
                        }
                    }

                    mesh.draw_tr_grad(current_temps, vis.max_temp, vis.min_temp);
                    mesh.draw_q_grad(current_temps, vis.max_temp, vis.min_temp);
                    //mesh.draw_tr(current_temps, vis.max_temp, vis.min_temp);
                }
            }
            if (display_triangles) {
                mesh.draw_tr();
            }

            if (display_quads) {
                mesh.draw_q();
            }

            if (display_nodes) {
                mesh.draw_nodes(3*(1/camera.zoom));
            }

        EndMode2D();


        Vector2 worldPos = GetScreenToWorld2D(GetMousePosition(), camera);
        // Snap to grid
        worldPos.x = round(worldPos.x / 0.1f) * 0.1f;
        worldPos.y = round(worldPos.y / 0.1f) * 0.1f;

        // Convert snapped world position back to screen position for drawing
        Vector2 screenPos = GetWorldToScreen2D(worldPos, camera);

        if (panelHover && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {

            int hit_node_id = geo::get_edge_clicked(mesh.edges, mesh.nodes, worldPos.x, worldPos.y);

            if (hit_node_id != -1) {

                bc_edge_clicked = hit_node_id;
                bc_options_saved = false;

                if (mesh_created && (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT))) {
                    selected_edges = mesh.get_continuous_edges(hit_node_id);
                } else {
                    selected_edges.clear();
                    selected_edges.push_back(hit_node_id);
                }

                if (mesh.edges[hit_node_id].bc_edge.initialised) {
                    bc_flux = mesh.edges[hit_node_id].bc_edge.flux;
                    bc_alfa = mesh.edges[hit_node_id].bc_edge.alfa;
                    bc_text = mesh.edges[hit_node_id].bc_edge.t_ext;
                } else {
                    bc_flux = 0.0f;
                    bc_alfa = 0.0f;
                    bc_text = 0.0f;
                }
            }
            else {
                bc_edge_clicked = -1;
            }
        }
        //std::cout<<worldPos.x<<", "<<worldPos.y<<"\n";

        if(creatingMesh){
            DrawCircleV(screenPos, 3, RED);

            Vector2 pos_vec({-44, -24 });
            DrawTextEx(GetFontDefault(), TextFormat("[%0.1f, %0.1f]", (float)worldPos.x, (float)worldPos.y), Vector2Add(screenPos, pos_vec), 20, 2, BLACK);

            if(IsKeyPressed(KEY_E)){
                //dodawanie punktu
                mesh.add_point(worldPos.x, worldPos.y);
            }

            if(IsKeyPressed(KEY_Q)){
                //usuwanie punktu
                mesh.pop_point();
                mesh.mesh_created=false;
                mesh_created=false;
                bc_edge_clicked = -1;
                selected_edges.clear();
                loading_visual = false;
               //std::cout<<mesh.nodes.size()<<"\n";
            }

        }

        rlDisableScissorTest();

        if(showXY){
            DrawCoordinateAxes(camera, panelMin, panelSize);
        }

        EndDrawing();
        // DRAW END
    }

    rlImGuiShutdown();
    CloseWindow();
}