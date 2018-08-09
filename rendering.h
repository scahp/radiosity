////////////////////////////////////////////////////////////////////////
//
// rendering.h: Put the calculated polys on the screen
//
// Copyright (c) Simon Frankau 2018
//

#ifndef RADIOSITY_RENDERING_H
#define RADIOSITY_RENDERING_H

#include "geom.h"

// Render the scene
void render(std::vector<Quad> f, std::vector<Vertex> v);

#endif // RADIOSITY_RENDERING_H
