#include "segment.hpp"
#include "utils.hpp"
#include "validation.hpp"
#include <tinyxml2.h>
#include <iomanip>
#include <sstream>

namespace mygpx {

double BaseSegment::getStartLat() const { return getStartPoint().getLatitude(); }
double BaseSegment::getStartLon() const { return getStartPoint().getLongitude(); }
double BaseSegment::getStartEle() const { return getStartPoint().getElevation(); }
double BaseSegment::getEndLat() const { return getEndPoint().getLatitude(); }
double BaseSegment::getEndLon() const { return getEndPoint().getLongitude(); }
double BaseSegment::getEndEle() const { return getEndPoint().getElevation(); }
double BaseSegment::getTotalLatDiff() const { return getEndLat() - getStartLat(); }
double BaseSegment::getTotalLonDiff() const { return getEndLon() - getStartLon(); }
double BaseSegment::getTotalEleDiff() const { return getEndEle() - getStartEle(); }

long long BaseSegment::getStartTime() const {
    if (!has_time) throw std::runtime_error("Segment does not have time information.");
    else return getStartPoint().getTime();
}
long long BaseSegment::getEndTime() const {
    if (!has_time) throw std::runtime_error("Segment does not have time information.");
    else return getEndPoint().getTime();
}
int BaseSegment::getTotalTime() const {
    if (!has_time) throw std::runtime_error("Segment does not have time information.");
    return getEndTime() - getStartTime();
}

const std::vector<GPXPoint>& BaseSegment::getPoints() const { return points; }
const GPXPoint& BaseSegment::getPoint(size_t index) const {
    validateIndex(index, points.size());
    return points[index];
}

void BaseSegment::toGPX(const std::string& filename) const {
    tinyxml2::XMLDocument doc;

    auto* decl = doc.NewDeclaration(R"(xml version="1.0" encoding="UTF-8")");
    doc.InsertFirstChild(decl);

    auto* gpx = doc.NewElement("gpx");
    gpx->SetAttribute("version", "1.1");
    gpx->SetAttribute("creator", "gpxlib");
    gpx->SetAttribute("xmlns", "http://www.topografix.com/GPX/1/1");
    doc.InsertEndChild(gpx);

    auto* trk = doc.NewElement("trk");
    gpx->InsertEndChild(trk);

    auto format6 = [](double value) {
        std::ostringstream oss;
        oss.setf(std::ios::fixed);
        oss << std::setprecision(6) << value;
        return oss.str();
    };
    auto format2 = [](double value) {
        std::ostringstream oss;
        oss.setf(std::ios::fixed);
        oss << std::setprecision(2) << value;
        return oss.str();
    };

    auto* trkseg = doc.NewElement("trkseg");
    trk->InsertEndChild(trkseg);

    for (const auto& pt : this->getPoints()) {
        auto* trkpt = doc.NewElement("trkpt");
        std::string lat_str = format6(pt.getLatitude());
        std::string lon_str = format6(pt.getLongitude());
        trkpt->SetAttribute("lat", lat_str.c_str());
        trkpt->SetAttribute("lon", lon_str.c_str());

        auto* ele = doc.NewElement("ele");
        std::string ele_str = format2(pt.getElevation());
        ele->SetText(ele_str.c_str());
        trkpt->InsertEndChild(ele);

        if (pt.getTime() != 0) {
            auto* time = doc.NewElement("time");
            std::string time_str = from_timestamp_seconds(pt.getTime());
            time->SetText(time_str.c_str());
            trkpt->InsertEndChild(time);
        }

        auto* extensions = doc.NewElement("extensions");
        auto* speed = doc.NewElement("speed");
        std::string speed_str = format2(pt.getSpeed());
        speed->SetText(speed_str.c_str());
        extensions->InsertEndChild(speed);

        auto* course = doc.NewElement("course");
        std::string course_str = format2(pt.getCourse());
        course->SetText(course_str.c_str());
        extensions->InsertEndChild(course);

        trkpt->InsertEndChild(extensions);
        trkseg->InsertEndChild(trkpt);
    }


    if (doc.SaveFile(filename.c_str()) != tinyxml2::XML_SUCCESS) {
        throw std::runtime_error("Failed to save GPX file.");
    }
}

}