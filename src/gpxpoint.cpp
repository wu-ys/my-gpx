#include "gpxpoint.hpp"
#include "utils.hpp"

namespace mygpx {

  GPXPoint::GPXPoint(double latitude, double longitude, double elevation, const std::string& iso8601)
      : lat(latitude), lon(longitude), ele(elevation), time(to_timestamp_seconds(iso8601)) {}


  double ComputePointDistance(const GPXPoint& p1, const GPXPoint& p2) {
    return ComputePointDistance(p1.lat, p1.lon, p2.lat, p2.lon);
  }

  double ComputeTimeDifference(const GPXPoint& p1, const GPXPoint& p2) {
    return static_cast<double>(p2.time - p1.time);
  }

  double ComputeElevationDifference(const GPXPoint& p1, const GPXPoint& p2) {
    return p2.ele - p1.ele;
  }

  double ComputeVerticalSpeed(const GPXPoint& p1, const GPXPoint& p2) {
    double elevation_diff = ComputeElevationDifference(p1, p2);
    double time_diff = ComputeTimeDifference(p1, p2);
    if (time_diff == 0) {
      return 0.0;
    }
    return elevation_diff / time_diff; // meters per second
  }

  double ComputeHorizontalSpeedRate(const GPXPoint& p1, const GPXPoint& p2) {
    double distance = ComputePointDistance(p1, p2);
    double time_diff = std::abs(ComputeTimeDifference(p1, p2));
    if (time_diff == 0) {
      return 0.0;
    }
    return distance / time_diff; // meters per second
  }

}