#include "gpxloader.hpp"

int main() {

    mygpx::GPXLoader loader;

    mygpx::GPXSegment segment = loader.loadSegmentFromFile("file/2025.11.18-00_00.gpx");

    std::cout << "Segment with " << segment.getPointCount() << " points.\n";

    return 0;

}