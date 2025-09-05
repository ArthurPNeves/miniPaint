#include <iostream>
#include "libs/httplib.h"
#include "libs/json.hpp"
#include "algorithms.h"
#include "transformations.h"

using json = nlohmann::json;

void set_cors_headers(httplib::Response &res) {
    res.set_header("Access-Control-Allow-Origin", "*");
    res.set_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    res.set_header("Access-Control-Allow-Headers", "Content-Type, Accept");
}

int main() {
    httplib::Server svr;

    // OPTIONS handlers (CORS preflight)
    svr.Options("/draw", [](const httplib::Request &, httplib::Response &res) { set_cors_headers(res); res.status = 200; });
    svr.Options("/transform", [](const httplib::Request &, httplib::Response &res) { set_cors_headers(res); res.status = 200; });
    svr.Options("/clip", [](const httplib::Request &, httplib::Response &res) { set_cors_headers(res); res.status = 200; });
    svr.Options("/.*", [](const httplib::Request &, httplib::Response &res) { set_cors_headers(res); res.status = 200; });

    // POST /draw
    svr.Post("/draw", [](const httplib::Request &req, httplib::Response &res) {
        try {
            auto data = json::parse(req.body);
            std::string tipo = data.value("tipo", std::string(""));
            if (tipo == "") {
                if (data.contains("xc")) tipo = "circulo";
                else tipo = "linha";
            }
            json resposta;
            resposta["tipo"] = tipo;
            resposta["dados"] = data;
            resposta["pixels"] = rasterize(data, tipo);

            set_cors_headers(res);
            res.set_content(resposta.dump(), "application/json");
        } catch (const std::exception &e) {
            set_cors_headers(res);
            res.status = 500;
            res.set_content(json({{"error", e.what()}}).dump(), "application/json");
        }
    });

    // POST /transform
    svr.Post("/transform", [](const httplib::Request &req, httplib::Response &res) {
        try {
            auto data = json::parse(req.body);
            std::string tipo = data["tipo"].get<std::string>();
            json dados = data["dados"];
            std::string transf = data["transf"].get<std::string>();
            json params = data["params"];

            json novosDados = aplicarTransformacao(dados, tipo, transf, params);
            json resposta;
            resposta["tipo"] = tipo;
            resposta["dados"] = novosDados;
            resposta["pixels"] = rasterize(novosDados, tipo);

            set_cors_headers(res);
            res.set_content(resposta.dump(), "application/json");
        } catch (const std::exception &e) {
            set_cors_headers(res);
            res.status = 500;
            res.set_content(json({{"error", e.what()}}).dump(), "application/json");
        }
    });

    // POST /clip  -> usa recortarObjeto para linhas e circulos
    svr.Post("/clip", [](const httplib::Request &req, httplib::Response &res) {
        try {
            auto data = json::parse(req.body);
            json resposta = recortarObjeto(data);
            set_cors_headers(res);
            res.set_content(resposta.dump(), "application/json");
        } catch (const std::exception &e) {
            set_cors_headers(res);
            res.status = 500;
            res.set_content(json({{"error", e.what()}}).dump(), "application/json");
        }
    });

    std::cout << "Servidor rodando em http://localhost:8080\n";
    svr.listen("0.0.0.0", 8080);
    return 0;
}
