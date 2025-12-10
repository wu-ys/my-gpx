#include "utils.hpp"
#include <cmath>

namespace mygpx {

long long to_timestamp_seconds(const std::string& iso8601) {
    std::tm tm = {};
    std::istringstream ss(iso8601);

    // Example: 2025-11-17T16:00:01Z
    ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
    if (ss.fail()) {
        throw std::runtime_error("Failed to parse time string.");
    }

    // Convert to UTC time_t
    time_t t = timegm(&tm);  // IMPORTANT: UTC (Linux/WSL provides timegm)
    return static_cast<long long>(t);
}

long long to_timestamp_millis(const std::string& iso8601) {
    return to_timestamp_seconds(iso8601) * 1000LL;
}

double ComputePointDistance(double lat1, double lon1, double lat2, double lon2) {

    const double pi = 3.14159265358979323846;
    const double R = 6371000.0; // Earth radius in meters
    double lat1_rad = lat1 * PI / 180.0;
    double lon1_rad = lon1 * PI / 180.0;
    double lat2_rad = lat2 * PI / 180.0;
    double lon2_rad = lon2 * PI / 180.0;

    double dlat = lat2_rad - lat1_rad;
    double dlon = lon2_rad - lon1_rad;

    double a = sin(dlat / 2) * sin(dlat / 2) +
               cos(lat1_rad) * cos(lat2_rad) *
               sin(dlon / 2) * sin(dlon / 2);
    double c = 2 * atan2(sqrt(a), sqrt(1 - a));

    return R * c; // Distance in meters

}


}