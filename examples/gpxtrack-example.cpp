#include "gpxloader.hpp"

int main() {

    mygpx::GPXLoader loader;

    std::vector<mygpx::Track> tracks = loader.loadTracksFromFile("file/2025.11.18-00_00.gpx");

    for (auto t : tracks)
      std::cout << "Track with " << t.getPointCount() << " points.\n";

    tracks[0].toGPX("file/example-track-output.gpx");

    return 0;

}