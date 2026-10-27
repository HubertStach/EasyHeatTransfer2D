#pragma once

#include "raylib.h"


inline Vector2 ScreenToWorld(Vector2 screen, const Camera2D &camera) {
    const Vector2 v = GetScreenToWorld2D(screen, camera);
    return { v.x, -v.y };
}

inline Vector2 WorldToScreen(Vector2 world, const Camera2D &camera) {
    return GetWorldToScreen2D({ world.x, -world.y }, camera);
}