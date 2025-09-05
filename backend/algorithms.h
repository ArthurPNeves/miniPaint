#ifndef ALGORITHMS_H
#define ALGORITHMS_H

#include <vector>
#include <utility>
#include <string>
#include "libs/json.hpp"

using json = nlohmann::json;
using Point = std::pair<int,int>;

std::vector<Point> dda(int x1, int y1, int x2, int y2);
std::vector<Point> bresenhamLine(int x1, int y1, int x2, int y2);
std::vector<Point> bresenhamCircle(int xc, int yc, int r);

// Rasteriza um objeto (linha / circulo) para pixels
json rasterize(const json& dados, const std::string& tipo);

// Recorta um objeto (linha ou círculo) contra a janela.
// Entrada: pode ser { tipo:"linha", dados:{...}, xmin,ymin,xmax,ymax, metodo, algoritmo }
// ou diretamente os campos (x1,y1,x2,y2,xmin,...). Retorna { dados: original, pixels: [...], aceita: bool }.
json recortarObjeto(const json& body);

#endif // ALGORITHMS_H
