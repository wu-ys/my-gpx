#include "utils.hpp"
#include "validation.hpp"
#include <cmath>
#include <fstream>
#include <sstream>
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

std::string from_timestamp_seconds(long long timestamp) {
    std::time_t t = static_cast<std::time_t>(timestamp);
    std::tm* gm_time = gmtime(&t); // Convert to UTC

    char buffer[30];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%SZ", gm_time);
    return std::string(buffer);
}

std::string from_timestamp_millis(long long timestamp) {
    long long seconds = timestamp / 1000LL;
    return from_timestamp_seconds(seconds);
}

double kmph_to_mps(double kmph) { return kmph / 3.6; }
double mps_to_kmph(double mps) { return mps * 3.6; }

bool out_of_china(double lat, double lon) {
    return not (73.66 < lon && lon < 135.05 && 3.86 < lat && lat < 53.55);
}
bool out_of_china(GeoPoint gp) {
    auto [lat, lon] = gp;
    return not (73.66 < lon && lon < 135.05 && 3.86 < lat && lat < 53.55);
}

double transformlat(double lat, double lon) {
    double ret = -100.0 + 2.0 * lon + 3.0 * lat + 0.2 * lat * lat + 0.1 * lon * lat + 0.2 * sqrt(abs(lon));
    ret += (20.0 * sin(6.0 * lon * PI) + 20.0 * sin(2.0 * lon * PI)) * 2.0 / 3.0;
    ret += (20.0 * sin(lat * PI) + 40.0 * sin(lat / 3.0 * PI)) * 2.0 / 3.0;
    ret += (160.0 * sin(lat / 12.0 * PI) + 320 * sin(lat * PI / 30.0)) * 2.0 / 3.0;
    return ret;
}
double transformlon(double lat, double lon) {
    double ret = 300.0 + lon + 2.0 * lat + 0.1 * lon * lon + 0.1 * lon * lat + 0.1 * sqrt(abs(lon));
    ret += (20.0 * sin(6.0 * lon * PI) + 20.0 * sin(2.0 * lon * PI)) * 2.0 / 3.0;
    ret += (20.0 * sin(lon * PI) + 40.0 * sin(lon / 3.0 * PI)) * 2.0 / 3.0;
    ret += (150.0 * sin(lon / 12.0 * PI) + 300.0 * sin(lon / 30.0 * PI)) * 2.0 / 3.0;
    return ret;
}

GeoPoint gcj02_to_wgs84(double lat, double lon) {

    validateLatitude(lat);
    validateLongitude(lon);
    return gcj02_to_wgs84(std::make_pair(lat,lon));

}

GeoPoint gcj02_to_wgs84(GeoPoint gp) {
    double a = EARTH_EQUATOR_RADIUS;
    double ee = EARTH_ECCENTRICITY;
    if (out_of_china(gp))
        return gp;
    auto& [lat, lon] = gp;
    double dlat = transformlat(lat - 35.0, lon - 105.0);
    double dlon = transformlon(lat - 35.0, lon - 105.0);
    double radlat = lat / 180.0 * PI;
    double magic = sin(radlat);
    magic = 1 - ee * magic * magic;
    double sqrtmagic = sqrt(magic);
    dlat = (dlat * 180.0) / ((a * (1 - ee)) / (magic * sqrtmagic) * PI);
    dlon = (dlon * 180.0) / (a / sqrtmagic * cos(radlat) * PI);
    lat -= dlat;
    lon -= dlon;
    return gp;

}

GeoPoint wgs84_to_gcj02(double lat, double lon) {

    validateLatitude(lat);
    validateLongitude(lon);
    return wgs84_to_gcj02(std::make_pair(lat,lon));

}

GeoPoint wgs84_to_gcj02(GeoPoint gp) {
    double a = EARTH_EQUATOR_RADIUS;
    double ee = EARTH_ECCENTRICITY;
    if (out_of_china(gp))
        return gp;
    auto& [lat, lon] = gp;
    double dlat = transformlat(lat - 35.0, lon - 105.0);
    double dlon = transformlon(lat - 35.0, lon - 105.0);
    double radlat = lat / 180.0 * PI;
    double magic = sin(radlat);
    magic = 1 - ee * magic * magic;
    double sqrtmagic = sqrt(magic);
    dlat = (dlat * 180.0) / ((a * (1 - ee)) / (magic * sqrtmagic) * PI);
    dlon = (dlon * 180.0) / (a / sqrtmagic * cos(radlat) * PI);
    lat += dlat;
    lon += dlon;
    return gp;
}

