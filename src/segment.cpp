#include "segment.hpp"

namespace mygpx {

double BaseSegment::getStartLat() const { return getStartPoint().getLatitude(); }
double BaseSegment::getStartLon() const { return getStartPoint().getLongitude(); }
double BaseSegment::getStartEle() const { return getStartPoint().getElevation(); }
double BaseSegment::getEndLat() const { return getEndPoint().getLatitude(); }
double BaseSegment::getEndLon() const { return getEndPoint().getLongitude(); }
double BaseSegment::getEndEle() const { return getEndPoint().getElevation(); }
double BaseSegment::getTotalLatDiff() const { return getEndLat() - getStartLat(); }
double BaseSegment::getTotalLonDiff() const { return getEndLon() - getStartLon(); }
double BaseSegment::getTotalEleDiff() const { return getEndEle() - getStartEle(); }

long long BaseSegment::getStartTime() const {
    if (!has_time) throw std::runtime_error("Segment does not have time information.");
    else return getStartPoint().getTime();
}
long long BaseSegment::getEndTime() const {
    if (!has_time) throw std::runtime_error("Segment does not have time information.");
    else return getEndPoint().getTime();
}
int BaseSegment::getTotalTime() const {
    if (!has_time) throw std::runtime_error("Segment does not have time information.");
    return getEndTime() - getStartTime();
}

const std::vector<GPXPoint>& BaseSegment::getPoints() const { return points; }
const GPXPoint& BaseSegment::getPoint(size_t index) const {
    validateIndex(index, points.size());
    return points[index];
}

}