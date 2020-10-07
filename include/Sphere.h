//
// Created by alex on 11/1/19.
//

#ifndef SPHERE_VIS_SPHERE_H
#define SPHERE_VIS_SPHERE_H

#include <vector>
#include <cmath>

struct Sphere
{
    std::vector<float> vertices;
    std::vector<int> indices;
};

Sphere generateSphere(float radius, int ringCount, int sectorCount)
{
    Sphere sphere;

    sphere.vertices.reserve(3 * ringCount * sectorCount);

    float ringStep = 1.f / (float(ringCount) - 1.f);
    float sectorStep = 1.f / (float(sectorCount) - 1.f);

    sphere.vertices.push_back(0.f);
    sphere.vertices.push_back(-radius);
    sphere.vertices.push_back(0.f);

    for(int r = 1; r < ringCount - 1; r++)
    {
        for(int s = 0; s < sectorCount - 1; s++)
        {
            float x = radius *
                    cosf(2.f * float(M_PI) * float(s) * sectorStep) *
                    sinf(float(M_PI) * float(r) * ringStep);
            float y = radius * sinf(float(-M_PI) / 2.f + float(M_PI) * float(r) * ringStep);
            float z = radius *
                    sinf(2.f * float(M_PI) * float(s) * sectorStep) *
                    sinf(float(M_PI) * float(r) * sectorStep);

            sphere.vertices.push_back(x);
            sphere.vertices.push_back(y);
            sphere.vertices.push_back(z);
        }
    }

    sphere.vertices.push_back(0.f);
    sphere.vertices.push_back(radius);
    sphere.vertices.push_back(0.f);

    sphere.indices.reserve(ringCount * sectorCount * 4);

    for(int s = 0; s < sectorCount - 1; s++)
    {
        sphere.indices.push_back(0);
        sphere.indices.push_back(s + 2);
        sphere.indices.push_back(s + 1);
    }

    for(int r = 1; r < ringCount - 2; r++)
    {
        for(int s = 0; s < sectorCount - 1; s++)
        {
            sphere.indices.push_back((r - 1) * (sectorCount - 1) + s + 1);
            sphere.indices.push_back((r - 1) * (sectorCount - 1) + s + 2);
            sphere.indices.push_back(r * (sectorCount - 1) + s + 2);
            sphere.indices.push_back(r * (sectorCount - 1) + s + 1);
        }
    }

    for(int s = 0; s < sectorCount - 1; s++)
    {
        sphere.indices.push_back((ringCount - 3) * (sectorCount - 1) + s + 1);
        sphere.indices.push_back((ringCount - 3) * (sectorCount - 1) + s + 2);
        sphere.indices.push_back((ringCount - 2) * (sectorCount - 1) + 1);
    }

    return sphere;
}

#endif //SPHERE_VIS_SPHERE_H
