#include "track.hpp"
#include "utils.hpp"
#include "validation.hpp"
#include <tinyxml2.h>
#include <iomanip>
#include <sstream>

namespace mygpx {

size_t Track::getPointCount() const {
    size_t total = 0;
    for (const auto& seg : segs) {
        total += seg->getPointCount();
    }
    return total;
}

GPXPoint Track::getStartPoint() const {
    validateInRange(segs.size(), static_cast<size_t>(1));
    return segs.front()->getStartPoint();
}

GPXPoint Track::getEndPoint() const {
    validateInRange(segs.size(), static_cast<size_t>(1));
    return segs.back()->getEndPoint();
}

size_t Track::getSegmentPointCount(size_t idx) const {
    validateIndex(idx, segs.size());
    return segs[idx]->getPointCount();
}

GPXPoint Track::getSegmentStartPoint(size_t idx) const {
    validateIndex(idx, segs.size());
    return segs[idx]->getStartPoint();
}

GPXPoint Track::getSegmentEndPoint(size_t idx) const {
    validateIndex(idx, segs.size());
    return segs[idx]->getEndPoint();
}

double Track::getTotalDistance() const {
    double total = 0.0;
    for (const auto& seg : segs) {
        total += seg->getTotalDistance();
    }
    return total;
}

double Track::getSegmentDistance(size_t idx) const {
    validateIndex(idx, segs.size());
    return segs[idx]->getTotalDistance();
}

std::string Track::getSegmentType(size_t idx) const {
    validateIndex(idx, segs.size());
    return segs[idx]->getType();
}

double Track::getStartLat() const { return getStartPoint().getLatitude(); }
double Track::getStartLon() const { return getStartPoint().getLongitude(); }
double Track::getStartEle() const { return getStartPoint().getElevation(); }
double Track::getEndLat() const { return getEndPoint().getLatitude(); }
double Track::getEndLon() const { return getEndPoint().getLongitude(); }
double Track::getEndEle() const { return getEndPoint().getElevation(); }
double Track::getTotalLatDiff() const { return getEndLat() - getStartLat(); }
double Track::getTotalLonDiff() const { return getEndLon() - getStartLon(); }
double Track::getTotalEleDiff() const { return getEndEle() - getStartEle(); }

long long Track::getStartTime() const {
    validateInRange(segs.size(), static_cast<size_t>(1));
    return segs.front()->getStartTime();
}

long long Track::getEndTime() const {
    validateInRange(segs.size(), static_cast<size_t>(1));
    return segs.back()->getEndTime();
}

int Track::getTotalTime() const {
    return getEndTime() - getStartTime();
}

long long Track::getSegmentStartTime(size_t idx) const {
    validateIndex(idx, segs.size());
    return segs[idx]->getStartTime();
}
long long Track::getSegmentEndTime(size_t idx) const {
    validateIndex(idx, segs.size());
    return segs[idx]->getEndTime();
}
int Track::getSegmentTime(size_t idx) const { return getSegmentEndTime(idx) - getSegmentStartTime(idx); }

double Track::getSegmentStartLat(size_t idx) const { return getSegmentStartPoint(idx).getLatitude(); }
double Track::getSegmentStartLon(size_t idx) const { return getSegmentStartPoint(idx).getLongitude(); }
double Track::getSegmentStartEle(size_t idx) const { return getSegmentStartPoint(idx).getElevation(); }
double Track::getSegmentEndLat(size_t idx) const { return getSegmentEndPoint(idx).getLatitude(); }
double Track::getSegmentEndLon(size_t idx) const { return getSegmentEndPoint(idx).getLongitude(); }
double Track::getSegmentEndEle(size_t idx) const { return getSegmentEndPoint(idx).getElevation(); }
double Track::getSegmentLatDiff(size_t idx) const { return getSegmentEndLat(idx) - getSegmentStartLat(idx); }
double Track::getSegmentLonDiff(size_t idx) const { return getSegmentEndLon(idx) - getSegmentStartLon(idx); }
double Track::getSegmentEleDiff(size_t idx) const { return getSegmentEndEle(idx) - getSegmentStartEle(idx); }

const std::vector<GPXPoint>& Track::getPoints() const {
    points_cache.clear();
    for (const auto& seg : segs) {
        const auto& pts = seg->getPoints();
        points_cache.insert(points_cache.end(), pts.begin(), pts.end());
    }
    return points_cache;
}

const GPXPoint& Track::getPoint(size_t index) const {
    size_t offset = 0;
    for (const auto& seg : segs) {
        size_t count = seg->getPointCount();
        if (index < offset + count) {
            return seg->getPoint(index - offset);
        }
        offset += count;
    }
    throw std::out_of_range("Track point index out of range.");
}

void Track::addSegment(std::shared_ptr<BaseSegment> seg) {
    segs.push_back(seg);
}

void Track::addSegment(std::shared_ptr<BaseSegment> seg, size_t idx) {
    validateIndex(idx, segs.size() + 1);
    segs.insert(segs.begin() + idx, seg);
}

std::shared_ptr<BaseSegment> Track::getSegment(size_t idx) {
    validateIndex(idx, segs.size());
    return segs[idx];
}

std::string Track::getSegmentType(size_t idx) {
    validateIndex(idx, segs.size());
    return segs[idx]->getType();
}

void Track::toGPX(const std::string& filename) const {
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

    for (const auto& seg : segs) {
        auto* trkseg = doc.NewElement("trkseg");
        trk->InsertEndChild(trkseg);

        for (const auto& pt : seg->getPoints()) {
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
    }

    if (doc.SaveFile(filename.c_str()) != tinyxml2::XML_SUCCESS) {
        throw std::runtime_error("Failed to save GPX file.");
    }
}

}
