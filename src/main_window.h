#include "raylib.h"
#include <vector>
#include "mes/mes.h"
#include "mesh/geometry.h"

struct MainWindow{
    const int screenWidth = 1280;
    const int screenHeight = 800;
    float toolbarWidth = 250.0f;

    // camera
    Camera2D camera = { 0 };

    bool showDemoWindow = false;
    bool showXY = true;
    bool creatingMesh = false;
    bool show_grid_bool = true;

    //mesh options
    float spacing = 1.0f;
    bool mesh_created = false;
    bool setting_bcs = false;

    //main options
    float mouseSensitivity = 8.0f;
    float camera_base_zoom = 80.0f;
    bool resetSensitivity = false;

    //solver options
    bool can_solve=false;
    Fem::GlobalData configuration;

    MainWindow();
};