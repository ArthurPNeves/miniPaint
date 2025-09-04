#pragma once
#include <vector>
#include <utility>
#include <string>
#include "libs/json.hpp"

using Point = std::pair<int,int>;
using json = nlohmann::json;

// Rasterizações
std::vector<Point> dda(int x1, int y1, int x2, int y2);
std::vector<Point> bresenhamLine(int x1, int y1, int x2, int y2);
std::vector<Point> bresenhamCircle(int xc, int yc, int r);

// Função genérica: rasteriza a partir da geometria
json rasterize(
    const json& dados,
    const std::string& tipo
);