#include "flightsegment.hpp"
#include "rapidcsv.h"

namespace mygpx {

void FlightSegment::loadFromVariflightCSV(const std::string& filename) {
  
    rapidcsv::Document doc(filename, rapidcsv::LabelParams(0, -1));

    std::vector<double> latitudes = doc.GetColumn<double>("Latitude");
    std::vector<double> longitudes = doc.GetColumn<double>("Longitude");
    std::vector<float> elevations = doc.GetColumn<float>("Height");
    std::vector<long long> timestamps = doc.GetColumn<long long>("Time");
    std::vector<float> speeds = doc.GetColumn<float>("Speed");
    std::vector<float> course = doc.GetColumn<float>("Angle");

    addPoints(latitudes, longitudes, elevations, timestamps, speeds, course);

}


}
