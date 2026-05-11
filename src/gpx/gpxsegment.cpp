#include "gpxsegment.hpp"
#include "gpxloader.hpp"
#include <cmath>
namespace mygpx {

void GPXSegment::addPoint(double latitude, double longitude, float elevation, long long timestamp) {
    validateLatitude(latitude);
    validateLongitude(longitude);
    if (timestamp == 0) has_time = false;
    points.emplace_back(latitude, longitude, elevation, timestamp);
}

void GPXSegment::addPoint(double latitude, double longitude, float elevation, long long timestamp, float speed, float course) {
    validateLatitude(latitude);
    validateLongitude(longitude);
    if (timestamp == 0) has_time = false;
    points.emplace_back(latitude, longitude, elevation, timestamp, speed, course);
}

void GPXSegment::addPoints(const std::vector<GPXPoint>& newPoints) {

    for (const auto& pt : newPoints) {
        validateLatitude(pt.getLatitude());
        validateLongitude(pt.getLongitude());
        if (pt.getTime() == 0) has_time = false;
    }

    points.insert(points.end(), newPoints.begin(), newPoints.end());

}

void GPXSegment::addPoints(const std::vector<double>& latitudes,
                           const std::vector<double>& longitudes,
                           const std::vector<float>& elevations,
                           const std::vector<long long>& timestamps) {

    size_t count = latitudes.size();
    if (longitudes.size() != count || elevations.size() != count) {
        throw std::invalid_argument("All input vectors must have the same length.");
    }

    if (timestamps.size() != count) has_time = false;

    points.reserve(points.size() + count);

    for (size_t i = 0; i < count; ++i) {
        validateLatitude(latitudes[i]);
        validateLongitude(longitudes[i]);
        points.emplace_back(latitudes[i], longitudes[i], elevations[i], timestamps[i]);
        if (timestamps[i] == 0) has_time = false;
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
        if (timestamps[i] == 0) has_time = false;
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
    double totalDistance = 0.0;
    for (int i = 0; i < points.size() - 1; ++i)
        totalDistance += ComputePointDistance(points[i], points[i + 1]);

    return totalDistance;

}


void GPXSegment::SetInterval(double distance = 50) {
    validateInRange(distance, 25, 2000);
    
    if (points.size() <= 0) return;
    
    std::vector<GPXPoint> new_points = {points[0]};

    double intervalMax = distance * 1.5;

    for (int i = 0; i < points.size() - 1; ++i) {
        double d = ComputePointDistance(points[i], points[i + 1]);

        int target_segs = 0;

        if (d < intervalMax) {
            new_points.emplace_back(points[i+1]);
        } else {
            target_segs = std::round(d / distance);
            std::vector<GPXPoint> seg_points(target_segs);

            for (int i = 0; i < target_segs; ++i) {
                double ratio = (i + 1) / (double)target_segs;
                double lat = 
                    points[i].getLatitude() + ratio * (points[i + 1].getLatitude() - points[i].getLatitude());
                double lon = 
                    points[i].getLongitude() + ratio * (points[i + 1].getLongitude() - points[i].getLongitude());
                float ele = 
                    points[i].getElevation() + ratio * (points[i + 1].getElevation() - points[i].getElevation());
                long long time = 0;
                if (has_time) time = 
                    points[i].getTime() + std::round(ratio * (points[i + 1].getTime() - points[i].getTime()));
                seg_points[i] = GPXPoint(lat, lon, ele, time);
            }

            new_points.insert(new_points.end(), seg_points.begin(), seg_points.end());
        }
    }

    points = new_points;

}


void GPXSegment::DetectStopAndMovements() const {

    int n = getPointCount();

    std::vector<short> movement_type(n);

    


}

}