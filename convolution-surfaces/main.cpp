// (c) 2021 Andreas Klein
// This code is licensed under MIT license (see LICENSE for details)

#include <memory>

#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <iostream>
namespace constants
{
    // Width and height of the output image
    const std::int32_t Width = 512;
    const std::int32_t Height = 512;
    // Camera frame
    const glm::vec3 Eye(0, 0., 3.0f);
    const glm::vec3 Target(0.0f, 0., 0.0f);
    const glm::vec3 Up(0, 1, 0);
    const float FOV = glm::radians(45.0f);
    // Minimum and maximum distance for first interval
    const float tMin = 0.0f;
    const float tMax = 10.0f;
    // Ray marching step size
    const float StepSize = 0.01f;

    // Scaling factor for the fall-off functions
    const float S = 1.4;
    const float SurfaceThreshold = 0.3f;

    const glm::vec3 BackgroundColor(0.98, 0.98, 0.98);

} // namespace constants

namespace globals
{
    glm::vec3 CamDir(0.0f);
    glm::vec3 CamUp(0.0f);
    glm::vec3 CamRight(0.0f);
} // namespace globals

struct Ray
{
    glm::vec3 org;
    glm::vec3 dir;
};

float ToNDC(float val, float maxVal)
{
    return 2.0f * (val + 0.5f) / maxVal - 1.0f;
}

void InitCamera()
{
    globals::CamDir = glm::normalize(constants::Target - constants::Eye);
    // Gram-Schmidt orthogonalization
    globals::CamUp = glm::normalize(constants::Up - glm::dot(constants::Up, globals::CamDir) * globals::CamDir);
    globals::CamRight = glm::cross(globals::CamDir, globals::CamUp);
}

Ray CreateRay(float x, float y)
{
    float aspect = constants::Width / (float)constants::Height;
    float pixelWidth = std::tan(constants::FOV * 0.5f) * aspect;
    float pixelHeight = std::tan(constants::FOV * 0.5f);
    glm::vec3 d =
        glm::normalize(globals::CamRight * pixelWidth * x + globals::CamUp * pixelHeight * y + globals::CamDir);
    return Ray{constants::Eye, d};
}

float point(const glm::vec3 &p)
{
    float denom = (1.0f + constants::S * constants::S * glm::length(p));
    return 1.0f / (denom * denom);
}
float f(const glm::vec3 &p)
{
    return point(p + glm::vec3(0, 0.5, 0)) + point(p - glm::vec3(0, 0.5, 0));
}
glm::vec3 normal(const glm::vec3 &p)
{
    const float eps = 0.001f;
    float x = f(p) - f(p + glm::vec3(eps, 0, 0));
    float y = f(p) - f(p + glm::vec3(0, eps, 0));
    float z = f(p) - f(p + glm::vec3(0, 0, eps));
    return glm::normalize(glm::vec3(x, y, z));
}

float RayMarch(const Ray &ray)
{
    for (float t = constants::tMin; t < constants::tMax;)
    {
        glm::vec3 p = ray.org + t * ray.dir;
        float dist = f(p);
        if (dist > constants::SurfaceThreshold)
        {
            return t;
        }
        t += constants::StepSize;
    }
    return std::numeric_limits<float>::max();
}

int main(int argc, char **argv)
{
    InitCamera();
    std::unique_ptr<glm::u8vec3[]> image(new glm::u8vec3[constants::Width * constants::Height]);
    for (std::int32_t x = 0; x < constants::Width; ++x)
    {
        for (std::int32_t y = 0; y < constants::Height; ++y)
        {
            // create a ray through the pixel at (x,y)
            auto ray = CreateRay(ToNDC(x, constants::Width), ToNDC(y, constants::Height));
            glm::vec3 color = constants::BackgroundColor;

            float distance = RayMarch(ray);
            if (distance < constants::tMax)
            {
                float ndotl = glm::dot(normal(ray.org + distance * ray.dir), -ray.dir);
                color = glm::vec3(ndotl, 0.0f, 0.0f);
            }
            // image origin is on the lower left corner, invert y
            image[x + (constants::Height - 1 - y) * constants::Width] =
                glm::u8vec3(glm::clamp(color, 0.0f, 1.0f) * 255.0f);
        }
    }
    stbi_write_png("result.png", constants::Width, constants::Height, 3, glm::value_ptr(image[0]),
                   std::uint32_t(sizeof(glm::u8vec3) * constants::Width));
    return 0;
}