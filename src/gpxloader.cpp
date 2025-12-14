#include "gpxloader.hpp"
#include <tinyxml2.h>

namespace mygpx {

std::vector<GPXPoint> GPXLoader::loadSegmentFromFile(const std::string& filename) {
    std::vector<GPXPoint> points;

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

                bool speedLoaded = loadSpeed;
                bool courseLoaded = loadCourse;

                // get lat/lon
                trkpt->QueryDoubleAttribute("lat", &lat);
                trkpt->QueryDoubleAttribute("lon", &lon);

                // ele may not exist
                auto* qele = trkpt->FirstChildElement("ele");
                if (qele) qele->QueryFloatText(&ele);
                else ele = 0;

                // time may not exist
                auto* qtime = trkpt->FirstChildElement("time");
                if (qtime) time = qtime->GetText();

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

                points.emplace_back(lat, lon, ele, time, speed, course);

            }
        }
    }

    return points;

}

std::vector<GPXSegment> GPXLoader::loadSegmentsFromFile(const std::string& filename) {

    std::vector<GPXSegment> segments;

    tinyxml2::XMLDocument doc;
    if (doc.LoadFile(filename.c_str()) != tinyxml2::XML_SUCCESS) {
        throw std::runtime_error("Failed to load GPX file.");
    }

    // find <trkpt> labels
    auto* root = doc.RootElement();
    for (auto* trk = root->FirstChildElement("trk"); trk; trk = trk->NextSiblingElement("trk")) {
        for (auto* trkseg = trk->FirstChildElement("trkseg"); trkseg; trkseg = trkseg->NextSiblingElement("trkseg")) {

            GPXSegment seg;

            for (auto* trkpt = trkseg->FirstChildElement("trkpt"); trkpt; trkpt = trkpt->NextSiblingElement("trkpt")) {

                double lat, lon;
                float ele, speed = 0.0, course = 0.0;
                std::string time;

                bool speedLoaded = loadSpeed;
                bool courseLoaded = loadCourse;

                // get lat/lon
                trkpt->QueryDoubleAttribute("lat", &lat);
                trkpt->QueryDoubleAttribute("lon", &lon);

                // ele may not exist
                auto* qele = trkpt->FirstChildElement("ele");
                if (qele) qele->QueryFloatText(&ele);
                else ele = 0;

                // time may not exist
                auto* qtime = trkpt->FirstChildElement("time");
                if (qtime) time = qtime->GetText();

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
        


            segments.push_back(seg);
        }
    }

    return segments;

}

}