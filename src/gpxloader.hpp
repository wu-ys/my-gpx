#include "config.hpp"
#include "gpxsegment.hpp"
#include "gpxpoint.hpp"
#include "validation.hpp"

namespace mygpx {

class GPXLoader {

private:
    // config and result
    bool loadSpeed;
    bool loadCourse;

public:

    GPXLoader() {}
    ~GPXLoader() {}

    // set whether to load extension data
    void setLoadSpeed(bool enable) { loadSpeed = enable; }
    void setLoadCourse(bool enable) { loadCourse = enable; }

    // get current settings / see whether spped and course are successfully loaded
    bool getLoadSpeed() const { return loadSpeed; }
    bool getLoadCourse() const { return loadCourse; }

    // load as one segment
    std::vector<GPXPoint> loadSegmentFromFile(const std::string& filename);

    // load multiple segments
    std::vector<GPXSegment> loadSegmentsFromFile(const std::string& filename);

};

}