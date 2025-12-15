#include "utils.hpp"
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

bool out_of_china(double lng, double lat) {
    return not (73.66 < lng && lng < 135.05 && 3.86 < lat && lat < 53.55);
}

double transformlat(double lng, double lat) {
    double ret = -100.0 + 2.0 * lng + 3.0 * lat + 0.2 * lat * lat + 0.1 * lng * lat + 0.2 * sqrt(abs(lng));
    ret += (20.0 * sin(6.0 * lng * PI) + 20.0 * sin(2.0 * lng * PI)) * 2.0 / 3.0;
    ret += (20.0 * sin(lat * PI) + 40.0 * sin(lat / 3.0 * PI)) * 2.0 / 3.0;
    ret += (160.0 * sin(lat / 12.0 * PI) + 320 * sin(lat * PI / 30.0)) * 2.0 / 3.0;
    return ret;
}

double transformlng(double lng, double lat) {
    double ret = 300.0 + lng + 2.0 * lat + 0.1 * lng * lng + 0.1 * lng * lat + 0.1 * sqrt(abs(lng));
    ret += (20.0 * sin(6.0 * lng * PI) + 20.0 * sin(2.0 * lng * PI)) * 2.0 / 3.0;
    ret += (20.0 * sin(lng * PI) + 40.0 * sin(lng / 3.0 * PI)) * 2.0 / 3.0;
    ret += (150.0 * sin(lng / 12.0 * PI) + 300.0 * sin(lng / 30.0 * PI)) * 2.0 / 3.0;
    return ret;
}


std::pair<double, double> gcj02_to_wgs84(double lat, double lon) {
    double a = EARTH_EQUATOR_RADIUS;
    double ee = EARTH_ECCENTRICITY;
    if (out_of_china(lon, lat))
        return std::make_pair(lon, lat);
    else {
        double dlat = transformlat(lon - 105.0, lat - 35.0);
        double dlon = transformlng(lon - 105.0, lat - 35.0);
        double radlat = lat / 180.0 * PI;
        double magic = sin(radlat);
        magic = 1 - ee * magic * magic;
        double sqrtmagic = sqrt(magic);
        dlat = (dlat * 180.0) / ((a * (1 - ee)) / (magic * sqrtmagic) * PI);
        dlon = (dlon * 180.0) / (a / sqrtmagic * cos(radlat) * PI);
        double mglat = lat + dlat;
        double mglon = lon + dlon;
        return std::make_pair(lon * 2 - mglon, lat * 2 - mglat);
    }
}

std::pair<double, double> wgs84_to_gcj02(double lat, double lon) {
    double a = EARTH_EQUATOR_RADIUS;
    double ee = EARTH_ECCENTRICITY;
    if (out_of_china(lon, lat))
        return std::make_pair(lon, lat);
    double dlat = transformlat(lon - 105.0, lat - 35.0);
    double dlon = transformlng(lon - 105.0, lat - 35.0);
    double radlat = lat / 180.0 * PI;
    double magic = sin(radlat);
    magic = 1 - ee * magic * magic;
    double sqrtmagic = sqrt(magic);
    dlat = (dlat * 180.0) / ((a * (1 - ee)) / (magic * sqrtmagic) * PI);
    dlon = (dlon * 180.0) / (a / sqrtmagic * cos(radlat) * PI);
    double mglat = lat + dlat;
    double mglon = lon + dlon;
    return std::make_pair(mglon, mglat);
}

double ComputePointDistance(double lat1, double lon1, double lat2, double lon2) {

    const double R = EARTH_AVERAGE_RADIUS; // Earth radius in meters
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