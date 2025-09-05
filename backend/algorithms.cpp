#include "algorithms.h"
#include <cmath>
#include <algorithm>
#include <limits>
#include <set>
#include <iostream>

// ----------------- rasterização -----------------

// DDA
std::vector<Point> dda(int x1, int y1, int x2, int y2) {
    std::vector<Point> pixels;
    int dx = x2 - x1;
    int dy = y2 - y1;
    int steps = std::max(std::abs(dx), std::abs(dy));
    if (steps == 0) { pixels.push_back({x1,y1}); return pixels; }
    float Xinc = dx / (float) steps;
    float Yinc = dy / (float) steps;
    float X = x1, Y = y1;
    for (int i = 0; i <= steps; ++i) {
        pixels.push_back({(int)std::lround(X), (int)std::lround(Y)});
        X += Xinc; Y += Yinc;
    }
    return pixels;
}

// Bresenham (linha)
std::vector<Point> bresenhamLine(int x0, int y0, int x1, int y1) {
    std::vector<Point> pixels;
    int dx = std::abs(x1 - x0);
    int dy = std::abs(y1 - y0);
    int sx = x0 < x1 ? 1 : -1;
    int sy = y0 < y1 ? 1 : -1;
    int err = dx - dy;

    while (true) {
        pixels.push_back({x0, y0});
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x0 += sx; }
        if (e2 < dx) { err += dx; y0 += sy; }
    }
    return pixels;
}

// Bresenham (círculo)
std::vector<Point> bresenhamCircle(int xc, int yc, int r) {
    std::vector<Point> pixels;
    int x = 0, y = r;
    int d = 3 - 2 * r;
    auto plot8 = [&](int px, int py) {
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
        plot8(x, y);
        ++x;
        if (d > 0) {
            --y;
            d = d + 4 * (x - y) + 10;
        } else {
            d = d + 4 * x + 6;
        }
    }
    // remover duplicatas ocasionais
    std::sort(pixels.begin(), pixels.end());
    pixels.erase(std::unique(pixels.begin(), pixels.end()), pixels.end());
    return pixels;
}

// Rasterize JSON -> pixels (ser usado em /draw e transformações)
json rasterize(const json& dados, const std::string& tipo) {
    std::vector<Point> pts;
    if (tipo == "linha") {
        std::string algoritmo = dados.value("algoritmo", std::string("bresenham"));
        int x1 = dados["x1"].get<int>();
        int y1 = dados["y1"].get<int>();
        int x2 = dados["x2"].get<int>();
        int y2 = dados["y2"].get<int>();
        if (algoritmo == "dda")
            pts = dda(x1, y1, x2, y2);
        else
            pts = bresenhamLine(x1, y1, x2, y2);
    } else if (tipo == "circulo") {
        int xc = dados["xc"].get<int>();
        int yc = dados["yc"].get<int>();
        int r = dados["r"].get<int>();
        pts = bresenhamCircle(xc, yc, r);
    }

    json pixels = json::array();
    for (const auto& p : pts)
        pixels.push_back({{"x", p.first}, {"y", p.second}});
    return pixels;
}

// ----------------- clipping helpers (Cohen–Sutherland / Liang–Barsky) -----------------
namespace {
    constexpr int CS_INSIDE = 0; // 0000
    constexpr int CS_LEFT   = 1; // 0001
    constexpr int CS_RIGHT  = 2; // 0010
    constexpr int CS_BOTTOM = 4; // 0100
    constexpr int CS_TOP    = 8; // 1000

    inline int csOutCode(double x, double y, double xmin, double ymin, double xmax, double ymax) {
        int code = CS_INSIDE;
        if (x < xmin) code |= CS_LEFT;
        else if (x > xmax) code |= CS_RIGHT;
        if (y < ymin) code |= CS_BOTTOM;
        else if (y > ymax) code |= CS_TOP;
        return code;
    }

    bool cohenSutherlandClip(double xmin, double ymin, double xmax, double ymax,
                             double& x0, double& y0, double& x1, double& y1) {
        const double eps = 1e-12;
        int out0 = csOutCode(x0, y0, xmin, ymin, xmax, ymax);
        int out1 = csOutCode(x1, y1, xmin, ymin, xmax, ymax);

        while (true) {
            if ((out0 | out1) == 0) {
                // ambos dentro
                return true;
            } else if (out0 & out1) {
                // ambos fora do mesmo lado
                return false;
            } else {
                int out = out0 ? out0 : out1;
                double x=0,y=0;

                if (out & CS_TOP) {
                    // interseção com y = ymax
                    if (std::fabs(y1 - y0) < eps) {
                        x = x0; // segmento quase horizontal, aproxima
                    } else {
                        x = x0 + (x1 - x0) * ((ymax - y0) / (y1 - y0));
                    }
                    y = ymax;
                } else if (out & CS_BOTTOM) {
                    if (std::fabs(y1 - y0) < eps) {
                        x = x0;
                    } else {
                        x = x0 + (x1 - x0) * ((ymin - y0) / (y1 - y0));
                    }
                    y = ymin;
                } else if (out & CS_RIGHT) {
                    if (std::fabs(x1 - x0) < eps) {
                        y = y0;
                    } else {
                        y = y0 + (y1 - y0) * ((xmax - x0) / (x1 - x0));
                    }
                    x = xmax;
                } else { // LEFT
                    if (std::fabs(x1 - x0) < eps) {
                        y = y0;
                    } else {
                        y = y0 + (y1 - y0) * ((xmin - x0) / (x1 - x0));
                    }
                    x = xmin;
                }

                if (out == out0) {
                    x0 = x; y0 = y;
                    out0 = csOutCode(x0, y0, xmin, ymin, xmax, ymax);
                } else {
                    x1 = x; y1 = y;
                    out1 = csOutCode(x1, y1, xmin, ymin, xmax, ymax);
                }
            }
        }
    }

