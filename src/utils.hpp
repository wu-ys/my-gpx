#pragma once
#include "config.hpp"

namespace mygpx {

constexpr double PI = 3.14159265358979323846;
constexpr double EARTH_EQUATOR_RADIUS = 6378245.0;
constexpr double EARTH_POLAR_RADIUS = 6356752.3142;
constexpr double EARTH_AVERAGE_RADIUS = 6371000.0;
constexpr double EARTH_ECCENTRICITY = 0.00669342162296594323;

long long to_timestamp_seconds(const std::string& iso8601);
long long to_timestamp_millis(const std::string& iso8601);

std::string from_timestamp_seconds(long long timestamp);
std::string from_timestamp_millis(long long timestamp);

double kmph_to_mps(double kmph);
double mps_to_kmph(double mps);

std::pair<double, double> gcj02_to_wgs84(double lat, double lon);
std::pair<double, double> wgs84_to_gcj02(double lat, double lon);

double ComputePointDistance(double lat1, double lon1, double lat2, double lon2);

std::string getAmapAPIKey();


}