#include "gpxloader.hpp"
#include <tinyxml2.h>

namespace mygpx {

GPXSegment GPXLoader::loadSegmentFromFile(const std::string& filename) {

    GPXSegment seg;
    tinyxml2::XMLDocument doc;
    if (doc.LoadFile(filename.c_str()) != tinyxml2::XML_SUCCESS) {
        throw std::runtime_error("Failed to load GPX file.");
    }

    // find <trkpt> labels
    auto* root = doc.RootElement();
    for (auto* trk = root->FirstChildElement("trk"); trk; trk = trk->NextSiblingElement("trk")) {
        for (auto* trkseg = trk->FirstChildElement("trkseg"); trkseg; trkseg = trkseg->NextSiblingElement("trkseg")) {

            for (auto* trkpt = trkseg->FirstChildElement("trkpt"); trkpt; trkpt = trkpt->NextSiblingElement("trkpt")) {

                double lat, lon;
                float ele, speed = 0.0, course = 0.0;
                std::string time;

                speedLoaded = loadSpeed;
                courseLoaded = loadCourse;
                timeLoaded = loadTime;
                eleLoaded = loadEle;

                // get lat/lon
                trkpt->QueryDoubleAttribute("lat", &lat);
                trkpt->QueryDoubleAttribute("lon", &lon);

                // ele may not exist
                auto* qele = trkpt->FirstChildElement("ele");
                if (loadEle) {
                    if (qele) qele->QueryFloatText(&ele);
                    else eleLoaded = false;
                } else ele = 0;

                // time may not exist
                auto* qtime = trkpt->FirstChildElement("time");
                if (loadTime) {
                    if (qtime) time = qtime->GetText();
                    else timeLoaded = false;
                } else time = "";

                // how to use the information from extensions?
                auto* qext = trkpt->FirstChildElement("extensions");
                if (qext && loadSpeed) {
                    auto* qspeed = qext->FirstChildElement("speed");
                    if (qspeed) qspeed->QueryFloatText(&speed);
                    else { speed = 0.0; speedLoaded = false; }
                }
                if (qext && loadCourse) {
                    auto* qcourse = qext->FirstChildElement("course");
                    if (qcourse) qcourse->QueryFloatText(&course);
                    else { course = 0.0; courseLoaded = false; }
                }

                seg.addPoint(lat, lon, ele, time, speed, course);

            }
        }
    }

    return seg;

}

std::vector<Track> GPXLoader::loadTracksFromFile(const std::string& filename) {

    std::vector<Track> tracks;

    tinyxml2::XMLDocument doc;
    if (doc.LoadFile(filename.c_str()) != tinyxml2::XML_SUCCESS) {
        throw std::runtime_error("Failed to load GPX file.");
    }

    // find <trkpt> labels
    auto* root = doc.RootElement();
    for (auto* trk = root->FirstChildElement("trk"); trk; trk = trk->NextSiblingElement("trk")) {
        tracks.emplace_back();
        auto& track = tracks.back();
        for (auto* trkseg = trk->FirstChildElement("trkseg"); trkseg; trkseg = trkseg->NextSiblingElement("trkseg")) {
            GPXSegment seg;

            for (auto* trkpt = trkseg->FirstChildElement("trkpt"); trkpt; trkpt = trkpt->NextSiblingElement("trkpt")) {

                double lat, lon;
                float ele, speed = 0.0, course = 0.0;
                std::string time;

                speedLoaded = loadSpeed;
                courseLoaded = loadCourse;
                timeLoaded = loadTime;
                eleLoaded = loadEle;

                // get lat/lon
                trkpt->QueryDoubleAttribute("lat", &lat);
                trkpt->QueryDoubleAttribute("lon", &lon);

                // ele may not exist
                auto* qele = trkpt->FirstChildElement("ele");
                if (loadEle) {
                    if (qele) qele->QueryFloatText(&ele);
                    else eleLoaded = false;
                } else ele = 0;

                // time may not exist
                auto* qtime = trkpt->FirstChildElement("time");
                if (loadTime) {
                    if (qtime) time = qtime->GetText();
                    else timeLoaded = false;
                } else time = "";

                // how to use the information from extensions?
                auto* qext = trkpt->FirstChildElement("extensions");
                if (qext && loadSpeed) {
                    auto* qspeed = qext->FirstChildElement("speed");
                    if (qspeed) qspeed->QueryFloatText(&speed);
                    else { speed = 0.0; speedLoaded = false; }
                }
                if (qext && loadCourse) {
                    auto* qcourse = qext->FirstChildElement("course");
                    if (qcourse) qcourse->QueryFloatText(&course);
                    else { course = 0.0; courseLoaded = false; }
                }

                seg.addPoint(lat, lon, ele, time, speed, course);

            }

            track.segs.push_back(std::make_shared<GPXSegment>(seg));
        }
    }

    return tracks;

}


}