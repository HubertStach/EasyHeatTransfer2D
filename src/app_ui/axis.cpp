#include "imgui.h"

#include "raylib.h"
#include "../../include/rlgl.h"
#include "../../include/raymath.h"

#include "axis.h"

#include "rlImGui.h"
#include "rlImGuiColors.h"
#include <cstdio>

#define DARKGREY_303030 CLITERAL(Color){29, 29, 29, 255}

void DrawCoordinateAxes(Camera2D camera, ImVec2 panelMin, ImVec2 panelSize) {
    // Oblicz widoczny obszar świata
    Vector2 worldTopLeft = GetScreenToWorld2D({panelMin.x, panelMin.y}, camera);
    Vector2 worldBottomRight = GetScreenToWorld2D({panelMin.x + panelSize.x, panelMin.y + panelSize.y}, camera);
    
    // Parametry osi
    float axisHeight = 25; // wysokość osi X (na dole)
    float axisWidth = 35;  // szerokość osi Y (z lewej)
    
    // Tła dla osi (semi-transparent)
    DrawRectangle(panelMin.x, panelMin.y + panelSize.y - axisHeight, panelSize.x, axisHeight, 
                  ColorAlpha(DARKGREY_303030, 0.8f));
    DrawRectangle(panelMin.x, panelMin.y, axisWidth, panelSize.y - axisHeight, 
                  ColorAlpha(DARKGREY_303030, 0.8f));
    
    DrawLine(panelMin.x+axisWidth/1.1, worldTopLeft.y, panelMin.x+axisWidth/1.1, panelMin.y + panelSize.y - axisHeight, WHITE);
    DrawLine(panelMin.x, panelMin.y + panelSize.y - axisHeight, panelMin.x+panelSize.x, panelMin.y + panelSize.y - axisHeight, WHITE);
    // Oś X (na dole)
    DrawAxisX(worldTopLeft.x, worldBottomRight.x, panelMin, panelSize, axisHeight);
    
    // Oś Y (z lewej)  
    DrawAxisY(worldTopLeft.y, worldBottomRight.y, panelMin, panelSize, axisWidth, axisHeight, camera);
    
    // Narożnik (przecięcie osi)
    DrawRectangle(panelMin.x, panelMin.y + panelSize.y - axisHeight, axisWidth-5, axisHeight, DARKGREY_303030);
}

void DrawAxisX(float worldLeft, float worldRight, ImVec2 panelMin, ImVec2 panelSize, float axisHeight) {
    float worldWidth = worldRight - worldLeft;
    float step = CalculateAxisStep(worldWidth);
    
    // Znajdź pierwszy znacznik
    float firstTick = floor(worldLeft / step) * step;
    
    for (float worldX = firstTick; worldX <= worldRight + step; worldX += step) {
        // Przelicz na pozycję ekranu
        float screenX = panelMin.x + ((worldX - worldLeft) / worldWidth) * panelSize.x;
        
        if (screenX >= panelMin.x + 35 && screenX <= panelMin.x + panelSize.x) { // +35 dla osi Y
            // Znacznik
            DrawLine(screenX, panelMin.y + panelSize.y - axisHeight, 
                    screenX, panelMin.y + panelSize.y - axisHeight + 8, WHITE);
            
            // Tekst
            char text[32];
            snprintf(text, sizeof(text), "%.1f", worldX);
            int textWidth = MeasureText(text, 8);
            DrawText(text, screenX - textWidth/2, 
                    panelMin.y + panelSize.y - axisHeight + 10, 8, WHITE);
        }
    }
}

void DrawAxisY(float worldTop, float worldBottom, ImVec2 panelMin, ImVec2 panelSize, float axisWidth, float axisHeight, Camera2D camera) {
    float worldHeight = worldBottom - worldTop;
    float step = CalculateAxisStep(worldHeight);
    
    float firstTick = floor(worldTop / step) * step;
    
    for (float worldY = firstTick; worldY <= worldBottom + step; worldY += step) {
        // UŻYJ BEZPOŚREDNIO funkcji raylib - tak jak robi to Twoja kamera
        Vector2 worldPoint = {0, worldY}; // dowolne X, ważne tylko Y
        Vector2 screenPoint = GetWorldToScreen2D(worldPoint, camera);
        
        // Sprawdź czy punkt jest w obszarze osi
        if (screenPoint.y >= panelMin.y && screenPoint.y <= panelMin.y + panelSize.y - axisHeight) {
            // Znacznik
            DrawLine(panelMin.x + axisWidth - 8, screenPoint.y, panelMin.x + axisWidth, screenPoint.y, WHITE);
            
            // Tekst
            char text[32];
            snprintf(text, sizeof(text), "%.1f", worldY);
            DrawText(text, panelMin.x + 2, screenPoint.y - 4, 8, WHITE);
        }
    }
}

float CalculateAxisStep(float range) {
    if (range <= 0) return 1.0f;
    
    float rawStep = range / 8.0f; // ~8 znaczników
    float magnitude = pow(10, floor(log10(rawStep)));
    float normalized = rawStep / magnitude;
    
    if (normalized <= 1) return magnitude;
    else if (normalized <= 2) return 2 * magnitude;
    else if (normalized <= 5) return 5 * magnitude;
    else return 10 * magnitude;
}
