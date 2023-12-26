#include <iostream>
#include <vector>

#include <glm/glm.hpp>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

int main() {
    constexpr int width = 1024;
    constexpr int height = 768;

    std::cout << "Hello, World!" << std::endl;

    std::vector<glm::vec3> framebuffer;
    framebuffer.resize(width * height);

    stbi_write_png("out.png", width, height, 3, framebuffer.data(), width * sizeof(glm::vec3));
    return 0;
}
