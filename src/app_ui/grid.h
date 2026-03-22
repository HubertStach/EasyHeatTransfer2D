#ifndef GRID_H
#define GRID_H

#include "imgui.h"
#include "raylib.h"

void show_grid(Camera2D camera, float grid_size = 1.0f, Color grid_color = LIGHTGRAY, Color axis_color = GRAY, int screenWidth=1280, int screenHeight=720);
void show_adaptive_grid(Camera2D camera, Color grid_color = LIGHTGRAY, Color axis_color = GRAY);
void show_grid_dim(Camera2D camera, float grid_size = 1.0f);
void show_adaptive_grid_dim(Camera2D camera);

#endif