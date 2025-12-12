#include "config.hpp"
#include "segment.hpp"

namespace mygpx {

class GPXSegment : public BaseSegment {

private:
    std::vector<double> latitudes;
    std::vector<double> longitudes;
    std::vector<double> elevations;
    std::vector<long long> timestamps;
    int length;

public:

    GPXSegment() : length(0) {}

    void addPoint(double latitude, double longitude, double elevation, long long timestamp) {
        latitudes.push_back(latitude);
        longitudes.push_back(longitude);
        elevations.push_back(elevation);
        timestamps.push_back(timestamp);
    }

    size_t getPointCount() const {
        return latitudes.size();
    }

    GPXPoint getStartPoint() const {
        return GPXPoint(latitudes.front(), longitudes.front(), elevations.front(), timestamps.front());
    }

    GPXPoint getEndPoint() const {
        return GPXPoint(latitudes.back(), longitudes.back(), elevations.back(), timestamps.back());
    }

    double getLatitude(size_t index) const {
        return latitudes.at(index);
    }

    double getLongitude(size_t index) const {
        return longitudes.at(index);
    }

    double getElevation(size_t index) const {
        return elevations.at(index);
    }

    long long getTimestamp(size_t index) const {
        return timestamps.at(index);
    }


};

}