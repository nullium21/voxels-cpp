#include <iostream>
#include <vector>

#include <svogen.h>
#include <glm/glm.hpp>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

struct Voxel {
    glm::vec3 pos;
    glm::vec3 color;
};

bool ray_voxel(const glm::vec3 &start, const glm::vec3 &inv, const glm::vec3 &voxel, float side = 1.f / 8) {
    using std::min, std::max;

    float t1 = (voxel.x - side - start.x) * inv.x,
          t2 = (voxel.x + side - start.x) * inv.x,
          t3 = (voxel.y - side - start.y) * inv.y,
          t4 = (voxel.y + side - start.y) * inv.y,
          t5 = (voxel.z - side - start.z) * inv.z,
          t6 = (voxel.z + side - start.z) * inv.z;

    float tmin = max(max(min(t1, t2), min(t3, t4)), min(t5, t6));
    float tmax = min(min(max(t1, t2), max(t3, t4)), max(t5, t6));

    return tmax > 0 && tmin <= tmax;
}

glm::vec3 cast_ray(const std::vector<SvoNode> &svo, uint i, glm::vec3 pos, const glm::vec3 &cam, const glm::vec3 &dir, float side_length) {
    const SvoNode &node = svo[i];

    if (!node.is_filled) {
        return {};
    }

    if (!ray_voxel(cam, 1.f / dir, pos, side_length * 2.f)) {
        return {};
    }

    if (!node.first_child_offset) {
        return pos; // todo: color
    }

    uint j = i + node.first_child_offset;
    do {
        const SvoNode &child = svo[j];

        glm::bvec3 negative(child.sign_x, child.sign_y, child.sign_z);
        glm::vec3 cpos = (glm::vec3(negative) * -1.f) * (side_length / 2.f) + pos;

        auto color = cast_ray(svo, j, cpos, cam, dir, side_length / 2.f);
        if (glm::length(color) > 0) return color;

        j += child.next_sibling_offset;
    } while (svo[j].next_sibling_offset);

    return {};
}

int main() {
    constexpr int width = 1024;
    constexpr int height = 768;
    constexpr float fov = M_PI_2;
    constexpr glm::vec3 camera_pos(-2, -2, 2);

    std::cout << "Hello, World!" << std::endl;

    std::vector<std::uint8_t> framebuffer;
    framebuffer.resize(width * height * 3);

    SvoFileHeader pch;
    FILE *fp = fopen("../svogen/suzanne-svo.bin", "rb");
    fread(&pch, sizeof(SvoFileHeader), 1, fp);

    std::vector<SvoNode> voxels;
    voxels.resize(pch.n_nodes);
    fread(voxels.data(), sizeof(SvoNode), pch.n_nodes, fp);

    fclose(fp);

#pragma omp parallel for
    for (int pixel = 0; pixel < width * height; pixel++) {
        float dx =  (pixel%width + 0.5) -  width/2.;
        float dy = -(pixel/width + 0.5) + height/2.;
        float dz = -height/(2.*std::tan(fov/2.));

        glm::vec3 direction(dx, dy, dz);
        direction = glm::normalize(direction);
        glm::vec3 color = cast_ray(voxels, 0, glm::vec3(0), camera_pos, direction, pch.max_coords.x * 2.f);
        framebuffer[pixel*3+0] = 256 * color.r;
        framebuffer[pixel*3+1] = 256 * color.g;
        framebuffer[pixel*3+2] = 256 * color.b;
    }

    stbi_write_png("out.png", width, height, 3, framebuffer.data(), width * 3);
    return 0;
}
