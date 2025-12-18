#pragma once
#include <stdexcept>
#include <string>

namespace mygpx {

inline void validateLatitude(double latitude) {

    if (latitude < -90.0 || latitude > 90.0)
        throw std::out_of_range("Latitude must be between -90 and 90 degrees.");
    return;

}

inline void validateLongitude(double longitude) {

    if (longitude < -180.0 || longitude > 180.0)
        throw std::out_of_range("Longitude must be between -180 and 180 degrees.");
    return;

}

inline void validateIndex(size_t index, size_t maxSize) {

    if (index < 0 || index >= maxSize)
        throw std::out_of_range("Index is out of range.");
    return;

}

inline void validateInRange(size_t index, size_t min_arg, size_t max_arg = SIZE_MAX, std::string message = "") {

    if (index > max_arg)
        throw std::invalid_argument("Input: " + std::to_string(index) +
            " is greater than maximal limit: " + std::to_string(max_arg) + ".");

    if (index < min_arg)
        throw std::invalid_argument("Input: " + std::to_string(index) +
            " is less than minimal limit: " + std::to_string(min_arg) + ".");

    return;

}

inline void validateInRange(double value, double min_arg, double max_arg, std::string message = "") {

    if (value > max_arg)
        throw std::invalid_argument("Input: " + std::to_string(value) +
            " is greater than maximal limit: " + std::to_string(max_arg) + ".");

    if (value < min_arg)
        throw std::invalid_argument("Input: " + std::to_string(value) +
            " is less than minimal limit: " + std::to_string(min_arg) + ".");

    return;

}

}