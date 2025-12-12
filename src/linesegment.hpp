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

    LineSegment() {}
    ~LineSegment() {}
    LineSegment(const GPXPoint& start, const GPXPoint& end)
        : startPoint(start), endPoint(end) {}

    LineSegment(const GPXPoint& start, const GPXPoint& end, int numPoints)
        : startPoint(start), endPoint(end) {
        interpolatePoints(numPoints);
    }

    LineSegment(const GPXPoint& start, const GPXPoint& end, double intervalDistance)
        : startPoint(start), endPoint(end) {
        interpolatePoints(intervalDistance);
    }

    virtual size_t getPointCount() const override;
    virtual GPXPoint getStartPoint() const override;
    virtual GPXPoint getEndPoint() const override;
    virtual double getTotalDistance() const override;
    virtual std::ostream& operator<<(std::ostream& os) const override;

    void interpolatePoints(int numPoints);
    void interpolatePoints(double intervalDistance);

};

}