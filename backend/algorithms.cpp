#include "algorithms.h"
#include <cmath>

// DDA
std::vector<Point> dda(int x1, int y1, int x2, int y2) {
    std::vector<Point> pixels;
    int dx = x2 - x1, dy = y2 - y1;
    int steps = std::max(abs(dx), abs(dy));
    float Xinc = dx / (float) steps;
    float Yinc = dy / (float) steps;
    float X = x1, Y = y1;

    for (int i = 0; i <= steps; i++) {
        pixels.push_back({(int)X, (int)Y});
        X += Xinc; Y += Yinc;
    }
    return pixels;
}

// Bresenham - linha
std::vector<Point> bresenhamLine(int x1, int y1, int x2, int y2) {
    std::vector<Point> pixels;
    int dx = abs(x2 - x1), dy = abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;

    while (true) {
        pixels.push_back({x1, y1});
        if (x1 == x2 && y1 == y2) break;
        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x1 += sx; }
        if (e2 < dx)  { err += dx; y1 += sy; }
    }
    return pixels;
}

// Bresenham - círculo
std::vector<Point> bresenhamCircle(int xc, int yc, int r) {
    std::vector<Point> pixels;
    int x = 0, y = r, d = 3 - 2 * r;

    auto plot = [&](int px, int py) {
        pixels.push_back({xc + px, yc + py});
        pixels.push_back({xc - px, yc + py});
        pixels.push_back({xc + px, yc - py});
        pixels.push_back({xc - px, yc - py});
        pixels.push_back({xc + py, yc + px});
        pixels.push_back({xc - py, yc + px});
        pixels.push_back({xc + py, yc - px});
        pixels.push_back({xc - py, yc - px});
    };

    while (y >= x) {
        plot(x, y);
        x++;
        if (d > 0) { y--; d += 4 * (x - y) + 10; }
        else d += 4 * x + 6;
    }
    return pixels;
}

// Função genérica
json rasterize(const json& dados, const std::string& tipo) {
    std::vector<Point> pts;
    if (tipo == "linha") {
        std::string algoritmo = dados.value("algoritmo", "dda");
        if (algoritmo == "dda")
            pts = dda(dados["x1"], dados["y1"], dados["x2"], dados["y2"]);
        else
            pts = bresenhamLine(dados["x1"], dados["y1"], dados["x2"], dados["y2"]);
    } else if (tipo == "circulo") {
        pts = bresenhamCircle(dados["xc"], dados["yc"], dados["r"]);
    }

    json pixels = json::array();
    for (const auto& p : pts) {
        pixels.push_back({{"x", p.first}, {"y", p.second}});
    }
    return pixels;
}
