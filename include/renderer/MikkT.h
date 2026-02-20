#pragma once
#include "Mesh.h"

#include <vector>

class MikkT
{
public:
    static bool Generate(std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);
};
