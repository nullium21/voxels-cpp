//
// Created by lina on 26.12.23.
//

#include <cstdio>
#include <vector>
#include <optional>
#include <iostream>

#include <svogen.h>
#include <glm/common.hpp>

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#define VOXELIZER_IMPLEMENTATION
#include "voxelizer.h"
#include "glm/gtx/string_cast.hpp"

struct ObjData {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;

    std::string warn, err;

    bool has_errors() const {
        return !err.empty();
    }

    static std::optional<ObjData> load(const char *filename) {
        ObjData data;
        bool ok = tinyobj::LoadObj(&data.attrib, &data.shapes, &data.materials, &data.warn, &data.err, filename);

        std::printf("%s", data.err.data());

        if (!ok) return {};
        return data;
    }
};

vx_point_cloud_t *voxelize_mesh(const char *mesh_filename, glm::vec3 resolution, float precision = 0.1) {
    auto obj = ObjData::load(mesh_filename);
    if (!obj) return nullptr;

    if (obj->has_errors()) std::fprintf(stderr, "%s error:\n%s\n", mesh_filename, obj->err.data());

    auto &shape = obj->shapes[0];
    vx_mesh_t *mesh = vx_mesh_alloc(obj->attrib.vertices.size(), shape.mesh.indices.size());

    for (int i = 0; i < shape.mesh.indices.size(); i++) mesh->indices[i] = shape.mesh.indices[i].vertex_index;
    for (int i = 0; i < obj->attrib.vertices.size(); i++) {
        auto *vert = &(obj->attrib.vertices[3*i]);
        mesh->vertices[i].x = vert[0];
        mesh->vertices[i].y = vert[1];
        mesh->vertices[i].z = vert[2];
    }

    vx_point_cloud *pc = vx_voxelize_pc(mesh, resolution.x, resolution.y, resolution.z, precision);
    vx_mesh_free(mesh);

    return pc;
}

bool point_in_aabb_exists(const vx_point_cloud_t *pc, glm::vec3 minc, glm::vec3 maxc) {
    for (int i = 0; i < pc->nvertices; i++) {
        glm::vec3 vert(pc->vertices[i].x, pc->vertices[i].y, pc->vertices[i].z);
        if (glm::all(glm::greaterThanEqual(vert, minc)) && glm::all(glm::lessThanEqual(vert, maxc)))
            return true;
    }

    return false;
}

void create_svo_children(const vx_point_cloud_t *pc, uint self_index, std::vector<SvoNode> &nodes, glm::vec3 minc, glm::vec3 maxc, uint max_sd, uint subdiv) {
    constexpr glm::ivec3 mults[] = {
            { 1, 1, 1 }, { -1, -1, -1 },
            { -1, 1, 1 }, { 1, -1, 1 }, { 1, 1, -1 },
            { -1, -1, 1 }, { -1, 1, -1 }, { 1, -1, -1 }
    };

    glm::vec3 center = (maxc + minc) / 2.f;
    glm::vec3 sub_half_size = (maxc - minc) / 4.f;

    std::printf("Processing subtree %s..%s (subdivision %d/%d)...\n", glm::to_string(minc).data(), glm::to_string(maxc).data(), subdiv, max_sd);

    if (subdiv >= max_sd || !nodes[self_index].is_filled) return;

    uint32_t last_index = 0;
#pragma unroll
    for (glm::vec3 sub_mult : mults) {
        glm::vec3 sub_center = center + (sub_half_size / 2.f * sub_mult);

        glm::vec3 sub_min = sub_center - sub_half_size;
        glm::vec3 sub_max = sub_center + sub_half_size;

        uint index = nodes.size();
        SvoNode &node = nodes.emplace_back(SvoNode {
                point_in_aabb_exists(pc, sub_min, sub_max),
                signbit(sub_mult.x), signbit(sub_mult.y), signbit(sub_mult.z),
                last_index ? ((int32_t) index) - ((int32_t) last_index) : 0,
                0, 0
        });

        if (!nodes[self_index].first_child_offset)
            nodes[self_index].first_child_offset = ((int32_t) index) - (int32_t) self_index;

        if (last_index) nodes[last_index].next_sibling_offset = ((int32_t) index) - ((int32_t) last_index);

        create_svo_children(pc, index, nodes, sub_min, sub_max, max_sd, subdiv + 1);
        last_index = index;
    }
}

