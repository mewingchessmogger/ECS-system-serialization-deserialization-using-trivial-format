#pragma once
#include <variant>
#include "string"
#include "array"

using vec3 = std::array<float,3>;
struct Variable{
    std::variant<vec3, int, float, uint32_t, vec3*, int*, float*, uint32_t*> var{};
    std::string name{};
};

struct VariableAddress{
    std::variant<vec3*, int*, float*, uint32_t*> address{};
};