    bool liangBarskyClip(double xmin, double ymin, double xmax, double ymax,
                         double& x0, double& y0, double& x1, double& y1) {
        double dx = x1 - x0;
        double dy = y1 - y0;

        auto clipTest = [&](double p, double q, double& u1, double& u2)->bool {
            const double eps = 1e-12;
            if (std::fabs(p) < eps) {
                if (q < 0) return false;
                return true;
            }
            double r = q / p;
            if (p < 0) { // entering
                if (r > u2) return false;
                if (r > u1) u1 = r;
            } else { // leaving
                if (r < u1) return false;
                if (r < u2) u2 = r;
            }
            return true;
        };

        double u1 = 0.0, u2 = 1.0;
        if (!clipTest(-dx, x0 - xmin, u1, u2)) return false; // x >= xmin
        if (!clipTest( dx, xmax - x0, u1, u2)) return false; // x <= xmax
        if (!clipTest(-dy, y0 - ymin, u1, u2)) return false; // y >= ymin
        if (!clipTest( dy, ymax - y0, u1, u2)) return false; // y <= ymax

        double nx0 = x0 + u1 * dx;
        double ny0 = y0 + u1 * dy;
        double nx1 = x0 + u2 * dx;
        double ny1 = y0 + u2 * dy;

        x0 = nx0; y0 = ny0; x1 = nx1; y1 = ny1;
        return true;
    }
} // namespace

// ----------------- recortarObjeto -----------------
json recortarObjeto(const json& body) {
    // Normaliza entrada: se veio { tipo, dados, xmin,... } usa body["dados"], senão usa body como "dados"
    json objdados = body.contains("dados") ? body["dados"] : body;
    std::string tipo;
    if (body.contains("tipo")) tipo = body["tipo"].get<std::string>();
    else tipo = (objdados.contains("xc") ? "circulo" : "linha");

    // janela
    double xmin = body.contains("xmin") ? body["xmin"].get<double>() : 0.0;
    double ymin = body.contains("ymin") ? body["ymin"].get<double>() : 0.0;
    double xmax = body.contains("xmax") ? body["xmax"].get<double>() : 0.0;
    double ymax = body.contains("ymax") ? body["ymax"].get<double>() : 0.0;

    std::string metodo = body.value("metodo", std::string("cohen-sutherland"));
    std::string algoritmo = body.value("algoritmo", std::string("bresenham"));

    json resp;
    resp["dados"] = objdados; // devolve os dados originais (não alterados)
    resp["pixels"] = json::array();

    if (tipo == "linha") {
        double x1 = objdados["x1"].get<double>();
        double y1 = objdados["y1"].get<double>();
        double x2 = objdados["x2"].get<double>();
        double y2 = objdados["y2"].get<double>();

        double cx1 = x1, cy1 = y1, cx2 = x2, cy2 = y2;
        bool visible = false;
        if (metodo == "liang-barsky") {
            visible = liangBarskyClip(xmin, ymin, xmax, ymax, cx1, cy1, cx2, cy2);
        } else {
            visible = cohenSutherlandClip(xmin, ymin, xmax, ymax, cx1, cy1, cx2, cy2);
        }

        // Se o segmento intersecta a janela -> rasteriza apenas a parte visível
        if (visible) {
            int ix1 = (int)std::lround(cx1);
            int iy1 = (int)std::lround(cy1);
            int ix2 = (int)std::lround(cx2);
            int iy2 = (int)std::lround(cy2);
            std::vector<Point> pts;
            if (algoritmo == "dda")
                pts = dda(ix1, iy1, ix2, iy2);
            else
                pts = bresenhamLine(ix1, iy1, ix2, iy2);

            for (const auto& p : pts) resp["pixels"].push_back({{"x", p.first}, {"y", p.second}});
        }
        resp["aceita"] = !resp["pixels"].empty();
        return resp;
    }
    else if (tipo == "circulo") {
        int xc = objdados["xc"].get<int>();
        int yc = objdados["yc"].get<int>();
        int r  = objdados["r"].get<int>();

        // Rasteriza todo o círculo e filtra pixels dentro da janela
        std::vector<Point> all = bresenhamCircle(xc, yc, r);
        for (const auto& p : all) {
            if (p.first >= (int)std::lround(xmin) && p.first <= (int)std::lround(xmax)
             && p.second >= (int)std::lround(ymin) && p.second <= (int)std::lround(ymax)) {
                resp["pixels"].push_back({{"x", p.first}, {"y", p.second}});
            }
        }
        resp["aceita"] = !resp["pixels"].empty();
        return resp;
    }

    // Tipo desconhecido -> retorna vazio (objeto permanece, mas sem pixels visíveis).
    resp["aceita"] = false;
    return resp;
}
