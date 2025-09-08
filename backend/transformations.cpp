#include "transformations.h"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Funções auxiliares
static std::pair<int,int> centroLinha(const json& dados) {
    int cx = (dados["x1"].get<int>() + dados["x2"].get<int>()) / 2;
    int cy = (dados["y1"].get<int>() + dados["y2"].get<int>()) / 2;
    return {cx, cy};
}

static std::pair<int,int> centroCirculo(const json& dados) {
    return {dados["xc"], dados["yc"]};
}

// Linha
json transformarLinha(const json& dados, const std::string& transf, const json& params) {
    int x1 = dados["x1"].get<int>();
    int y1 = dados["y1"].get<int>();
    int x2 = dados["x2"].get<int>();
    int y2 = dados["y2"].get<int>();
    auto [cx, cy] = centroLinha(dados);

    if (transf == "translacao") {
        int dx = params["dx"].get<int>();
        int dy = params["dy"].get<int>();
        x1 += dx; y1 += dy;
        x2 += dx; y2 += dy;
    }
    else if (transf == "escala") {
        float sx = params["sx"].get<float>();
        float sy = params["sy"].get<float>();
        x1 = (int)((x1 - cx) * sx + cx);
        y1 = (int)((y1 - cy) * sy + cy);
        x2 = (int)((x2 - cx) * sx + cx);
        y2 = (int)((y2 - cy) * sy + cy);
    }
    else if (transf == "rotacao") {
        float ang = params["angulo"].get<float>() * M_PI / 180.0f;
        auto rot = [&](int x, int y) {
            int dx = x - cx, dy = y - cy;
            return std::pair<int,int>(
                (int)(dx * cos(ang) - dy * sin(ang) + cx),
                (int)(dx * sin(ang) + dy * cos(ang) + cy)
            );
        };
        auto p1 = rot(x1,y1), p2 = rot(x2,y2);
        x1=p1.first; y1=p1.second; x2=p2.first; y2=p2.second;
    }
    else if (transf == "reflexao") {
        std::string eixo = params["eixo"].get<std::string>();
        auto ref = [&](int x, int y) {

            if (eixo=="x") return std::pair<int,int>(x, -y);       
            if (eixo=="y") return std::pair<int,int>(-x, y);        
            return std::pair<int,int>(-x, -y);                     
        };
        auto p1 = ref(x1,y1), p2 = ref(x2,y2);
        x1=p1.first; y1=p1.second; x2=p2.first; y2=p2.second;
    }

    json novo = dados;
    novo["x1"]=x1; novo["y1"]=y1; novo["x2"]=x2; novo["y2"]=y2;
    return novo;
}


// Círculo
json transformarCirculo(const json& dados, const std::string& transf, const json& params) {
    int xc = dados["xc"].get<int>();
    int yc = dados["yc"].get<int>();
    int r  = dados["r"].get<int>();
    auto [cx, cy] = centroCirculo(dados);
    (void)cx; // Suprime o aviso de variável não utilizada
    (void)cy;

    if (transf == "translacao") {
        int dx = params["dx"].get<int>();
        int dy = params["dy"].get<int>();
        xc += dx; yc += dy;
    }
    else if (transf == "escala") {
        float sx = params["sx"].get<float>();
        float sy = params["sy"].get<float>();
        float s = (sx+sy)/2.0f;
        r = (int)(r * s);
    }
    else if (transf == "rotacao") {
        // círculo não muda
    }
    else if (transf == "reflexao") {
        std::string eixo = params["eixo"].get<std::string>();
        // Para círculos no sistema cartesiano com reflexão em relação à origem (0,0):
        // - Reflexão em X: mantém xc, inverte yc → (xc, -yc)
        // - Reflexão em Y: inverte xc, mantém yc → (-xc, yc)
        // - Reflexão em XY: inverte ambos → (-xc, -yc)
        if (eixo=="x") yc = -yc;                    // reflexão no eixo X (horizontal)
        else if (eixo=="y") xc = -xc;               // reflexão no eixo Y (vertical)  
        else { xc = -xc; yc = -yc; }                // reflexão na origem
    }

    json novo = dados;
    novo["xc"]=xc; novo["yc"]=yc; novo["r"]=r;
    return novo;
}



json aplicarTransformacao(
    const json& dados,
    const std::string& tipoObj,
    const std::string& transf,
    const json& params
) {
    if (tipoObj == "linha") return transformarLinha(dados, transf, params);
    if (tipoObj == "circulo") return transformarCirculo(dados, transf, params);
    return dados;
}
