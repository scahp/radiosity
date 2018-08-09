////////////////////////////////////////////////////////////////////////
//
// geom.h: Points, primitives, etc.
//
// Copyright (C) 2018 Simon Frankau
//

#ifndef RADIOSITY_GEOM_H
#define RADIOSITY_GEOM_H

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include <iostream>
#include <vector>

////////////////////////////////////////////////////////////////////////
// Yet another 3d point class

class Vertex
{
public:
    Vertex(double ix, double iy, double iz);
    Vertex(Vertex const &v);

    double len() const;
    Vertex norm() const;
    Vertex perp() const;
    Vertex scale(double s) const;

    Vertex operator+(Vertex const &rhs) const;
    Vertex operator-(Vertex const &rhs) const;

    double x() const;
    double y() const;
    double z() const;

    double p[3];
};

std::ostream &operator<<(std::ostream &os, Vertex const &v);

// Cross product.
Vertex cross(Vertex const &v1, Vertex const &v2);

// Dot product.
double dot(Vertex const &v1, Vertex const &v2);

// Orthogonalise v1, taking away the v2 component.
Vertex  orthog(Vertex const &v1, Vertex const &v2);

// Linear interpolation. 0 returns v1, 1 returns v2.
Vertex lerp(Vertex const &v1, Vertex const &v2, double i);

////////////////////////////////////////////////////////////////////////
// And a quadrilateral

class Quad
{
public:
    Quad(GLint v1, GLint v2,
         GLint v3, GLint v4,
         double l);

    void render(std::vector<Vertex> const &v) const;
    // For transfer calculations.
    void renderIndex(int index, std::vector<Vertex> const &v) const;

    GLint indices[4];
    // Does this quad emit light, or just reflect?
    bool isEmitter;
    // How much light it emits, or the fraction reflected.
    double light;
    // On-screen brightness.
    double brightness;
};

// Return the centre of the quad. Assumes paralellogram.
Vertex paraCentre(Quad const &q, std::vector<Vertex> const &vs);

// Return the vector for the cross product of the edges - this will
// have length proportional to area, and be normal to the quad.
// Also assumes parallelogram.
Vertex paraCross(Quad const &q, std::vector<Vertex> const &vs);

// Find area of given parallelogram.
double paraArea(Quad const &q, std::vector<Vertex> const &vs);

// Break apart the given quad into a bunch of quads, add them to "qs",
// and add the new vertices to "vs".
void subdivide(Quad const &quad,
               std::vector<Vertex> &vs,
               std::vector<Quad> &qs,
               GLint uCount, GLint vCount);

// Translate the given quads, in-place
void translate(Vertex const &t,
           std::vector<Quad> &qs,
           std::vector<Vertex> &vs);

// Scale the given quads, in-place.
void scale(double s,
           std::vector<Quad> &qs,
           std::vector<Vertex> &vs);

// Rotate the given quads, in-place.
void rotate(Vertex const &axis,
            double angle,
            std::vector<Quad> &qs,
            std::vector<Vertex> &vs);

// Flip the facing direction of the given quads, in-place.
void flip(std::vector<Quad> &qs,
          std::vector<Vertex> &vs);

////////////////////////////////////////////////////////////////////////
// Basic shapes.

extern std::vector<Vertex> const cubeVertices;
extern std::vector<Quad>   const cubeFaces;

#endif // RADIOSITY_GEOM_H
