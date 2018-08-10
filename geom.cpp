////////////////////////////////////////////////////////////////////////
//
// geom.cpp: Points, primitives, etc.
//
// Copyright (C) 2018 Simon Frankau
//

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include <iostream>
#include <cmath>
#include <map>
#include <vector>

#include "geom.h"

////////////////////////////////////////////////////////////////////////
// Vertex

Vertex::Vertex(double ix, double iy, double iz)
    : p {ix, iy, iz}
{
}

Vertex::Vertex(Vertex const &v)
    : p {v.x(), v.y(), v.z()}
{
}

double Vertex::len() const
{
    return std::sqrt(x() * x() + y() * y() + z() * z());
}

Vertex Vertex::norm() const
{
    double l = len();
    return Vertex(x() / l, y() / l, z() / l);
}

// Get an arbitrary perpendicular vector.
Vertex Vertex::perp() const
{
    // Choose the axis-aligned vector most orthogonal, and then
    // properly orthogonalise it.
    double ax = fabs(x()), ay = fabs(y()), az = fabs(z());
    if (ax < ay && ax < az) {
        return orthog(Vertex(1.0, 0.0, 0.0), *this);
    }
    if (ay < az) {
        return orthog(Vertex(0.0, 1.0, 0.0), *this);
    } else {
        return orthog(Vertex(0.0, 0.0, 1.0), *this);
    }
}

Vertex Vertex::scale(double s) const
{
    return Vertex(x() * s, y() * s, z() * s);
}

Vertex Vertex::operator+(Vertex const &rhs) const
{
    return Vertex(x() + rhs.x(), y() + rhs.y(), z() + rhs.z());
}

Vertex Vertex::operator-(Vertex const &rhs) const
{
    return Vertex(x() - rhs.x(), y() - rhs.y(), z() - rhs.z());
}

double Vertex::x() const { return p[0]; }
double Vertex::y() const { return p[1]; }
double Vertex::z() const { return p[2]; }

std::ostream &operator<<(std::ostream &os, Vertex const &v)
{
    return os << "(" << v.x() << ", " << v.y() << ", " << v.z() << ")";
}

// Cross product.
Vertex cross(Vertex const &v1, Vertex const &v2)
{
    return Vertex(v1.y() * v2.z() - v1.z() * v2.y(),
                  v1.z() * v2.x() - v1.x() * v2.z(),
                  v1.x() * v2.y() - v1.y() * v2.x());
}

// Dot product.
double dot(Vertex const &v1, Vertex const &v2)
{
    return v1.x() * v2.x()
         + v1.y() * v2.y()
         + v1.z() * v2.z();
}

// Orthogonalise v1, taking away the v2 component.
Vertex orthog(Vertex const &v1, Vertex const &v2)
{
    double c = dot(v1, v2) / dot(v2, v2);
    return v1 - v2.scale(c);
}

// Linear interpolation. 0 returns v1, 1 returns v2.
Vertex lerp(Vertex const &v1, Vertex const &v2, double i)
{
    double j = 1.0 - i;
    return Vertex(v1.x() * j + v2.x() * i,
                  v1.y() * j + v2.y() * i,
                  v1.z() * j + v2.z() * i);
}

////////////////////////////////////////////////////////////////////////
// Colour

Colour::Colour(double red, double green, double blue)
    : r(red), g(green), b(blue)
{
}

Colour::Colour(Colour const &c)
    : r(c.r), g(c.g), b(c.b)
{
}

Colour::Colour()
    : r(0.0), g(0.0), b(0.0)
{
}

Colour Colour::operator*(double x) const
{
    return Colour(r * x, g * x, b * x);
}

Colour Colour::operator*(Colour const &c) const
{
    return Colour(r * c.r, g * c.g, b * c.b);
}

Colour &Colour::operator+=(Colour const &c)
{
    r += c.r; g += c.g; b += c.b;
    return *this;
}

double Colour::asGrey() const
{
    return 0.2126 * r + 0.7152 * g + 0.0722 * b;
}

////////////////////////////////////////////////////////////////////////
// Quad

Quad::Quad(int v1, int v2, int v3, int v4, Colour const &c)
    : indices { v1, v2, v3, v4 }, isEmitter(false), materialColour(c)
{
}

void Quad::render(std::vector<Vertex> const &v) const
{
    Vertex const &v0 = v[indices[0]];
    Vertex const &v1 = v[indices[1]];
    Vertex const &v2 = v[indices[2]];
    Vertex const &v3 = v[indices[3]];
    Vertex n = cross(v3 - v0, v1 - v0).norm();
    glBegin(GL_QUADS);
    glColor3d(screenColour.r, screenColour.g, screenColour.b);
    glNormal3dv(n.p);
    glVertex3dv(v0.p);
    glVertex3dv(v1.p);
    glVertex3dv(v2.p);
    glVertex3dv(v3.p);
    glEnd();
}

