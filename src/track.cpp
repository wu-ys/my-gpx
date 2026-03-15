#include "track.hpp"
#include "validation.hpp"

namespace mygpx {

size_t Track::getPointCount() const {
    size_t total = 0;
    for (const auto& seg : segs) {
        total += seg->getPointCount();
    }
    return total;
}

GPXPoint Track::getStartPoint() const {
    validateInRange(segs.size(), static_cast<size_t>(1));
    return segs.front()->getStartPoint();
}

GPXPoint Track::getEndPoint() const {
    validateInRange(segs.size(), static_cast<size_t>(1));
    return segs.back()->getEndPoint();
}

size_t Track::getSegmentPointCount(size_t idx) const {
    validateIndex(idx, segs.size());
    return segs[idx]->getPointCount();
}

GPXPoint Track::getSegmentStartPoint(size_t idx) const {
    validateIndex(idx, segs.size());
    return segs[idx]->getStartPoint();
}

GPXPoint Track::getSegmentEndPoint(size_t idx) const {
    validateIndex(idx, segs.size());
    return segs[idx]->getEndPoint();
}

double Track::getTotalDistance() const {
    double total = 0.0;
    for (const auto& seg : segs) {
        total += seg->getTotalDistance();
    }
    return total;
}

double Track::getSegmentDistance(size_t idx) const {
    validateIndex(idx, segs.size());
    return segs[idx]->getTotalDistance();
}

std::string Track::getSegmentType(size_t idx) const {
    validateIndex(idx, segs.size());
    return segs[idx]->getType();
}

double Track::getStartLat() const { return getStartPoint().getLatitude(); }
double Track::getStartLon() const { return getStartPoint().getLongitude(); }
double Track::getStartEle() const { return getStartPoint().getElevation(); }
double Track::getEndLat() const { return getEndPoint().getLatitude(); }
double Track::getEndLon() const { return getEndPoint().getLongitude(); }
double Track::getEndEle() const { return getEndPoint().getElevation(); }
double Track::getTotalLatDiff() const { return getEndLat() - getStartLat(); }
double Track::getTotalLonDiff() const { return getEndLon() - getStartLon(); }
double Track::getTotalEleDiff() const { return getEndEle() - getStartEle(); }

long long Track::getStartTime() const {
    validateInRange(segs.size(), static_cast<size_t>(1));
    return segs.front()->getStartTime();
}

long long Track::getEndTime() const {
    validateInRange(segs.size(), static_cast<size_t>(1));
    return segs.back()->getEndTime();
}

int Track::getTotalTime() const {
    return getEndTime() - getStartTime();
}

long long Track::getSegmentStartTime(size_t idx) const {
    validateIndex(idx, segs.size());
    return segs[idx]->getStartTime();
}
long long Track::getSegmentEndTime(size_t idx) const {
    validateIndex(idx, segs.size());
    return segs[idx]->getEndTime();
}
int Track::getSegmentTime(size_t idx) const { return getSegmentEndTime(idx) - getSegmentStartTime(idx); }

double Track::getSegmentStartLat(size_t idx) const { return getSegmentStartPoint(idx).getLatitude(); }
double Track::getSegmentStartLon(size_t idx) const { return getSegmentStartPoint(idx).getLongitude(); }
double Track::getSegmentStartEle(size_t idx) const { return getSegmentStartPoint(idx).getElevation(); }
double Track::getSegmentEndLat(size_t idx) const { return getSegmentEndPoint(idx).getLatitude(); }
double Track::getSegmentEndLon(size_t idx) const { return getSegmentEndPoint(idx).getLongitude(); }
double Track::getSegmentEndEle(size_t idx) const { return getSegmentEndPoint(idx).getElevation(); }
double Track::getSegmentLatDiff(size_t idx) const { return getSegmentEndLat(idx) - getSegmentStartLat(idx); }
double Track::getSegmentLonDiff(size_t idx) const { return getSegmentEndLon(idx) - getSegmentStartLon(idx); }
double Track::getSegmentEleDiff(size_t idx) const { return getSegmentEndEle(idx) - getSegmentStartEle(idx); }

const std::vector<GPXPoint>& Track::getPoints() const {
    points_cache.clear();
    for (const auto& seg : segs) {
        const auto& pts = seg->getPoints();
        points_cache.insert(points_cache.end(), pts.begin(), pts.end());
    }
    return points_cache;
}

const GPXPoint& Track::getPoint(size_t index) const {
    size_t offset = 0;
    for (const auto& seg : segs) {
        size_t count = seg->getPointCount();
        if (index < offset + count) {
            return seg->getPoint(index - offset);
        }
        offset += count;
    }
    throw std::out_of_range("Track point index out of range.");
}

}
