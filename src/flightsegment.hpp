#pragma once
#include "config.hpp"
#include "gpxsegment.hpp"

namespace mygpx {

class FlightSegment : public GPXSegment {

public:

    FlightSegment() {}

    ~FlightSegment() {}

    FlightSegment(const std::string& name) { loadFromVariflightCSV(name); }

    void loadFromVariflightCSV(const std::string& filename);

};

}