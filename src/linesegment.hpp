#include "config.hpp"
#include "segment.hpp"
#include "gpxpoint.hpp"

namespace mygpx {

class LineSegment : public BaseSegment {

private:

    GPXPoint startPoint;
    GPXPoint endPoint;

    std::vector<GPXPoint> points;

public:
    virtual size_t getPointCount() const;
    virtual GPXPoint getStartPoint() const;
    virtual GPXPoint getEndPoint() const;
    virtual double getTotalDistance() const;

    void interpolatePoints(int numPoints);
    void interpolatePoints(double intervalDistance);

};

}