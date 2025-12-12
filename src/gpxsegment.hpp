#include "config.hpp"
#include "segment.hpp"

namespace mygpx {

class GPXSegment : public BaseSegment {

public:

    GPXSegment() {}

    ~GPXSegment() {}

    GPXSegment(const std::string& name) { loadFromFile(name); }

    void loadFromFile(const std::string& filename); // TODO: load from file

    void addPoint(double latitude, double longitude, double elevation, long long timestamp);

    virtual size_t getPointCount() const override;
    virtual GPXPoint getStartPoint() const override;
    virtual GPXPoint getEndPoint() const override;
    virtual double getTotalDistance() const override;


};

}