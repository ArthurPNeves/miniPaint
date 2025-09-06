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


const int CS_INSIDE = 0;
const int CS_LEFT   = 1;
const int CS_RIGHT  = 2;
const int CS_BOTTOM = 4;
const int CS_TOP    = 8;

static int computeOutCode(double x, double y, double rx, double ry, double rw, double rh) {
    int code = CS_INSIDE;
    if (x < rx) code |= CS_LEFT;
    else if (x > rx + rw) code |= CS_RIGHT;
    if (y < ry) code |= CS_TOP;
    else if (y > ry + rh) code |= CS_BOTTOM;
    return code;
}

bool cohen_sutherland_clip(double x1, double y1, double x2, double y2,
                           double rx, double ry, double rw, double rh,
                           double &ox1, double &oy1, double &ox2, double &oy2)
{
    int outcode1 = computeOutCode(x1, y1, rx, ry, rw, rh);
    int outcode2 = computeOutCode(x2, y2, rx, ry, rw, rh);
    bool accept = false;

    while (true) {
        if ((outcode1 | outcode2) == 0) {
            accept = true;
            break;
        } else if ((outcode1 & outcode2) != 0) {
            break;
        } else {
            int outcodeOut = outcode1 ? outcode1 : outcode2;
            double x = 0.0, y = 0.0;

            if (outcodeOut & CS_TOP) {
                if (y2 - y1 != 0) {
                    x = x1 + (x2 - x1) * (ry - y1) / (y2 - y1);
                    y = ry;
                }
            } else if (outcodeOut & CS_BOTTOM) {
                if (y2 - y1 != 0) {
                    x = x1 + (x2 - x1) * (ry + rh - y1) / (y2 - y1);
                    y = ry + rh;
                }
            } else if (outcodeOut & CS_RIGHT) {
                if (x2 - x1 != 0) {
                    y = y1 + (y2 - y1) * (rx + rw - x1) / (x2 - x1);
                    x = rx + rw;
                }
            } else if (outcodeOut & CS_LEFT) {
                if (x2 - x1 != 0) {
                    y = y1 + (y2 - y1) * (rx - x1) / (x2 - x1);
                    x = rx;
                }
            }

            if (outcodeOut == outcode1) {
                x1 = x; y1 = y;
                outcode1 = computeOutCode(x1, y1, rx, ry, rw, rh);
            } else {
                x2 = x; y2 = y;
                outcode2 = computeOutCode(x2, y2, rx, ry, rw, rh);
            }
        }
    }

    if (accept) {
        ox1 = x1; oy1 = y1; ox2 = x2; oy2 = y2;
        return true;
    } else {
        return false;
    }
}

bool liang_barsky_clip(double x0, double y0, double x1, double y1,
                       double rx, double ry, double rw, double rh,
                       double &ox0, double &oy0, double &ox1, double &oy1)
{
    double p[4], q[4];
    p[0] = -(x1 - x0);
    p[1] =  (x1 - x0);
    p[2] = -(y1 - y0);
    p[3] =  (y1 - y0);

    q[0] = x0 - rx;
    q[1] = rx + rw - x0;
    q[2] = y0 - ry;
    q[3] = ry + rh - y0;

    double u0 = 0.0, u1 = 1.0;

    for (int i = 0; i < 4; ++i) {
        if (p[i] == 0.0) {
            if (q[i] < 0.0) return false;
        } else {
            double t = q[i] / p[i];
            if (p[i] < 0.0) {
                if (t > u1) return false;
                if (t > u0) u0 = t;
            } else {
                if (t < u0) return false;
                if (t < u1) u1 = t;
            }
        }
    }

    if (u0 > u1) return false;

    double cx0 = x0 + u0 * (x1 - x0);
    double cy0 = y0 + u0 * (y1 - y0);
    double cx1 = x0 + u1 * (x1 - x0);
    double cy1 = y0 + u1 * (y1 - y0);

    ox0 = cx0; oy0 = cy0; ox1 = cx1; oy1 = cy1;
    return true;
}

// wrapper: alg 0 = CS, 1 = LB
bool clip_line(int alg,
               double x1,double y1,double x2,double y2,
               double rx,double ry,double rw,double rh,
               double &ox1,double &oy1,double &ox2,double &oy2)
{
    if (alg == 1) {
        return liang_barsky_clip(x1,y1,x2,y2, rx,ry,rw,rh, ox1,oy1,ox2,oy2);
    } else {
        return cohen_sutherland_clip(x1,y1,x2,y2, rx,ry,rw,rh, ox1,oy1,ox2,oy2);
    }
}


json recortarObjeto(const json &req) {
    json resp;

    try {
        std::string tipo = req.value("tipo", std::string("linha"));
        if (tipo != "linha") {
            resp["aceita"] = false;
            resp["error"] = "Recorte suporta apenas linhas.";
            return resp;
        }

        // Obter retângulo de recorte
        double xmin = req.value("xmin", 0.0);
        double ymin = req.value("ymin", 0.0);
        double xmax = req.value("xmax", 0.0);
        double ymax = req.value("ymax", 0.0);

        // Obter dados da linha
        auto dados = req["dados"];
        double x1 = dados["x1"].get<double>();
        double y1 = dados["y1"].get<double>();
        double x2 = dados["x2"].get<double>();
        double y2 = dados["y2"].get<double>();

        // Aplicar algoritmo de recorte de linha
        std::string algoritmo = req.value("algoritmo", std::string("CoSutherland"));
        int alg = (algoritmo == "LiBarsky") ? 1 : 0;
        
        double ox1, oy1, ox2, oy2;
        double rw = xmax - xmin;
        double rh = ymax - ymin;
        
        bool aceita = clip_line(alg, x1, y1, x2, y2, xmin, ymin, rw, rh, ox1, oy1, ox2, oy2);
        
        if (aceita) {
            // Criar nova linha recortada
            json novosDados = dados;
            novosDados["x1"] = (int)std::round(ox1);
            novosDados["y1"] = (int)std::round(oy1);
            novosDados["x2"] = (int)std::round(ox2);
            novosDados["y2"] = (int)std::round(oy2);
            
            // Rasterizar a nova linha
            auto pixels = rasterize(novosDados, tipo);
            
            resp["aceita"] = true;
            resp["pixels"] = pixels;
        } else {
            resp["aceita"] = false;
            resp["pixels"] = json::array();
        }
        
        return resp;
    } catch (const std::exception &ex) {
        resp["aceita"] = false;
        resp["error"] = std::string("recortarObjeto error: ") + ex.what();
        return resp;
    }
}
