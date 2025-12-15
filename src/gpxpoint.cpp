#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "gpxpoint.hpp"
#include <sstream>
#include <iomanip>
#include <httplib.h>
#include <tinyxml2.h>
namespace mygpx {

    double ComputePointDistance(const GPXPoint& p1, const GPXPoint& p2) {
        return ComputePointDistance(p1.lat, p1.lon, p2.lat, p2.lon);
    }

    double ComputeTimeDifference(const GPXPoint& p1, const GPXPoint& p2) {
        return static_cast<double>(p2.time - p1.time);
    }

    double ComputeElevationDifference(const GPXPoint& p1, const GPXPoint& p2) {
        return p2.ele - p1.ele;
    }

    double ComputeVerticalSpeed(const GPXPoint& p1, const GPXPoint& p2) {
        double elevation_diff = ComputeElevationDifference(p1, p2);
        double time_diff = ComputeTimeDifference(p1, p2);
        if (time_diff == 0) {
            return 0.0;
        }
        return elevation_diff / time_diff; // meters per second
    }

    double ComputeHorizontalSpeedRate(const GPXPoint& p1, const GPXPoint& p2) {
        double distance = ComputePointDistance(p1, p2);
        double time_diff = std::abs(ComputeTimeDifference(p1, p2));
        if (time_diff == 0) {
            return 0.0;
        }
        return distance / time_diff; // meters per second
    }

    std::ostream& operator<<(std::ostream& os, const GPXPoint& point) {
        os << "GPXPoint (" << std::abs(point.lat) << "°";
        if (point.lat > 0) os << "N";
        else if (point.lat < 0) os << "S";

        os << ", " << std::abs(point.lon) <<  "°";
        if (point.lon > 0) os << "E";
        else if (point.lon < 0) os << "W";

        os << ", " << point.ele << "m, ";

        os << from_timestamp_seconds(point.time) << ")";
        return os;
    }

    std::string GPXPoint::QueryAddress() const {
        httplib::Client cli("https://restapi.amap.com");

        httplib::Params params{
            {"key", getAmapAPIKey()},
            {"location", std::to_string(lon) + "," + std::to_string(lat)},
            {"radius", "1000"},
            {"extensions", "all"},
            {"roadlevel", "0"},
            {"output", "XML"}
        };

        httplib::Headers headers;

        auto res = cli.Get("/v3/geocode/regeo", params, headers);

        if (!res) {
            // Check the error type
            const auto err = res.error();

            switch (err) {
            case httplib::Error::SSLConnection:
                std::cout << "SSL connection failed, SSL error: "
                            << res.ssl_error() << std::endl;
                break;

            case httplib::Error::SSLLoadingCerts:
                std::cout << "SSL cert loading failed, OpenSSL error: "
                            << std::hex << res.ssl_openssl_error() << std::endl;
                break;

            case httplib::Error::SSLServerVerification:
                std::cout << "SSL verification failed, X509 error: "
                            << res.ssl_openssl_error() << std::endl;
                break;

            case httplib::Error::SSLServerHostnameVerification:
                std::cout << "SSL hostname verification failed, X509 error: "
                            << res.ssl_openssl_error() << std::endl;
                break;

            default:
                std::cout << "HTTP error: " << httplib::to_string(err) << std::endl;
            }
        } else {
            std::cout << "HTTP Status: " << res->status << std::endl;
        }

        tinyxml2::XMLDocument doc;

        if (doc.Parse(res->body.c_str()) != tinyxml2::XML_SUCCESS) {
            throw std::runtime_error("Failed to parse XML");
        }

        auto* response = doc.FirstChildElement("response");
        if (!response) throw std::runtime_error("Missing label <response>");
        auto* status = response->FirstChildElement("status");
        if (!status) throw std::runtime_error("Amap API returned error");
            if (std::string(status->GetText()) != "1") {
                throw std::runtime_error("Amap API returned status != 1");
            }
        
        auto* regeocode = response->FirstChildElement("regeocode");
        if (!regeocode) throw std::runtime_error("Missing label <regeocode>");
        auto* address_component = regeocode->FirstChildElement("addressComponent");
        if (!address_component) throw std::runtime_error("Missing label <addressComponent>");
        
        auto* province = address_component->FirstChildElement("province");
        auto* city = address_component->FirstChildElement("city");
        auto* district = address_component->FirstChildElement("district");
        auto* township = address_component->FirstChildElement("township");
        auto* streetNumber = address_component->FirstChildElement("streetNumber");
        auto* street = streetNumber ? streetNumber->FirstChildElement("street") : nullptr;
        auto* number = streetNumber ? streetNumber->FirstChildElement("number") : nullptr;

        std::string address;
        if (province && province->GetText()) address += province->GetText();
        if (city && city->GetText()) address += city->GetText();
        if (district && district->GetText()) address += district->GetText();
        if (township && township->GetText()) address += township->GetText();
        if (street && number && street->GetText() && number->GetText()) {
            address += street->GetText();
            address += number->GetText();
        }

        return address;

    }

}