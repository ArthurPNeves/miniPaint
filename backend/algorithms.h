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

bool cohen_sutherland_clip(double x1, double y1, double x2, double y2,
                                  double rx, double ry, double rw, double rh,
                                  double &ox1, double &oy1, double &ox2, double &oy2);

bool liang_barsky_clip(double x0, double y0, double x1, double y1,
                              double rx, double ry, double rw, double rh,
                              double &ox0, double &oy0, double &ox1, double &oy1);

bool clip_line(int alg,
              double x1,double y1,double x2,double y2,
              double rx,double ry,double rw,double rh,
              double &ox1,double &oy1,double &ox2,double &oy2);

json rasterize(const json& dados, const std::string& tipo);

json recortarObjeto(const json& body);

#endif
