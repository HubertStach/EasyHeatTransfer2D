//
// Created by marci on 30.03.2026.
//

#include "progress_bar.h"

#include <iomanip>
#include <iostream>
#include <string>

void showProgress(int current, int max)
{
    if (max <= 0) return;

    float percentage = (float)current / max * 100.0f;

    const int barWidth = 60;
    int filledWidth = (int)(percentage / 100.0f * barWidth);

    std::string bar = "[";
    for (int i = 0; i < barWidth; i++) {
        if (i < filledWidth) {
            bar += "=";
        } else {
            bar += " ";
        }
    }
    bar += "]";

    std::cout << "\r" << bar << " "
            << std::fixed << std::setprecision(1) << percentage << "% "
            << "(" << current<<"/"<<max << ")" << std::flush;


    if (current >= max) {
        std::cout << std::endl;
    }
}
