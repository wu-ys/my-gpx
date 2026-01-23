#pragma once
#include "config.hpp"
#include "gpxpoint.hpp"
#include "validation.hpp"

namespace mygpx {

class BaseSegment {

protected:
    std::vector<GPXPoint> points;

public:
    BaseSegment() {}
    virtual ~BaseSegment() {}

    virtual size_t getPointCount() const = 0;
    virtual GPXPoint getStartPoint() const = 0;
    virtual GPXPoint getEndPoint() const = 0;

    virtual double getTotalDistance() const = 0;

    virtual std::string getType() const = 0;
    
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
    double getTotalTime() const;
    
    virtual const std::vector<GPXPoint>& getPoints() const;
    virtual const GPXPoint& getPoint(size_t index) const;

};

}