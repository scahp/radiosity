////////////////////////////////////////////////////////////////////////
//
// cube.cpp: Calculate radiosity inside a cube.
//
// Copyright (c) Simon Frankau 2018
//

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include <iostream>
#include <cmath>
#include <vector>

#include "geom.h"
#include "glut_wrap.h"
#include "rendering.h"
#include "transfers.h"

// Relative change in total light in the scene by the point we stop
// iterating.
const double CONVERGENCE_TARGET = 0.001;

// Break up each base quad into subdivision^2 subquads for radiosity
// calculations.
int const SUBDIVISION = 32;

////////////////////////////////////////////////////////////////////////
// Radiosity calculations

void initLighting(std::vector<Quad> &qs, std::vector<Vertex> const &vs)
{
    for (std::vector<Quad>::iterator iter = qs.begin(), end = qs.end();
         iter != end; ++iter) {
        Vertex c = paraCentre(*iter, vs);
        // Put a big light in the top centre of the box.
        if (fabs(c.x()) < 0.5 && fabs(c.z()) < 0.5 & c.y() > 0.9) {
            iter->materialColour = iter->screenColour = Colour(2.0, 2.0, 2.0);
            iter->isEmitter = true;
        }
        // Make the left wall red, the right wall blue.
        if (c.x() < -0.999) {
            iter->materialColour = iter->materialColour * Colour(1.0, 0.5, 0.5);
        } else if (c.x() > 0.999) {
            iter->materialColour = iter->materialColour * Colour(0.5, 0.5, 1.0);
        }
    }
}

void iterateLighting(std::vector<Quad> &qs, std::vector<double> const &transfers)
{
    int const n = qs.size();
    std::vector<Colour> updatedColours(n);

    // Iterate over targets
    for (int i = 0; i < n; ++i) {
        Colour incoming;
        if (qs[i].isEmitter) {
            // Emission is just like having 1.0 light arrive.
            incoming = Colour(1.0, 1.0, 1.0);
        } else {
            // Iterate over sources
            for (int j = 0; j < n; ++j) {
                if (i == j) {
                    continue;
                }
                incoming += qs[j].screenColour * transfers[i * n + j];
            }
        }
        updatedColours[i] = incoming * qs[i].materialColour;
    }

    for (int i = 0; i < n; ++i) {
        qs[i].screenColour = updatedColours[i];
    }
}

// Calculate the total light in the scene, as area-weight sum of
// screenColour.
double calcLight(std::vector<Quad> &qs, std::vector<Vertex> const &vs)
{
    double totalLight = 0.0;
    for (std::vector<Quad>::iterator iter = qs.begin(), end = qs.end();
         iter != end; ++iter) {
        totalLight += iter->screenColour.asGrey() * paraArea(*iter, vs);
    }
    return totalLight;
}

////////////////////////////////////////////////////////////////////////
// And the main rendering bit...

// Geometry.
static std::vector<Quad> faces;
static std::vector<Vertex> vertices;
// Array of quad-to-quad light transfers.
static std::vector<double> transfers;
// And data for generating Gouraud shading.
static std::vector<SubdivInfo> subdivs;

// Subdivide the faces
void initGeometry(void)
{
    vertices = cubeVertices;
    // Draw the outer 'scene' cube, by subdividing the prototype.
    for (std::vector<Quad>::const_iterator iter = cubeFaces.begin(),
             end = cubeFaces.end(); iter != end; ++iter) {
        subdivs.push_back(subdivide(*iter, vertices, faces,
                                    SUBDIVISION, SUBDIVISION));
    }

    // Then draw the inner cube: Take the basic scene cube, scale it
    // down, rotate and move it...
    std::vector<Quad> sceneFaces(cubeFaces); // Enclosed cube
    scale(0.4, sceneFaces, vertices);
    flip(sceneFaces, vertices);
    rotate(Vertex(1.0, 0.0, 0.0), M_PI / 3.0, sceneFaces, vertices);
    rotate(Vertex(0.0, 0.0, 1.0), M_PI / 6.0, sceneFaces, vertices);
    translate(Vertex(0.0, -0.25, 0.0), sceneFaces, vertices);
    // Copy the subdivided version into 'faces' (lower subdivisions,
    // as smaller).
    for (std::vector<Quad>::const_iterator iter = sceneFaces.begin(),
             end = sceneFaces.end(); iter != end; ++iter) {
        subdivs.push_back(subdivide(*iter, vertices, faces,
                                    SUBDIVISION / 2, SUBDIVISION / 2));
    }
}

int main(int argc, char **argv)
{
    glutInit(&argc, argv);

    initGeometry();
    initLighting(faces, vertices);
    RenderTransferCalculator(vertices, faces, 256).calcAllLights(transfers);
    double light = 0.0;
    double relChange;
    do {
        iterateLighting(faces, transfers);
        double newLight = calcLight(faces, vertices);
        relChange = fabs(light / newLight - 1.0);
        light = newLight;
        std::cout << "Total light: " << light << std::endl;
    } while (relChange > CONVERGENCE_TARGET);

    normaliseBrightness(faces, vertices);
    std::vector<Vertex> gVertices;
    std::vector<GouraudQuad> gourauds;
    for (std::vector<SubdivInfo>::iterator iter = subdivs.begin(),
             end = subdivs.end(); iter != end; ++iter) {
        iter->generateGouraudQuads(gourauds, gVertices);
    }
    renderGouraud(gourauds, gVertices);
    return 0;
}
