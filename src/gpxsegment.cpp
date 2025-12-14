#include "gpxsegment.hpp"
#include "gpxloader.hpp"

namespace mygpx {

void GPXSegment::addPoint(double latitude, double longitude, float elevation, long long timestamp) {
    validateLatitude(latitude);
    validateLongitude(longitude);
    points.emplace_back(latitude, longitude, elevation, timestamp);
}

void GPXSegment::addPoint(double latitude, double longitude, float elevation, long long timestamp, float speed, float course) {
    validateLatitude(latitude);
    validateLongitude(longitude);
    points.emplace_back(latitude, longitude, elevation, timestamp, speed, course);
}

void GPXSegment::addPoints(const std::vector<GPXPoint>& newPoints) {

    for (const auto& pt : newPoints) {
        validateLatitude(pt.getLatitude());
        validateLongitude(pt.getLongitude());
    }
    points.insert(points.end(), newPoints.begin(), newPoints.end());

}

void GPXSegment::addPoints(const std::vector<double>& latitudes,
                           const std::vector<double>& longitudes,
                           const std::vector<float>& elevations,
                           const std::vector<long long>& timestamps) {

    size_t count = latitudes.size();
    if (longitudes.size() != count || elevations.size() != count || timestamps.size() != count) {
        throw std::invalid_argument("All input vectors must have the same length.");
    }

    points.reserve(points.size() + count);

    for (size_t i = 0; i < count; ++i) {
        validateLatitude(latitudes[i]);
        validateLongitude(longitudes[i]);
        points.emplace_back(latitudes[i], longitudes[i], elevations[i], timestamps[i]);
    }

}

void GPXSegment::addPoints(const std::vector<double>& latitudes,
                           const std::vector<double>& longitudes,
                           const std::vector<float>& elevations,
                           const std::vector<long long>& timestamps,
                           const std::vector<float>& speeds,
                           const std::vector<float>& courses) {

    size_t count = latitudes.size();
    if (longitudes.size() != count || elevations.size() != count || timestamps.size() != count ||
        speeds.size() != count || courses.size() != count) {
        throw std::invalid_argument("All input vectors must have the same length.");
    }

    points.reserve(points.size() + count);

    for (size_t i = 0; i < count; ++i) {
        validateLatitude(latitudes[i]);
        validateLongitude(longitudes[i]);
        points.emplace_back(latitudes[i], longitudes[i], elevations[i], timestamps[i], speeds[i], courses[i]);
    }

}

size_t GPXSegment::getPointCount() const {
    return points.size();
}

GPXPoint GPXSegment::getStartPoint() const {
    // check: non empty
    validateIndex(points.size(),1);
    return points[0];
}

GPXPoint GPXSegment::getEndPoint() const {
    validateIndex(points.size(),1);
    return points[points.size() - 1];
}

double GPXSegment::getTotalDistance() const {
    validateIndex(points.size(),1);
    double totalDistance = 0.0;
    for (int i = 0; i < points.size() - 1; ++i)
        totalDistance += ComputePointDistance(points[i], points[i + 1]);

    return totalDistance;

}

void GPXSegment::loadFromGPXFile(const std::string& filename) {

    GPXLoader loader;

    points = loader.loadSegmentFromFile(filename);

}

}