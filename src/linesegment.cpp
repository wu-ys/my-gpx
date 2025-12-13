#include "linesegment.hpp"
#include "validation.hpp"

namespace mygpx {

size_t LineSegment::getPointCount() const {
    return 2;
}

GPXPoint LineSegment::getStartPoint() const {
    return startPoint;
}

GPXPoint LineSegment::getEndPoint() const {
    return endPoint;
}

double LineSegment::getTotalDistance() const {
    return ComputePointDistance(startPoint, endPoint);
}

void LineSegment::interpolatePoints(int numPoints) {
    validateInRange(numPoints, 2);

    points.resize(numPoints);

    double latStep = (endPoint.getLatitude() - startPoint.getLatitude()) / (numPoints - 1);
    double lonStep = (endPoint.getLongitude() - startPoint.getLongitude()) / (numPoints - 1);
    double eleStep = (endPoint.getElevation() - startPoint.getElevation()) / (numPoints - 1);
    double timeStep = ComputeTimeDifference(startPoint, endPoint) / (double)(numPoints - 1);

    for (int i = 0; i < numPoints; ++i) {
        double lat = startPoint.getLatitude() + i * latStep;
        double lon = startPoint.getLongitude() + i * lonStep;
        double ele = startPoint.getElevation() + i * eleStep;
        long long time = startPoint.getTime() + static_cast<long long>(i * timeStep);

        points[i] = GPXPoint(lat, lon, ele, time);
    }

    return;
}

void LineSegment::interpolatePoints(double intervalDistance) {
    validateInRange(intervalDistance, 1.0); // minimal resolution: 1 meter

    double totalDistance = getTotalDistance();
    int numPoints = static_cast<int>(totalDistance / intervalDistance) + 1;

    interpolatePoints(numPoints);

    return;

}

std::ostream& operator<<(std::ostream& os, const LineSegment& segment) {
    os << "LineSegment:\n\t" << segment.getStartPoint() << " -> " << segment.getEndPoint() << ",\n\tDistance: " << segment.getTotalDistance() << " meters, Point count: " << segment.getPointCount() << std::endl;

    return os;

}