void Quad::renderIndex(int index, std::vector<Vertex> const &v) const
{
    Vertex const &v0 = v[indices[0]];
    Vertex const &v1 = v[indices[1]];
    Vertex const &v2 = v[indices[2]];
    Vertex const &v3 = v[indices[3]];
    Vertex n = cross(v3 - v0, v1 - v0).norm();
    glBegin(GL_QUADS);
    // We're not using that many polys, so skip the low bits. This
    // means we can see what's going on better if we do a test render.
    glColor3ub((index << 2) & 0xFC, (index >> 4) & 0xFC, (index >> 10) & 0xFC);
    glNormal3dv(n.p);
    glVertex3dv(v0.p);
    glVertex3dv(v1.p);
    glVertex3dv(v2.p);
    glVertex3dv(v3.p);
    glEnd();
}

// Return the centre of the quad. Assumes paralellogram.
Vertex paraCentre(Quad const &q, std::vector<Vertex> const &vs)
{
    return lerp(vs[q.indices[0]], vs[q.indices[2]], 0.5);
}

// Return the vector for the cross product of the edges - this will
// have length proportional to area, and be normal to the quad.
// Also assumes parallelogram.
//
// NB: Vector faces away from viewer in how we do things here.
Vertex paraCross(Quad const &q, std::vector<Vertex> const &vs)
{
    Vertex const &v0 = vs[q.indices[0]];
    Vertex const &v1 = vs[q.indices[1]];
    Vertex const &v3 = vs[q.indices[3]];
    return cross(v3 - v0, v1 - v0);
}

// Find area of given parallelogram.
double paraArea(Quad const &q, std::vector<Vertex> const &vs)
{
    return paraCross(q, vs).len();
}

// Applies a transform to the requested vertices, with a cache.
class VertexTransformer
{
private:
    std::vector<Vertex> &m_vertices;

    // Cache of vertices scaled already.
    std::map<int, int> m_scaledVertices;

    virtual Vertex transform(Vertex const &v) const = 0;

public:
    VertexTransformer(std::vector<Vertex> &vertices)
        : m_vertices(vertices)
    {
    }

    int operator()(int i)
    {
        // Return cached result is present.
        std::map<int, int>::const_iterator iter = m_scaledVertices.find(i);
        if (iter != m_scaledVertices.end()) {
            return iter->second;
        }
        // Else add and return.
        int j = m_vertices.size();
        m_scaledVertices[i] = j;
        m_vertices.push_back(transform(m_vertices[i]));
        return j;
    }

    void transformAll(std::vector<Quad> &qs)
    {
        for (int i = 0, n = qs.size(); i < n; ++i) {
            Quad &q = qs[i];
            for (int j = 0; j < 4; ++j) {
                q.indices[j] = (*this)(q.indices[j]);
            }
        }
    }
};

class VertexTranslater : public VertexTransformer
{
private:
    Vertex const &m_offset;

    virtual Vertex transform(Vertex const &v) const
    {
        return v + m_offset;
    }

public:
    VertexTranslater(Vertex const &offset,
                     std::vector<Vertex> &vertices)
        : m_offset(offset), VertexTransformer(vertices)
    {
    }
};

// Translate the given quads, in-place.
void translate(Vertex const &t,
          std::vector<Quad> &qs,
          std::vector<Vertex> &vs)
{
    VertexTranslater(t, vs).transformAll(qs);;
}

class VertexScaler : public VertexTransformer
{
private:
    double m_scale;

    virtual Vertex transform(Vertex const &v) const
    {
        return v.scale(m_scale);
    }

public:
    VertexScaler(double scale,
                 std::vector<Vertex> &vertices)
        : m_scale(scale), VertexTransformer(vertices)
    {
    }
};

// Scale the given quads, in-place.
void scale(double s,
           std::vector<Quad> &qs,
           std::vector<Vertex> &vs)
{
    VertexScaler(s, vs).transformAll(qs);
}

class VertexRotater : public VertexTransformer
{
private:
    double m_angle;
    // m_axis, m_plane1 and m_plane 2 form an orthonormal basis, where
    // m_plane1 and m_plane2 are the plane of rotation.
    Vertex m_axis;
    Vertex m_plane1;
    Vertex m_plane2;

    virtual Vertex transform(Vertex const &v) const
    {
        // Decompase v to our rotation's basis.
        double x = dot(v, m_plane1);
        double y = dot(v, m_plane2);
        double z = dot(v, m_axis);
        // Perform the rotation in the x-y plane
        double c = cos(m_angle);
        double s = sin(m_angle);
        double x2 =  c * x + s * y;
        double y2 = -s * x + c * y;
        // And construct a vector in the original basis.
        return m_plane1.scale(x2) +
            m_plane2.scale(y2) +
            m_axis.scale(z);
    }

public:
    VertexRotater(Vertex const &axis,
                  double angle,
                  std::vector<Vertex> &vertices)
        : m_angle(angle),
          m_axis(axis.norm()),
          m_plane1(m_axis.perp().norm()),
          m_plane2(cross(m_axis, m_plane1)),
          VertexTransformer(vertices)
    {
    }
};

