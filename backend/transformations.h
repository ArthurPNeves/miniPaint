#pragma once
#include "libs/json.hpp"
#include <string>

using json = nlohmann::json;

json transformarLinha(const json& dados, const std::string& transf, const json& params);
json transformarCirculo(const json& dados, const std::string& transf, const json& params);

json aplicarTransformacao(
    const json& dados,
    const std::string& tipoObj,
    const std::string& transf,
    const json& params
);
