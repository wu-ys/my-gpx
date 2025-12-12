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

long long BaseSegment::getStartTime() const { return getStartPoint().getTime(); }
long long BaseSegment::getEndTime() const { return getEndPoint().getTime(); }
double BaseSegment::getTotalTime() const { return getEndTime() - getStartTime(); }

const std::vector<GPXPoint>& BaseSegment::getPoints() const { return points; }
const GPXPoint& BaseSegment::getPoint(size_t index) const {
    validateIndex(index, points.size());
    return points[index];
}

}