uint calc_n_subdiv(uint8_t n_subdiv_frac, const glm::vec3 &min_coords, const glm::vec3 &max_coords) {
    glm::vec3 size = max_coords - min_coords;
    float side_length = std::max(size.x, std::max(size.y, size.z));
    uint side_length_p2 = std::bit_ceil((uint) std::ceil(side_length));
    uint n_subdiv = std::log2(side_length_p2) + n_subdiv_frac;
    return n_subdiv;
}

std::pair<glm::vec3, glm::vec3> calc_pc_aabb(const vx_point_cloud_t *pc) {
    glm::vec3 min_coords(INFINITY), max_coords(-INFINITY);
    for (int i = 0; i < pc->nvertices; i++) {
        vx_vertex_t &vert = pc->vertices[i];
        glm::vec3 v(vert.x, vert.y, vert.z);
        min_coords = glm::min(min_coords, v);
        max_coords = glm::max(max_coords, v);
    }
    return {min_coords, max_coords};
}

void create_svo(const vx_point_cloud_t *pc, std::vector<SvoNode> &nodes, glm::vec3 min_coords, glm::vec3 max_coords, uint n_subdiv) {
    SvoNode &root = nodes.emplace_back(SvoNode {
        pc->nvertices > 0,
        0, 0, 0,
        0, 0, 0
    });

    create_svo_children(pc, 0, nodes, min_coords, max_coords, n_subdiv, 0);
}

int main() {
    constexpr uint8_t n_subdiv_frac = 3;
    constexpr float resolution = 1.f / (1 << n_subdiv_frac);

    auto *pc = voxelize_mesh("suzanne.obj", glm::vec3(resolution), resolution / 8);
    if (!pc) return 1;

    PointCloudFileHeader header = {
            glm::vec3(std::numeric_limits<float>::max()),
            glm::vec3(std::numeric_limits<float>::min()),
            (uint32_t) pc->nvertices,
            n_subdiv_frac
    };

    for (int i = 0; i < pc->nvertices; i++) {
        glm::vec3 vert(pc->vertices[i].x, pc->vertices[i].y, pc->vertices[i].z);
        header.max_coords = glm::max(header.max_coords, vert);
        header.min_coords = glm::min(header.min_coords, vert);
    }

    FILE *fp = fopen("suzanne-points.bin", "wb");
    fwrite(&header, sizeof(PointCloudFileHeader), 1, fp);
    fwrite(pc->vertices, sizeof(vx_vertex_t), pc->nvertices, fp);
    fclose(fp);

    auto [min_coords, max_coords] = calc_pc_aabb(pc);
    uint n_subdiv = calc_n_subdiv(n_subdiv_frac, min_coords, max_coords);
    uint side_length = 1 << (n_subdiv - n_subdiv_frac);

    min_coords = glm::vec3(-((float) side_length) / 2.f);
    max_coords = glm::vec3( ((float) side_length) / 2.f);

    std::vector<SvoNode> nodes;
    create_svo(pc, nodes, min_coords, max_coords, n_subdiv);

    vx_point_cloud_free(pc);

    std::printf("Created %zu nodes\n", nodes.size());

    SvoFileHeader svo_header {
        min_coords, max_coords,
        n_subdiv,
        (uint32_t) nodes.size()
    };

    FILE *fp_svo = fopen("suzanne-svo.bin", "wb");
    fwrite(&svo_header, sizeof(SvoFileHeader), 1, fp_svo);
    fwrite(nodes.data(), sizeof(SvoNode), nodes.size(), fp_svo);
    fclose(fp_svo);

    return 0;
}