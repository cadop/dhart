///
///	\file		cost_algorithms.h
///	\brief		Contains implementation for the <see cref="HF::SpatialStructures::CostAlgorithms">HF::SpatialStructures::CostAlgorithms</cref> namespace
///
///	\author		TBA
///	\date		03 Jul 2020
///

#define _USE_MATH_DEFINES
#include <cmath>
#include <algorithm>

#include "cost_algorithms.h"
#include "graph.h"
#include "Constants.h"

#include <iostream>

using HF::SpatialStructures::Node;
using HF::SpatialStructures::IntEdge;
using HF::SpatialStructures::Graph;
using HF::SpatialStructures::EdgeSet;

bool HF::SpatialStructures::CostAlgorithms::is_floating_type(std::string value) {
    char* ptr = nullptr;
    std::strtof(value.c_str(), &ptr);
    return *ptr == '\0';
}

double HF::SpatialStructures::CostAlgorithms::to_radians(double degrees) {
    return degrees * (M_PI / 180);
}

double HF::SpatialStructures::CostAlgorithms::to_degrees(double radians) {
    return radians * (180 / M_PI);
}

float HF::SpatialStructures::CostAlgorithms::calculate_magnitude(std::array<float, 3> dir_a, std::array<float, 3> dir_b) {
    return std::sqrtf(std::pow(dir_a[0] - dir_b[0], 2.0) + std::pow(dir_a[1] - dir_b[1], 2.0) + std::pow(dir_a[2] - dir_b[2], 2.0));
}

float HF::SpatialStructures::CostAlgorithms::calculate_dot_product(std::array<float, 3> dir_a, std::array<float, 3> dir_b) {
    return (dir_a[0] * dir_b[0]) + (dir_a[1] * dir_b[1]) + (dir_a[2] * dir_b[2]);
}

bool HF::SpatialStructures::CostAlgorithms::is_perpendicular(std::array<float, 3> dir_a, std::array<float, 3> dir_b) {
    float dot_product = calculate_dot_product(dir_a, dir_b);

    // Mathematically,
    // two vectors are perpendicular if their dot product is
    // equal to zero. But since it is a mortal sin to test
    // floating point numbers for equality (using operator==) --
    // we can test if the dot product of
    // vector_a and vector_b is 'close enough' to zero,
    // by determining if our dot_product calculation
    // is less than our ROUNDING_PRECISION constant.
    // (which is 0.0001)    
    return std::abs(dot_product) < HF::SpatialStructures::ROUNDING_PRECISION;
}
