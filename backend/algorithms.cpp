#include "algorithms.h"
#include <cmath>
#include <algorithm>
#include <limits>
#include <set>
#include <iostream>


// DDA
std::vector<Point> dda(int x1, int y1, int x2, int y2) {
    std::vector<Point> pixels;
    int dx = x2 - x1;
    int dy = y2 - y1;
    
    // Calcula o número de passos seguindo a lógica da imagem
    int passos;
    if (std::abs(dx) > std::abs(dy)) {
        passos = std::abs(dx);
    } else {
        passos = std::abs(dy);
    }
    
    if (passos == 0) { 
        pixels.push_back({x1, y1}); 
        return pixels; 
    }
    
    float x_incr = dx / (float)passos;
    float y_incr = dy / (float)passos;
    
    float x = x1, y = y1;
    
    pixels.push_back({(int)std::round(x), (int)std::round(y)}); // pixel inicial
    for (int k = 1; k <= passos; k++) {
        x = x + x_incr;
        y = y + y_incr;
        pixels.push_back({(int)std::round(x), (int)std::round(y)});
    }
    
    return pixels;
}

// Bresenham (linha)
std::vector<Point> bresenhamLine(int x1, int y1, int x2, int y2) {
    std::vector<Point> pixels;
    int dx, dy, x, y, i;
    int const1, const2, p;
    int incrx, incry;
    
    dx = x2 - x1;
    dy = y2 - y1;
    
    if (dx >= 0) {
        incrx = 1;
    } else {
        incrx = -1;
        dx = -dx;
    }
    
    if (dy >= 0) {
        incry = 1;
    } else {
        incry = -1;
        dy = -dy;
    }
    
    x = x1;
    y = y1;
    pixels.push_back({x, y}); 
    
    if (dy < dx) {
        p = 2 * dy - dx;
        const1 = 2 * dy;
        const2 = 2 * (dy - dx);
        
        for (i = 0; i < dx; i++) {
            x += incrx;
            if (p < 0) {
                p += const1;
            } else {
                y += incry;
                p += const2;
            }
            pixels.push_back({x, y});
        }
    } else {
        // Caso onde dy é dominante
        p = 2 * dx - dy;
        const1 = 2 * dx;
        const2 = 2 * (dx - dy);
        
        for (i = 0; i < dy; i++) {
            y += incry;
            if (p < 0) {
                p += const1;
            } else {
                x += incrx;
                p += const2;
            }
            pixels.push_back({x, y}); // colora_pixel
        }
    }
    
    return pixels;
}

