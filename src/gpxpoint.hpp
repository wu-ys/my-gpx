#include "config.hpp"

namespace mygpx{

class GPXPoint {

public:

    GPXPoint() : lat(0.0), lon(0.0), ele(0.0), time(0) {}

    ~GPXPoint() {}

    GPXPoint(double latitude, double longitude, double elevation, long long timestamp)
        : lat(latitude), lon(longitude), ele(elevation), time(timestamp) {}

    GPXPoint(double latitude, double longitude, double elevation, const std::string& iso8601);

    double getLatitude() const { return lat; }
    double getLongitude() const { return lon; }
    double getElevation() const { return ele; }
    long long getTime() const { return time; }

    friend double ComputePointDistance(const GPXPoint& p1, const GPXPoint& p2);
    friend double ComputeTimeDifference(const GPXPoint& p1, const GPXPoint& p2);
    friend double ComputeElevationDifference(const GPXPoint& p1, const GPXPoint& p2);
    friend double ComputeVerticalSpeed(const GPXPoint& p1, const GPXPoint& p2);
    friend double ComputeHorizontalSpeedRate(const GPXPoint& p1, const GPXPoint& p2);

private:

    double lat;
    double lon;
    double ele;
    long long time;

};

}