// TODO
GeoPoint bd09_to_wgs84(double lat, double lon) {
    validateLatitude(lat);
    validateLongitude(lon);
    return bd09_to_wgs84(std::make_pair(lat,lon));
}
GeoPoint bd09_to_gcj02(double lat, double lon) {
    validateLatitude(lat);
    validateLongitude(lon);
    return bd09_to_gcj02(std::make_pair(lat,lon));
}
GeoPoint wgs84_to_bd09(double lat, double lon) {
    validateLatitude(lat);
    validateLongitude(lon);
    return wgs84_to_bd09(std::make_pair(lat,lon));
}
GeoPoint gcj02_to_bd09(double lat, double lon) {
    validateLatitude(lat);
    validateLongitude(lon);
    return gcj02_to_bd09(std::make_pair(lat,lon));
}

GeoPoint bd09_to_wgs84(GeoPoint gp) {
    return gcj02_to_wgs84(gp);
}
GeoPoint bd09_to_gcj02(GeoPoint gp) {
    return gp;
}
GeoPoint wgs84_to_bd09(GeoPoint gp) {
    return wgs84_to_gcj02(gp);
}
GeoPoint gcj02_to_bd09(GeoPoint gp) {
    return gp;
}

GeoPoint to_wgs84(double lat, double lon, CoordinateSystem coord) {

    validateLatitude(lat);
    validateLongitude(lon);
    return to_wgs84(std::make_pair(lat,lon), coord);

}
GeoPoint to_wgs84(GeoPoint gp, CoordinateSystem coord) {

    std::pair<double,double> result = gp;

    switch (coord) {
      case CoordinateSystem::CoordinateSystem_GCJ02:
        result = gcj02_to_wgs84(gp);
        break;
      case CoordinateSystem::CoordinateSystem_BD09:
        result = bd09_to_wgs84(gp);
        break;
      default:
        break;
    }

    return result;
}

GeoPoint to_gcj02(double lat, double lon, CoordinateSystem coord) {

    validateLatitude(lat);
    validateLongitude(lon);
    return to_gcj02(std::make_pair(lat,lon), coord);

}
GeoPoint to_gcj02(GeoPoint gp, CoordinateSystem coord) {

    std::pair<double,double> result = gp;

    switch (coord) {
      case CoordinateSystem::CoordinateSystem_WGS84:
        result = wgs84_to_gcj02(gp);
        break;
      case CoordinateSystem::CoordinateSystem_BD09:
        result = bd09_to_gcj02(gp);
        break;
      default:
        break;
    }

    return result;
}

GeoPoint to_bd09(double lat, double lon, CoordinateSystem coord) {

    validateLatitude(lat);
    validateLongitude(lon);
    return to_bd09(std::make_pair(lat,lon), coord);

}
GeoPoint to_bd09(GeoPoint gp, CoordinateSystem coord) {

    std::pair<double,double> result = gp;

    switch (coord) {
      case CoordinateSystem::CoordinateSystem_WGS84:
        result = wgs84_to_bd09(gp);
        break;
      case CoordinateSystem::CoordinateSystem_GCJ02:
        result = gcj02_to_bd09(gp);
        break;
      default:
        break;
    }

    return result;
}

double ComputePointDistance(double lat1, double lon1, double lat2, double lon2) {

    validateLatitude(lat1);
    validateLatitude(lat2);
    validateLongitude(lon1);
    validateLongitude(lon2);

    return ComputePointDistance(std::make_pair(lat1,lon1), std::make_pair(lat2, lon2));

}
double ComputePointDistance(GeoPoint gp1, GeoPoint gp2) {

    const double R = EARTH_AVERAGE_RADIUS; // Earth radius in meters
    const auto&[lat1, lon1] = gp1;
    const auto&[lat2, lon2] = gp2;
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

std::string getAmapAPIKey() {

    std::string path = std::string(MY_SOURCE_DIR) + "/AMAP_API_KEY";

    std::ifstream infile(path);
    if (!infile.is_open()) {
        throw std::runtime_error("Failed to open AMAP_API_KEY file at: " + path);
    }
    std::string apiKey;
    std::getline(infile, apiKey);
    infile.close();
    return apiKey;

}


}