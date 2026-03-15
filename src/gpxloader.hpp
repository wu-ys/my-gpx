#pragma once
#include "config.hpp"
#include "gpxsegment.hpp"
#include "gpxpoint.hpp"
#include "track.hpp"
#include "validation.hpp"

namespace mygpx {

class GPXLoader {

private:
    // config and result
    bool loadSpeed;
    bool loadCourse;
    bool loadTime;
    bool loadEle;

    bool speedLoaded;
    bool courseLoaded;
    bool timeLoaded;
    bool eleLoaded;

public:

    GPXLoader() {}
    ~GPXLoader() {}

    // set whether to load extension data
    void setLoadSpeed(bool enable)  { loadSpeed  = enable; }
    void setLoadCourse(bool enable) { loadCourse = enable; }
    void setLoadEle(bool enable)    { loadEle    = enable; }
    void setLoadTime(bool enable)   { loadTime   = enable; }

    // see whether speed and course are successfully loaded
    bool getSpeedLoaded() const  { return speedLoaded;  }
    bool getCourseLoaded() const { return courseLoaded; }
    bool getEleLoaded() const    { return eleLoaded;    }
    bool getTimeLoaded() const   { return timeLoaded;   }

    // load as one segment
    GPXSegment loadSegmentFromFile(const std::string& filename);

    // // load multiple segments
    // std::vector<GPXSegment> loadSegmentsFromFile(const std::string& filename);

    // load multiple tracks
    std::vector<Track> loadTracksFromFile(const std::string& filename);

};

}