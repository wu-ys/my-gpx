#pragma once
#include "config.hpp"

namespace mygpx {

constexpr double PI = 3.14159265358979323846;

long long to_timestamp_seconds(const std::string& iso8601);
long long to_timestamp_millis(const std::string& iso8601);

std::string from_timestamp_seconds(long long timestamp);
std::string from_timestamp_millis(long long timestamp);

double kmph_to_mps(double kmph);
double mps_to_kmph(double mps);

double ComputePointDistance(double lat1, double lon1, double lat2, double lon2);


}