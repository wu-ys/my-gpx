#include "config.hpp"
#include "segment.hpp"

namespace mygpx {

class GPXSegment : public BaseSegment {

public:

    GPXSegment() {}

    ~GPXSegment() {}

    GPXSegment(const std::string& name) { loadFromGPXFile(name); }

    GPXSegment(const std::vector<GPXPoint>& newPoints) {
        addPoints(newPoints);
    }

    GPXSegment(const std::vector<double>& latitudes,
               const std::vector<double>& longitudes,
               const std::vector<float>& elevations,
               const std::vector<long long>& timestamps) 
    {
        addPoints(latitudes, longitudes, elevations, timestamps);
    }

    GPXSegment(const std::vector<double>& latitudes,
               const std::vector<double>& longitudes,
               const std::vector<float>& elevations,
               const std::vector<long long>& timestamps,
               const std::vector<float>& speeds,
               const std::vector<float>& courses) 
    {
        addPoints(latitudes, longitudes, elevations, timestamps, speeds, courses);
    }


    void loadFromGPXFile(const std::string& filename);

    void addPoint(double latitude, double longitude, float elevation, long long timestamp);
    void addPoint(double latitude, double longitude, float elevation, long long timestamp, float speed, float course);
    void addPoint(double latitude, double longitude, float elevation, const std::string& iso8601) {
        addPoint(latitude, longitude, elevation, to_timestamp_seconds(iso8601));
    }
    void addPoint(double latitude, double longitude, float elevation, const std::string& iso8601, float speed, float course) {
        addPoint(latitude, longitude, elevation, to_timestamp_seconds(iso8601), speed, course);
    }

    void addPoints(const std::vector<GPXPoint>& newPoints);
    void addPoints(const std::vector<double>& latitudes,
                   const std::vector<double>& longitudes,
                   const std::vector<float>& elevations,
                   const std::vector<long long>& timestamps);
    void addPoints(const std::vector<double>& latitudes,
                   const std::vector<double>& longitudes,
                   const std::vector<float>& elevations,
                   const std::vector<long long>& timestamps,
                   const std::vector<float>& speeds,
                   const std::vector<float>& courses);

    virtual size_t getPointCount() const override;
    virtual GPXPoint getStartPoint() const override;
    virtual GPXPoint getEndPoint() const override;
    virtual double getTotalDistance() const override;


};

}