// Bresenham (círculo)
std::vector<Point> bresenhamCircle(int xc, int yc, int r) {
    std::vector<Point> pixels;
    int x, y, p;
    
    // Procedimento plot_circle_points - plota os 8 pontos simétricos
    auto plot_circle_points = [&]() {
        pixels.push_back({xc + x, yc + y});
        pixels.push_back({xc - x, yc + y});
        pixels.push_back({xc + x, yc - y});
        pixels.push_back({xc - x, yc - y});
        pixels.push_back({xc + y, yc + x});
        pixels.push_back({xc - y, yc + x});
        pixels.push_back({xc + y, yc - x});
        pixels.push_back({xc - y, yc - x});
    };
    
    // Inicialização
    x = 0;
    y = r;
    p = 3 - 2 * r;
    plot_circle_points();
    
    // Loop principal - enquanto x < y
    while (x < y) {
        if (p < 0) {
            // Caso p < 0
            p = p + 4 * x + 6;
        } else {
            // Caso p >= 0
            p = p + 4 * (x - y) + 10;
            y = y - 1;
        }
        x = x + 1;
        plot_circle_points();
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


// Limites da janela (globais para o algoritmo)
static double xmin_global, xmax_global, ymin_global, ymax_global;

// Função region_code conforme mostrado na imagem
int region_code(double x, double y) {
    int codigo = 0;
    
    if (x < xmin_global) {
        codigo = codigo + 1;
    }
    
    if (x > xmax_global) {
        codigo = codigo + 2;
    }
    
    if (y < ymin_global) {
        codigo = codigo + 4;
    }
    
    if (y > ymax_global) {
        codigo = codigo + 8;
    }
    
    return codigo;
}

bool cohen_sutherland_clip(double x1, double y1, double x2, double y2,
                           double rx, double ry, double rw, double rh,
                           double &ox1, double &oy1, double &ox2, double &oy2)
{
    // Definir limites da janela globalmente
    xmin_global = rx;
    xmax_global = rx + rw;
    ymin_global = ry;
    ymax_global = ry + rh;
    
    bool aceite = false;
    bool feito = false;
    int c1, c2, cfora;
    double xint, yint;
    
    while (!feito) {
        c1 = region_code(x1, y1);
        c2 = region_code(x2, y2);
        
        if ((c1 == 0) && (c2 == 0)) {
            aceite = true;
            feito = true;
        } else if ((c1 & c2) != 0) {
            feito = true;
        } else {
            if (c1 != 0) {
                cfora = c1;
            } else {
                cfora = c2;
            }
            
            // Calcula interseção com os limites da janela
            if ((cfora & 1) == 1) {
                xint = xmin_global;
                yint = y1 + (y2 - y1) * (xmin_global - x1) / (x2 - x1);
            } else if ((cfora & 2) == 2) {
                xint = xmax_global;
                yint = y1 + (y2 - y1) * (xmax_global - x1) / (x2 - x1);
            } else if ((cfora & 4) == 4) {
                yint = ymin_global;
                xint = x1 + (x2 - x1) * (ymin_global - y1) / (y2 - y1);
            } else if ((cfora & 8) == 8) {
                yint = ymax_global;
                xint = x1 + (x2 - x1) * (ymax_global - y1) / (y2 - y1);
            }
            
            if (c1 == cfora) {
                x1 = xint;
                y1 = yint;
            } else {
                x2 = xint;
                y2 = yint;
            }
        }
    }
    
    if (aceite) {
        ox1 = x1; oy1 = y1; ox2 = x2; oy2 = y2;
        return true;
    } else {
        return false;
    }
}

// Função cliptest conforme mostrado na imagem
bool cliptest(double p, double q, double &u1, double &u2) {
    bool result = true;
    
    if (p < 0.0) {
        // Fora para dentro
        double r = q / p;
        if (r > u2) {
            result = false;
        } else if (r > u1) {
            u1 = r;
        }
    } else if (p > 0.0) {
        // Dentro para fora
        double r = q / p;
        if (r < u1) {
            result = false;
        } else if (r < u2) {
            u2 = r;
        }
    } else if (q < 0.0) {
        // p = 0, linha paralela e fora
        result = false;
    }
    
    return result;
}

bool liang_barsky_clip(double x1, double y1, double x2, double y2,
                       double xmin, double ymin, double rw, double rh,
                       double &ox1, double &oy1, double &ox2, double &oy2)
{
    double u1 = 0.0, u2 = 1.0;
    double dx = x2 - x1;
    double dy = y2 - y1;
    double xmax = xmin + rw;
    double ymax = ymin + rh;
    
    // Testa todos os limites da janela
    if (cliptest(-dx, x1 - xmin, u1, u2)) {
        if (cliptest(dx, xmax - x1, u1, u2)) {  
            if (cliptest(-dy, y1 - ymin, u1, u2)) { 
                if (cliptest(dy, ymax - y1, u1, u2)) { 
                    if (u2 < 1.0) {
                        x2 = x1 + u2 * dx;
                        y2 = y1 + u2 * dy;
                    }
                    if (u1 > 0.0) {
                        x1 = x1 + u1 * dx;
                        y1 = y1 + u1 * dy;
                    }
                    
                    // Desenha a linha recortada
                    ox1 = x1; oy1 = y1; ox2 = x2; oy2 = y2;
                    return true;
                }
            }
        }
    }
    
    return false;
}

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
