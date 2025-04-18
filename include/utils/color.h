#pragma once

#include <ostream>

#include "utils/vec3.h"

// Color type alias
using color = vec3;

// Write a pixel of color to the output stream
void write_color(std::ostream &os, const color &pixel_color);