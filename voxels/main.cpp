#include <iostream>
#include <vector>

#include <glm/glm.hpp>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

struct Voxel {
    glm::vec3 pos;
    glm::vec3 color;
};

bool ray_voxel(const glm::vec3 &start, const glm::vec3 &dir, const glm::vec3 &voxel) {
    using std::min, std::max;

    auto vmin = voxel - .5f;
    auto vmax = voxel + .5f;

    glm::vec3 inv = 1.f / dir;

    float t1 = (vmin.x - start.x)*inv.x,
          t2 = (vmax.x - start.x)*inv.x,
          t3 = (vmin.y - start.y)*inv.y,
          t4 = (vmax.y - start.y)*inv.y,
          t5 = (vmin.z - start.z)*inv.z,
          t6 = (vmax.z - start.z)*inv.z;

    float tmin = max(max(min(t1, t2), min(t3, t4)), min(t5, t6));
    float tmax = min(min(max(t1, t2), max(t3, t4)), max(t5, t6));

    if (tmax < 0) return false;
    if (tmin > tmax) return false;
    return true;
}

glm::vec3 cast_ray(const glm::vec3 &camera_pos, const glm::vec3 &direction) {
    std::vector<Voxel> voxels {
            { {0, 0, -10}, {0.2,0.5,0.1} },
            { {1, 2, -5}, {0.3,0.6,0.4} },
            { {2, 2, -5}, {0.1,0.4,0.8} },
    };

    for (const auto &v : voxels)
        if (ray_voxel(camera_pos, direction, v.pos)) {
            return v.color;
        }

    return glm::vec3(0);
}

int main() {
    constexpr int width = 1024;
    constexpr int height = 768;
    constexpr float fov = M_PI_2;
    constexpr glm::vec3 camera_pos(0);

    std::cout << "Hello, World!" << std::endl;

    std::vector<std::uint8_t> framebuffer;
    framebuffer.resize(width * height * 3);

#pragma omp parallel for
    for (int pixel = 0; pixel < width * height; pixel++) {
        float dx =  (pixel%width + 0.5) -  width/2.;
        float dy = -(pixel/width + 0.5) + height/2.;
        float dz = -height/(2.*std::tan(fov/2.));

        glm::vec3 direction(dx, dy, dz);
        direction = glm::normalize(direction);
        glm::vec3 color = cast_ray(camera_pos, direction);
        framebuffer[pixel*3+0] = 256 * color.r;
        framebuffer[pixel*3+1] = 256 * color.g;
        framebuffer[pixel*3+2] = 256 * color.b;
    }

    stbi_write_png("out.png", width, height, 3, framebuffer.data(), width * 3);
    return 0;
}
