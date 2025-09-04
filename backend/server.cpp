#include <iostream>
#include "libs/httplib.h"
#include "libs/json.hpp"
#include "algorithms.h"
#include "transformations.h"

using json = nlohmann::json;

int main() {
    httplib::Server svr;

    svr.Options("/.*", [](const httplib::Request &, httplib::Response &res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
        res.set_header("Access-Control-Allow-Headers", "Content-Type");
        res.status = 200;
    });

    // Criar objeto
    svr.Post("/draw", [](const httplib::Request& req, httplib::Response& res) {
        auto data = json::parse(req.body);
        std::string tipo = (data["algoritmo"] == "bresenham_circulo" ? "circulo" : "linha");

        json resposta;
        resposta["tipo"] = tipo;
        resposta["dados"] = data;
        resposta["pixels"] = rasterize(data, tipo);

        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_content(resposta.dump(), "application/json");
    });

    // Transformar objeto
    svr.Post("/transform", [](const httplib::Request& req, httplib::Response& res) {
        auto data = json::parse(req.body);
        std::string tipoObj = data["tipo"];
        json dados = data["dados"];
        std::string transf = data["transf"];
        json params = data["params"];

        json novosDados = aplicarTransformacao(dados, tipoObj, transf, params);

        json resposta;
        resposta["tipo"] = tipoObj;
        resposta["dados"] = novosDados;
        resposta["pixels"] = rasterize(novosDados, tipoObj);

        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_content(resposta.dump(), "application/json");
    });

    std::cout << "Servidor rodando em http://localhost:8080\n";
    svr.listen("0.0.0.0", 8080);
}
