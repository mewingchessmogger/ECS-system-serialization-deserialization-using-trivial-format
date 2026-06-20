#pragma once

#include "memory"
#include "string"

#include "reflections.hpp"

struct Renderable{
    uint32_t meshID{};

    REFLECT_1(meshID);
    REFLECT_ADDRESS_1(meshID);
};

struct Parent{
    uint32_t parentID{};
    uint32_t level{};
    REFLECT_2(parentID,level);
    REFLECT_ADDRESS_2(parentID,level)
};

    
