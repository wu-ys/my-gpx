#pragma once
#include "config.hpp"
#include "segment.hpp"
#include <memory>
#include <vector>

namespace mygpx {

class Track {

  private:
    std::vector<std::shared_ptr<BaseSegment> > segs;
    mutable std::vector<GPXPoint> points_cache;

  public:

    Track() {}
    ~Track() {}

    size_t getPointCount() const;
    GPXPoint getStartPoint() const;
    GPXPoint getEndPoint() const;
    size_t getSegmentPointCount(size_t idx) const;
    GPXPoint getSegmentStartPoint(size_t idx) const;
    GPXPoint getSegmentEndPoint(size_t idx) const;

    double getTotalDistance() const;
    double getSegmentDistance(size_t idx) const;

    std::string getSegmentType(size_t idx) const;

    double getStartLat() const;
    double getStartLon() const;
    double getStartEle() const;
    double getEndLat() const;
    double getEndLon() const;
    double getEndEle() const;
    double getTotalLatDiff() const;
    double getTotalLonDiff() const;
    double getTotalEleDiff() const;
    long long getStartTime() const;
    long long getEndTime() const;
    int getTotalTime() const;

    double getSegmentStartLat(size_t idx) const;
    double getSegmentStartLon(size_t idx) const;
    double getSegmentStartEle(size_t idx) const;
    double getSegmentEndLat(size_t idx) const;
    double getSegmentEndLon(size_t idx) const;
    double getSegmentEndEle(size_t idx) const;
    double getSegmentLatDiff(size_t idx) const;
    double getSegmentLonDiff(size_t idx) const;
    double getSegmentEleDiff(size_t idx) const;
    long long getSegmentStartTime(size_t idx) const;
    long long getSegmentEndTime(size_t idx) const;
    int getSegmentTime(size_t idx) const;

    const std::vector<GPXPoint>& getPoints() const;
    const GPXPoint& getPoint(size_t index) const;

  public:

    friend class GPXLoader;

    void addSegment(std::shared_ptr<BaseSegment> seg);

    void addSegment(std::shared_ptr<BaseSegment> seg, size_t idx);

    void addSegmentByTime(std::shared_ptr<BaseSegment> seg);

    std::shared_ptr<BaseSegment> getSegment(size_t idx);
    std::string getSegmentType(size_t idx);

    void toGPX(const std::string& filename) const;

};

}
