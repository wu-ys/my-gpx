#pragma once
#include "config.hpp"
#include "utils.hpp"

namespace mygpx{

class GPXPoint {

public:

    GPXPoint() : lat(0.0), lon(0.0), ele(0.0), time(0), speed(0.0), course(0.0) {}

    ~GPXPoint() {}

    GPXPoint(double latitude, double longitude, float elevation, long long timestamp, float speed = 0.0, float course = 0.0)
        : lat(latitude), lon(longitude), ele(elevation), time(timestamp), speed(speed), course(course) {}

    GPXPoint(double latitude, double longitude, float elevation, const std::string& iso8601, float speed = 0.0, float course = 0.0)
        : lat(latitude), lon(longitude), ele(elevation), time(to_timestamp_seconds(iso8601)), speed(speed), course(course) {}

    double getLatitude() const { return lat; }
    double getLongitude() const { return lon; }
    float getElevation() const { return ele; }
    long long getTime() const { return time; }
    float getSpeed() const { return speed; }
    float getCourse() const { return course; }

    friend double ComputePointDistance(const GPXPoint& p1, const GPXPoint& p2);
    friend double ComputeTimeDifference(const GPXPoint& p1, const GPXPoint& p2);
    friend double ComputeElevationDifference(const GPXPoint& p1, const GPXPoint& p2);
    friend double ComputeVerticalSpeed(const GPXPoint& p1, const GPXPoint& p2);
    friend double ComputeHorizontalSpeedRate(const GPXPoint& p1, const GPXPoint& p2);

    friend std::ostream& operator<<(std::ostream& os, const GPXPoint& point);

private:

    double lat;
    double lon;
    float ele;
    long long time;
    float speed;
    float course;

};

}