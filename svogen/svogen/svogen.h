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

struct SvoFileHeader {
    glm::ivec3 min_coords, max_coords;
    uint32_t n_subdiv;
    uint32_t n_nodes;
};

struct SvoNode {
    bool is_filled : 1;
    uint8_t sign_x : 1;
    uint8_t sign_y : 1;
    uint8_t sign_z : 1;
    int32_t prev_sibling_offset, next_sibling_offset;
    int32_t first_child_offset;
};
