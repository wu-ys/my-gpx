#pragma once
#include "config.hpp"
#include "utils.hpp"
#include "validation.hpp"
#include <array>
#include <string>
#include <vector>

namespace mygpx {

    struct GeoHashDecoded {
        GeoPoint center;   // lat, lon
        GeoBBox bbox;      // lat_min, lat_max, lon_min, lon_max
        GeoPoint error;    // lat_error, lon_error (half-size)
        int precision;
    };

    std::string encode_geohash(double latitude, double longitude, int precision);
    std::string encode_geohash(GeoPoint gp, int precision);

    GeoPoint decode_geohash(const std::string& geohash);
    GeoBBox decode_geohash_bbox(const std::string& geohash);
    GeoHashDecoded decode_geohash_info(const std::string& geohash);

    // GeoPoint compute_geohash_distance(const std::string& geohash1, const std::string& geohash2);
    std::string south(const std::string& geohash);
    std::string north(const std::string& geohash);
    std::string west(const std::string& geohash);
    std::string east(const std::string& geohash);

    std::array<std::string, 8> neighbors(const std::string& geohash);

    std::vector<std::string> geohash_cover(GeoBBox bbox, int precision);

    int precision_for_radius(double radius_meters);
    int precision_for_bbox(GeoBBox bbox);

}
