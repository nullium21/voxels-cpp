//
// Created by lina on 26.12.23.
//

#include <cstdio>
#include <vector>
#include <optional>
#include <iostream>

#include "glm/glm.hpp"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#define VOXELIZER_IMPLEMENTATION
#include "voxelizer.h"

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

int main() {
    constexpr float resolution = 8;

    auto *pc = voxelize_mesh("suzanne.obj", glm::vec3(1.f / resolution), (1.f / resolution) / 8);
    if (!pc) return 1;

    struct PointCloudFileHeader {
        glm::vec3 min_coords, max_coords;
        uint32_t n_points;
    } header = {
            glm::vec3(std::numeric_limits<float>::max()),
            glm::vec3(std::numeric_limits<float>::min()),
            (uint32_t) pc->nvertices
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

    vx_point_cloud_free(pc);

    return 0;
}