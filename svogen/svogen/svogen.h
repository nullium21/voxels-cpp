//
// Created by lina on 26.12.23.
//

#pragma once

#include "glm/vec3.hpp"

struct PointCloudFileHeader {
    glm::vec3 min_coords, max_coords;
    uint32_t n_points;
    uint8_t n_subdivisions; // resolution = 1 / 2**n_subdivisions
};

using PointCloudVertex = glm::vec3;
