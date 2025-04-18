#pragma once

#include <limits>

// Constants

const double infinity = std::numeric_limits<double>::infinity();
const double pi = 3.1415926535897932385;

// Utility Functions

double degrees_to_radians(double degrees);

// Returns a random real in [0,1).
double random_double();

// Returns a random real in [min,max).
double random_double(double min, double max);