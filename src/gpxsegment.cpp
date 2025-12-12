#include "gpxsegment.hpp"

namespace mygpx {

void GPXSegment::addPoint(double latitude, double longitude, double elevation, long long timestamp) {
    validateLatitude(latitude);
    validateLongitude(longitude);
    points.emplace_back(latitude, longitude, elevation, timestamp);
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

}