// Rotate the given quads, in-place.
void rotate(Vertex const &axis,
            double angle,
            std::vector<Quad> &qs,
            std::vector<Vertex> &vs)
{
    VertexRotater(axis, angle, vs).transformAll(qs);
}

// Flip the facing direction of the quads.
void flip(std::vector<Quad> &qs,
          std::vector<Vertex> &vs)
{
    for (int i = 0, n = qs.size(); i < n; ++i) {
        Quad &q = qs[i];
        std::swap(q.indices[1], q.indices[3]);
    }
}

////////////////////////////////////////////////////////////////////////
// Gouraud-shaded quad, used only for final rendering.

GouraudQuad::GouraudQuad(int v1, int v2, int v3, int v4,
                         Colour c1, Colour c2, Colour c3, Colour c4)
    : m_indices { v1, v2, v3, v4 },
      m_colours { c1, c2, c3, c4 }
{
}

void GouraudQuad::render(std::vector<Vertex> const &v) const
{
    Vertex const &v0 = v[m_indices[0]];
    Vertex const &v1 = v[m_indices[1]];
    Vertex const &v2 = v[m_indices[2]];
    Vertex const &v3 = v[m_indices[3]];
    Vertex n = cross(v3 - v0, v1 - v0).norm();
    glBegin(GL_QUADS);
    glNormal3dv(n.p);
    glColor3d(m_colours[0].r, m_colours[0].g, m_colours[0].b);
    glVertex3dv(v0.p);
    glColor3d(m_colours[1].r, m_colours[1].g, m_colours[1].b);
    glVertex3dv(v1.p);
    glColor3d(m_colours[2].r, m_colours[2].g, m_colours[2].b);
    glVertex3dv(v2.p);
    glColor3d(m_colours[3].r, m_colours[3].g, m_colours[3].b);
    glVertex3dv(v3.p);
    glEnd();
}

////////////////////////////////////////////////////////////////////////
// Subdivision.

// Break apart the given quad into a bunch of quads, add them to "qs",
// and add the new vertices to "vs".
SubdivInfo subdivide(Quad const &quad,
                     std::vector<Vertex> &vs,
                     std::vector<Quad> &qs,
                     int uCount, int vCount)
{
    int const vertexStart = vs.size();
    Vertex v0 = vs[quad.indices[0]];
    Vertex v1 = vs[quad.indices[1]];
    Vertex v2 = vs[quad.indices[2]];
    Vertex v3 = vs[quad.indices[3]];

    // Generate the grid of points we will build the quads from.
    for (int v = 0; v < vCount + 1; ++v) {
        for (int u = 0; u < uCount + 1; ++u) {
            Vertex u0 = lerp(v0, v1, static_cast<double>(u) / uCount);
            Vertex u1 = lerp(v3, v2, static_cast<double>(u) / uCount);
            Vertex pt = lerp(u0, u1, static_cast<double>(v) / vCount);
            vs.push_back(pt);
        }
    }

    // Build the corners of the quads.
    int const faceStart = qs.size();
    for (int v = 0; v < vCount; ++v) {
        for (int u = 0; u < uCount; ++u) {
            int base = vertexStart + v * (uCount + 1) + u;
            qs.push_back(Quad(base, base + 1,
                              base + uCount + 2, base + uCount + 1,
                              quad.materialColour));
            qs.back().isEmitter = quad.isEmitter;
        }
    }

    return SubdivInfo(uCount, vCount, vertexStart, faceStart);
}


SubdivInfo::SubdivInfo(int uCount, int vCount, int vertexStart, int faceStart)
    : m_uCount(uCount), m_vCount(vCount),
      m_vertexStart(vertexStart), m_faceStart(faceStart)
{
}

void SubdivInfo::generateGouraudQuads(std::vector<Vertex> const &vs,
                                      std::vector<Quad> const &qsIn,
                                      std::vector<GouraudQuad> &qsOut)
{
// NYI
/* Using:
    int m_uCount;
    int m_vCount;
    int m_vertexStart;
    int m_faceStart;
*/
}

////////////////////////////////////////////////////////////////////////
// Basic shapes.

// Colour of the walls, etc.
static Colour const C = Colour(0.9, 0.9, 0.9);

std::vector<Quad> const cubeFaces = {
    Quad(1, 0, 2, 3, C), Quad(3, 2, 6, 7, C), Quad(7, 6, 4, 5, C),
    Quad(5, 4, 0, 1, C), Quad(4, 6, 2, 0, C), Quad(7, 5, 1, 3, C)
};

std::vector<Vertex> const cubeVertices = {
    Vertex(-1, -1, -1),
    Vertex(-1, -1, +1),
    Vertex(-1, +1, -1),
    Vertex(-1, +1, +1),
    Vertex(+1, -1, -1),
    Vertex(+1, -1, +1),
    Vertex(+1, +1, -1),
    Vertex(+1, +1, +1),
};
