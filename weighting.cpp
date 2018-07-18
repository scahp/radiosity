////////////////////////////////////////////////////////////////////////
//
// weighting.cpp: Weightings to apply to a rendered image to get
// how much light comes from which point.
//
// Copyright (c) Simon Frankaus 2018
//

#include <math.h>

#include "geom.h"

// Generate a weightings array based on projecting rectangles onto a
// sphere. Assumes 90 degree field of view.
//
// TODO: This doesn't include cos(angle from normal) factor.
void projWeights(int resolution, std::vector<GLdouble> &weights)
{
    // From -1 to +1.
    GLdouble conv = 2.0 / resolution;
    // Make total come to 1.
    GLdouble weight = 3.0 / (M_PI * 2.0);

    for (int y = 0; y < resolution; ++y) {
        for (int x = 0; x < resolution; ++x) {
            Vertex v1 = Vertex(x       * conv - 1, y       * conv - 1, 1);
            Vertex v2 = Vertex((x + 1) * conv - 1, y       * conv - 1, 1);
            Vertex v3 = Vertex(x       * conv - 1, (y + 1) * conv - 1, 1);
            v1 = v1.norm(); v2 = v2.norm(); v3 = v3.norm();
            weights.push_back(weight * cross(v3 - v1, v2 - v1).len());
        }
    }
}

// Like projWeights, but axis-aligns things first.
//
// First step towards doing an analytic version.
void projWeights2(int resolution, std::vector<GLdouble> &weights)
{
    // From -1 to +1.
    GLdouble conv = 2.0 / resolution;
    // Make total come to 1.
    GLdouble weight = 3.0 / (M_PI * 2.0);

    for (int y = 0; y < resolution; ++y) {
        for (int x = 0; x < resolution; ++x) {
            GLdouble px = (x + 0.5) * conv - 1.0;
            GLdouble py = (y + 0.5) * conv - 1.0;
            double dist = sqrt(px * px + py * py);

            Vertex v1 = Vertex(dist - conv/2, 0, 1);
            Vertex v2 = Vertex(dist + conv/2, 0, 1);
            // When projecting onto sphere, the tangential component is scaled by 1/r.
            GLdouble origLen = v1.len();
            v1 = v1.norm(); v2 = v2.norm();
            weights.push_back(weight * conv * (v2 - v1).len() / origLen);
        }
    }
}

const int WEIGHT_RESOLUTION = 512;

// Quick test that the integral comes out right.
//
// TODO: I should really get some cpp_unit, shouldn't I?
int main(void)
{
    std::vector<GLdouble> ws, ws2;
    projWeights(WEIGHT_RESOLUTION, ws);
    projWeights2(WEIGHT_RESOLUTION, ws2);
    GLdouble total = 0.0, total2 = 0.0, maxError = 0.0;
    for (int i = 0, n = ws.size(); i < n; ++i) {
        // if (i % WEIGHT_RESOLUTION == 0) std::cout << std::endl;
        // std::cout << ws[i] << " vs " << ws2[i] << "(" << ws[i] / ws2[i] << ")" << std::endl;
        total += ws[i];
        total2 += ws2[i];
        maxError = fmax(maxError, fabs(1 - ws[i] / ws2[i]));
    }
    std::cout << "Should be about 1.0: " << total << ", " << total2 << std::endl;
    std::cout << "Max error: " << maxError << std::endl;
}
