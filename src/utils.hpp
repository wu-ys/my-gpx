#pragma once
#include "config.hpp"

namespace mygpx {

constexpr double PI = 3.14159265358979323846;
constexpr double EARTH_EQUATOR_RADIUS = 6378245.0;
constexpr double EARTH_POLAR_RADIUS = 6356752.3142;
constexpr double EARTH_AVERAGE_RADIUS = 6371000.0;
constexpr double EARTH_ECCENTRICITY = 0.00669342162296594323;

using GeoPoint = std::pair<double,double>; // lat, lon
using GeoBBox = std::tuple<double, double, double, double>; // lat_min, lat_max, lon_min, lon_max

enum CoordinateSystem {
    CoordinateSystem_GCJ02,
    CoordinateSystem_WGS84,
    CoordinateSystem_BD09
};

long long to_timestamp_seconds(const std::string& iso8601);
long long to_timestamp_millis(const std::string& iso8601);

std::string from_timestamp_seconds(long long timestamp);
std::string from_timestamp_millis(long long timestamp);

double kmph_to_mps(double kmph);
double mps_to_kmph(double mps);

GeoPoint gcj02_to_wgs84(double lat, double lon);
GeoPoint wgs84_to_gcj02(double lat, double lon);
GeoPoint bd09_to_wgs84(double lat, double lon);
GeoPoint bd09_to_gcj02(double lat, double lon);
GeoPoint wgs84_to_bd09(double lat, double lon);
GeoPoint gcj02_to_bd09(double lat, double lon);

GeoPoint gcj02_to_wgs84(GeoPoint gp);
GeoPoint wgs84_to_gcj02(GeoPoint gp);
GeoPoint bd09_to_wgs84(GeoPoint gp);
GeoPoint bd09_to_gcj02(GeoPoint gp);
GeoPoint wgs84_to_bd09(GeoPoint gp);
GeoPoint gcj02_to_bd09(GeoPoint gp);

GeoPoint to_wgs84(double lat, double lon, CoordinateSystem coord);
GeoPoint to_gcj02(double lat, double lon, CoordinateSystem coord);
GeoPoint to_bd09(double lat, double lon, CoordinateSystem coord);

GeoPoint to_wgs84(GeoPoint gp, CoordinateSystem coord);
GeoPoint to_gcj02(GeoPoint gp, CoordinateSystem coord);
GeoPoint to_bd09(GeoPoint gp, CoordinateSystem coord);

double ComputePointDistance(double lat1, double lon1, double lat2, double lon2);
double ComputePointDistance(GeoPoint gp1, GeoPoint gp2);

std::string getAmapAPIKey();


}