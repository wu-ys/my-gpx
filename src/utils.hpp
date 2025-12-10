#include "config.hpp"

namespace mygpx {

constexpr double PI = 3.14159265358979323846;

long long to_timestamp_seconds(const std::string& iso8601);
long long to_timestamp_millis(const std::string& iso8601);

double ComputePointDistance(double lat1, double lon1, double lat2, double lon2);


}